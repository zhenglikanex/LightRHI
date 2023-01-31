#include "dynamic_descriptor_heap.h"

#include "d12_device.h"

namespace light::rhi
{
	DynamicDescriptorHeap::DynamicDescriptorHeap(D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heap_type,
		uint32_t heap_size)
		: device_(device)
		, heap_type_(heap_type)
		, heap_size_(heap_size)
		, descriptor_table_bit_mask_(0)
		, stale_descriptor_table_bit_mask_(0)
		, current_descriptor_heap_(nullptr)
		, current_gpu_descriptor_handle_(D3D12_DEFAULT)
		, current_cpu_descriptor_handle_(D3D12_DEFAULT)
		, num_free_handles_(0)
	{
		descriptor_handle_increment_size_ = device_->GetDescriptorHandleIncrementSize(heap_type_);

		cpu_descriptor_handles_.resize(heap_size_);
	}

	DynamicDescriptorHeap::~DynamicDescriptorHeap()
	{
	}

	void DynamicDescriptorHeap::StageDescriptors(uint32_t parameter_index, uint32_t offset, uint32_t num_descriptors,
		D3D12_CPU_DESCRIPTOR_HANDLE src_descriptors)
	{
		if(num_descriptors > heap_size_ || parameter_index >= kMaxDescriptorTables)
		{
			throw std::bad_alloc();
		}

		DescriptorTableCache& descriptor_table_cache = descriptor_table_cache_[parameter_index];

		if((offset + num_descriptors) > descriptor_table_cache.num_descriptors)
		{
			throw std::length_error("Number of descriptors exceeds the number of descriptors in the descriptor table.");
		}

		// 将描述符暂存到DynamicDescriptorHeap的cpu可见堆,等待提交到gpu
		D3D12_CPU_DESCRIPTOR_HANDLE* dst_descriptor = (descriptor_table_cache.base_descriptor + offset);
		for (uint32_t i = 0; i < num_descriptors; ++i)
		{
			dst_descriptor[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(src_descriptors, descriptor_handle_increment_size_ * i);
		}

		// 设置需要更新的描述符表掩码位
		stale_descriptor_table_bit_mask_ |= (1 << parameter_index);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE DynamicDescriptorHeap::CopyDescriptor(D12CommandList* command_list,
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_descriptor)
	{
		// 是否需要新的堆
		if (!current_descriptor_heap_ || num_free_handles_ < 1)
		{
			current_descriptor_heap_ = RequestDescriptorHeap();
			current_cpu_descriptor_handle_ = current_descriptor_heap_->GetCPUDescriptorHandleForHeapStart();
			current_gpu_descriptor_handle_ = current_descriptor_heap_->GetGPUDescriptorHandleForHeapStart();

			command_list->SetDescriptorHeap(heap_type_, current_descriptor_heap_);

			// current_descriptor_heap_更新后所描述符表都需要更新
			// 需要将stale_descriptor_table_bit_mask_至为descriptor_table_bit_mask_
			stale_descriptor_table_bit_mask_ = descriptor_table_bit_mask_;
		}

		device_->GetNative()->CopyDescriptorsSimple(1, current_cpu_descriptor_handle_, cpu_descriptor, heap_type_);

		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = current_gpu_descriptor_handle_;

		current_gpu_descriptor_handle_.Offset(1, descriptor_handle_increment_size_);
		current_cpu_descriptor_handle_.Offset(1, descriptor_handle_increment_size_);
		num_free_handles_ -= 1;

		return gpu_handle;
	}

	void DynamicDescriptorHeap::CommitStatedDescriptorsForDraw(D12CommandList* command_list)
	{
		CommitDescriptorTables(command_list, &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable);
	}

	void DynamicDescriptorHeap::CommitStatedDescriptorsForCompute(D12CommandList* command_list)
	{
		CommitDescriptorTables(command_list, &ID3D12GraphicsCommandList::SetComputeRootDescriptorTable);
	}

	void DynamicDescriptorHeap::ParseRootSignature(const RootSignature* root_signature)
	{
		assert(root_signature);

		// root signature如果改变，所有的描述符都要重新绑定
		stale_descriptor_table_bit_mask_ = 0;

		if(heap_type_ == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		{
			descriptor_table_bit_mask_ = root_signature->GetDescriptorTableBitMask();
		}
		else if (heap_type_ == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		{
			descriptor_table_bit_mask_ = root_signature->GetSamplerTableBitMask();
		}

		uint32_t descriptor_table_bit_mask = descriptor_table_bit_mask_;

		uint32_t current_offset = 0;
		DWORD index;
		while(_BitScanForward(&index,descriptor_table_bit_mask))
		{
			uint32_t num_descriptors = root_signature->GetNumDescriptors(index);

			auto& descritpor_table = descriptor_table_cache_[index];
			descritpor_table.num_descriptors = num_descriptors;
			descritpor_table.base_descriptor = cpu_descriptor_handles_.data() + current_offset;

			current_offset += num_descriptors;

			// 将当前mask设为0,保证循环正确
			descriptor_table_bit_mask ^= (1 << index);
		}
	}

	void DynamicDescriptorHeap::Rest()
	{
		available_descriptor_heaps_ = descriptor_heap_pool_;

		current_descriptor_heap_ = nullptr;
		current_cpu_descriptor_handle_ = CD3DX12_CPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
		current_gpu_descriptor_handle_ = CD3DX12_GPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);

		descriptor_table_bit_mask_ = 0;
		stale_descriptor_table_bit_mask_ = 0;
		num_free_handles_ = 0;

		for (size_t i = 0; i < kMaxDescriptorTables; ++i)
		{
			descriptor_table_cache_[i].Reset();
		}
	}

	ID3D12DescriptorHeap* DynamicDescriptorHeap::RequestDescriptorHeap()
	{
		if(available_descriptor_heaps_.empty())
		{
			auto heap = CreateDescriptorHeap();
			descriptor_heap_pool_.push(heap);
			return heap;
		}
		else
		{
			auto heap = available_descriptor_heaps_.front();
			available_descriptor_heaps_.pop();
			return heap;
		}
	}

	Handle<ID3D12DescriptorHeap> DynamicDescriptorHeap::CreateDescriptorHeap()
	{
		Handle<ID3D12DescriptorHeap> heap;

		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Type = heap_type_;
		desc.NumDescriptors = heap_size_;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		device_->GetNative()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap));

		return heap;
	}

	uint32_t DynamicDescriptorHeap::ComputeStaleDescriptorCount() const
	{
		uint32_t num_descriptors = 0;
		for (size_t i = 0; i < kMaxDescriptorTables; ++i)
		{
			num_descriptors += descriptor_table_cache_[i].num_descriptors;
		}

		return num_descriptors;
	}

	void DynamicDescriptorHeap::CommitDescriptorTables(D12CommandList* command_list,
		std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> set_func)
	{
		uint32_t num_descriptors = ComputeStaleDescriptorCount();

		// 是否需要新的堆
		if(!current_descriptor_heap_ || num_free_handles_ < num_descriptors)
		{
			current_descriptor_heap_ = RequestDescriptorHeap();
			current_cpu_descriptor_handle_ = current_descriptor_heap_->GetCPUDescriptorHandleForHeapStart();
			current_gpu_descriptor_handle_ = current_descriptor_heap_->GetGPUDescriptorHandleForHeapStart();

			command_list->SetDescriptorHeap(heap_type_, current_descriptor_heap_);

			// current_descriptor_heap_更新后所描述符表都需要更新
			// 需要将stale_descirptor_table_bit_mask_至为descriptor_table_bit_mask_
			stale_descriptor_table_bit_mask_ = descriptor_table_bit_mask_;
		}

		DWORD index = 0;
		while(_BitScanForward(&index,stale_descriptor_table_bit_mask_))
		{
			DescriptorTableCache& descriptor_table_cache = descriptor_table_cache_[index];

			uint32_t num_src_descriptors = descriptor_table_cache_[index].num_descriptors;
			D3D12_CPU_DESCRIPTOR_HANDLE* src_descritpor_range_start = descriptor_table_cache_[index].base_descriptor;

			device_->GetNative()->CopyDescriptorsSimple(
				num_src_descriptors,
				current_cpu_descriptor_handle_,
				*src_descritpor_range_start,
				heap_type_);
			
			set_func(
				command_list->GetD3D12GraphicsCommandList(),
				index,
				current_gpu_descriptor_handle_);

			// 移动current handle
			current_cpu_descriptor_handle_.Offset(num_src_descriptors,descriptor_handle_increment_size_);
			current_gpu_descriptor_handle_.Offset(num_src_descriptors, descriptor_handle_increment_size_);

			num_free_handles_ -= num_src_descriptors;

			// 重置提交的描述符表掩码，保证循环正确
			stale_descriptor_table_bit_mask_ ^= (1 << index);
		}
	}
}
