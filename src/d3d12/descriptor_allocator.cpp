#include "descriptor_allocator.h"

#include "d12_device.h"

namespace light::rhi
{
	DescriptorAllocation::DescriptorAllocation()
	{
		descriptor_.ptr = 0;
		num_handles_ = 0;
		descriptor_size_ = 0;
		page_ = nullptr;
	}

	DescriptorAllocation::DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, uint32_t num_handles,
		uint32_t descriptor_size, DescriptorAllocatorPage* page)
		: descriptor_(descriptor)
		, num_handles_(num_handles)
		, descriptor_size_(descriptor_size)
		, page_(page)
	{

	}

	DescriptorAllocation::~DescriptorAllocation()
	{
		Free();
	}

	DescriptorAllocation::DescriptorAllocation(DescriptorAllocation&& allocation) noexcept
		: descriptor_(allocation.descriptor_)
		, num_handles_(allocation.num_handles_)
		, descriptor_size_(allocation.descriptor_size_)
		, page_(std::move(allocation.page_))
	{

	}

	DescriptorAllocation& DescriptorAllocation::operator=(DescriptorAllocation&& other) noexcept
	{
		Free();

		descriptor_ = other.descriptor_;
		num_handles_ = other.num_handles_;
		descriptor_size_ = other.descriptor_size_;
		page_ = std::move(other.page_);

		other.descriptor_.ptr = 0;
		other.num_handles_ = 0;
		other.descriptor_size_ = 0;

		return *this;
	}

	bool DescriptorAllocation::IsNull() const
	{
		return descriptor_.ptr == 0;
	}

	bool DescriptorAllocation::IsValid() const
	{
		return !IsNull();
	}

	void DescriptorAllocation::Reset()
	{
		descriptor_.ptr = 0;
		num_handles_ = 0;
		descriptor_size_ = 0;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocation::GetDescriptorHandle(uint32_t offset) const
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptor_, static_cast<INT>(offset * descriptor_size_));
	}

	uint32_t DescriptorAllocation::GetNumHandles() const
	{
		return num_handles_;
	}

	void DescriptorAllocation::Free()
	{
		if(!IsNull())
		{
			page_->Free(std::move(*this));
		}
	}

	bool DescriptorAllocatorPage::HasSpace(uint32_t num_descriptors) const
	{
		return num_descriptors <= num_free_handles_&& free_list_.lower_bound(num_descriptors) != free_list_.end();
	}

	uint32_t DescriptorAllocatorPage::NumFreeHandles() const
	{
		return num_free_handles_;
	}

	DescriptorAllocation DescriptorAllocatorPage::Allocate(uint32_t num_descriptors)
	{
		std::unique_lock<std::mutex> lock(mutex_);

		if(num_descriptors > num_free_handles_)
		{
			return DescriptorAllocation();
		}

		// 找一个合适大小的池子
		auto pool_it = free_list_.lower_bound(num_descriptors);
		if(pool_it == free_list_.end())
		{
			return DescriptorAllocation();
		}

		// 从池子中返回一个
		auto block_info = *pool_it->second.begin();
		pool_it->second.erase(pool_it->second.begin());

		// 拆分剩余block
		SizeType new_size = pool_it->first - num_descriptors;
		if (new_size > 0)
		{
			AddNewBlock(block_info.offset + num_descriptors, new_size);
		}

		// 移除分配完的池子
		if(pool_it->second.empty())
		{
			free_list_.erase(pool_it);
		}

		return DescriptorAllocation(
			CD3DX12_CPU_DESCRIPTOR_HANDLE(base_descriptor_, block_info.offset, descriptor_handle_increment_size_),
			num_descriptors, 
			descriptor_handle_increment_size_, 
			this);
	}

	void DescriptorAllocatorPage::Free(DescriptorAllocation&& allocation)
	{
		// 计算对于heap起始位置的偏移
		auto offset = ComputeOffset(allocation.GetDescriptorHandle());

		std::unique_lock lock(mutex_);

		// 知道当前帧结束在释放
		stale_descriptor_infos_.emplace(offset, allocation.GetNumHandles());
		allocation.Reset();
	}

	void DescriptorAllocatorPage::ReleaseStaleDescriptors()
	{
		std::unique_lock lock(mutex_);

		while (!stale_descriptor_infos_.empty())
		{
			auto& info = stale_descriptor_infos_.front();
			FreeBlock(info.offset, info.size);

			stale_descriptor_infos_.pop();
		}
	}

	DescriptorAllocatorPage::DescriptorAllocatorPage(D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type,
		uint32_t num_descriptors)
			: device_(device)
			, heap_type_(type)
		, num_descriptors_(num_descriptors)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
		heap_desc.Type = heap_type_;
		heap_desc.NumDescriptors = num_descriptors_;

		ThrowIfFailed(device->GetNative()->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&d3d12_descriptor_heap_)));

		base_descriptor_ = d3d12_descriptor_heap_->GetCPUDescriptorHandleForHeapStart();
		descriptor_handle_increment_size_ = device_->GetDescriptorHandleIncrementSize(type);
		num_free_handles_ = num_descriptors_;

		// 初始化free_list
		AddNewBlock(0, num_descriptors_);
	}

	uint32_t DescriptorAllocatorPage::ComputeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle)
	{
		return (handle.ptr - base_descriptor_.ptr) / descriptor_handle_increment_size_;
	}

	void DescriptorAllocatorPage::AddNewBlock(uint32_t offset, uint32_t num_descriptors)
	{
		auto it = free_list_.find(num_descriptors);
		if(it != free_list_.end())
		{
			it->second.emplace(offset);
		}
		else
		{
			free_list_.emplace(num_descriptors, std::set({ BlockInfo(offset) }));
		}
	}

	void DescriptorAllocatorPage::FreeBlock(uint32_t offset, uint32_t num_descriptors)
	{
		auto pool_it = free_list_.find(num_descriptors);
		if (pool_it == free_list_.end())
		{
			free_list_.emplace(num_descriptors, std::set({ BlockInfo(offset) }));
			return;
		}

		uint32_t new_offset = offset;
		uint32_t new_num_descriptors = num_descriptors;
		BlockInfo cur_block(offset);

		auto next_block_it = pool_it->second.upper_bound(cur_block);
		
		// 向上合并连续的块
		if (next_block_it != pool_it->second.begin())
		{
			auto prev_block_it = --next_block_it;
			
			// 是否连续
			if (prev_block_it->offset + num_descriptors == offset)
			{
				new_offset = prev_block_it->offset;
				new_num_descriptors += num_descriptors;

				//移除,等待合并后加入
				pool_it->second.erase(prev_block_it);
			}
		}

		// 向下合并连续的块
		if (next_block_it != pool_it->second.end())
		{
			// 是否连续
			if (offset + num_descriptors == next_block_it->offset)
			{
				new_num_descriptors += num_descriptors;
				
				//移除,等待合并后加入
				pool_it->second.erase(next_block_it);
			}
		}

		// 移除空的池子
		if(pool_it->second.empty())
		{
			free_list_.erase(pool_it);
		}

		// 插入block
		AddNewBlock(new_offset, new_num_descriptors);
	}

	DescriptorAllocator::DescriptorAllocator(D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t page_size)
		: device_(device)
		, heap_type_(type)
		, page_size_(page_size)
	{
	}

	DescriptorAllocator::~DescriptorAllocator()
	{

	}

	DescriptorAllocation DescriptorAllocator::Allocate(uint32_t num_descriptors)
	{
		std::unique_lock<std::mutex> lock(mutex_);

		DescriptorAllocation allocation;

		auto iter = available_pages_.begin();
		while(iter != available_pages_.end())
		{
			auto& page = pages_[*iter];
			allocation = page->Allocate(num_descriptors);

			if(page->NumFreeHandles() == 0)
			{
				// 将空的从存活的列表中移除
				iter = available_pages_.erase(iter);
			}
			else
			{
				++iter;
			}

			if(allocation.IsValid())
			{
				break;
			}
		}

		if(allocation.IsNull())
		{
			auto page = CreateAllocatorPage();
			allocation = page->Allocate(num_descriptors);
		}

		return std::move(allocation);
	}

	void DescriptorAllocator::ReleaseStaleDescriptors()
	{
		std::unique_lock<std::mutex> lock(mutex_);

		for (size_t i = 0; i < pages_.size(); ++i)
		{
			pages_[i]->ReleaseStaleDescriptors();

			if (pages_[i]->NumFreeHandles() > 0)
			{
				available_pages_.insert(i);
			}
		}
	}

	DescriptorAllocatorPage* DescriptorAllocator::CreateAllocatorPage()
	{
		auto page = std::make_unique<DescriptorAllocatorPage>(device_, heap_type_, page_size_);
		pages_.push_back(std::move(page));

		available_pages_.insert(pages_.size() - 1);

		return pages_.back().get();
	}
}
