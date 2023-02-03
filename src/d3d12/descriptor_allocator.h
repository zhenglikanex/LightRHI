#pragma once

#include <memory>
#include <set>
#include <map>
#include <optional>
#include <queue>
#include <mutex>

#include <rhi/resource.h>

#include "d3dx12.h"

namespace light::rhi
{
	class D12Device;

	class DescriptorAllocatorPage;

	class DescriptorAllocation
	{
	public:
		// create a null descriptor
		DescriptorAllocation();

		DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, uint32_t num_handles, uint32_t descriptor_size, DescriptorAllocatorPage* page);

		~DescriptorAllocation();

		// Copies are not allowed.
		DescriptorAllocation(const DescriptorAllocation&) = delete;
		DescriptorAllocation& operator=(const DescriptorAllocation&) = delete;

		// Move is allowed.
		DescriptorAllocation(DescriptorAllocation&& allocation) noexcept;

		DescriptorAllocation& operator=(DescriptorAllocation&& other) noexcept;

		// 检查是否有效
		bool IsNull() const;

		// 检查是否有效
		bool IsValid() const;

		void Reset();

		D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle(uint32_t offset = 0) const;

		uint32_t GetNumHandles() const;
	private:
		void Free();

		D3D12_CPU_DESCRIPTOR_HANDLE descriptor_;
		uint32_t num_handles_;
		uint32_t descriptor_size_;
		DescriptorAllocatorPage* page_;
	};

	class DescriptorAllocatorPage
	{
	public:
		D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const { return heap_type_; }

		// 检查是否还有连续的剩余描述符
		bool HasSpace(uint32_t num_descriptors) const;

		uint32_t NumFreeHandles() const;

		DescriptorAllocation Allocate(uint32_t num_descriptors);

		void Free(DescriptorAllocation&& allocation);

		void ReleaseStaleDescriptors();

		DescriptorAllocatorPage(D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t num_descriptors);

		// 计算对于Handle对于Heap起始位置的偏移
		uint32_t ComputeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle);

		// 添加描述符到FreeList
		void AddNewBlock(uint32_t offset, uint32_t num_descriptors);

		void FreeBlock(uint32_t offset, uint32_t num_descriptors);

	private:
		using OffsetType = uint32_t;

		using SizeType = uint32_t;

		struct BlockInfo
		{
			explicit BlockInfo(OffsetType offset)
				: offset(offset)
			{
				
			}

			OffsetType offset;

			bool operator<(BlockInfo rhs) const
			{
				return offset < rhs.offset;
			}
		};

		using FreeList = std::map<SizeType,std::set<BlockInfo>>;

		struct StaleDescriptorInfo
		{
			StaleDescriptorInfo(OffsetType offset, SizeType size)
				: offset(offset)
				, size(size)
			{
			}


			OffsetType offset;
			SizeType size;
		};

		D12Device* device_;
		D3D12_DESCRIPTOR_HEAP_TYPE heap_type_;
		Handle<ID3D12DescriptorHeap> d3d12_descriptor_heap_;
		CD3DX12_CPU_DESCRIPTOR_HANDLE base_descriptor_;
		uint32_t descriptor_handle_increment_size_;
		uint32_t num_descriptors_;
		uint32_t num_free_handles_;

		FreeList free_list_;
		std::queue<StaleDescriptorInfo> stale_descriptor_infos_;

		std::mutex mutex_;
	};

	class DescriptorAllocator
	{
	public:
		DescriptorAllocator(D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t page_size = 256);
		~DescriptorAllocator();
		
		DescriptorAllocation Allocate(uint32_t num_descriptors = 1);
		
		void ReleaseStaleDescriptors();
	private:
		DescriptorAllocatorPage* CreateAllocatorPage();

		D12Device* device_;
		D3D12_DESCRIPTOR_HEAP_TYPE heap_type_;
		uint32_t page_size_;

		std::vector<std::unique_ptr<DescriptorAllocatorPage>> pages_;
		std::set<size_t> available_pages_;
		std::mutex mutex_;
	};
}