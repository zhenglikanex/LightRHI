#pragma once

#include <cstdint>
#include <queue>
#include <functional>

#include "rhi/resource.h"

#include "d3dx12.h"


namespace light::rhi
{
	class D12Device;
	class D12CommandList;
	class RootSignature;

	class DynamicDescriptorHeap
	{
	public:
		DynamicDescriptorHeap(D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heap_type, uint32_t heap_size = 1024);

		~DynamicDescriptorHeap();

		void StageDescriptors(uint32_t parameter_index, uint32_t offset, uint32_t num_descriptors,
			D3D12_CPU_DESCRIPTOR_HANDLE src_descriptors);

		D3D12_GPU_DESCRIPTOR_HANDLE CopyDescriptor(D12CommandList* command_list, D3D12_CPU_DESCRIPTOR_HANDLE cpu_descriptor);

		void CommitStatedDescriptorsForDraw(D12CommandList* command_list);

		void CommitStatedDescriptorsForCompute(D12CommandList* command_list);

		void ParseRootSignature(const RootSignature* root_signature);

		void Rest();
	private:
		ID3D12DescriptorHeap* RequestDescriptorHeap();

		Handle<ID3D12DescriptorHeap> CreateDescriptorHeap();

		// 计算需要提交到GPU可见堆的数量
		uint32_t ComputeStaleDescriptorCount() const;

		void CommitDescriptorTables(D12CommandList* command_list,
			std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> set_func);

		//每个根签名的最大描述符表数。
		//32位掩码用于跟踪作为描述符表的根参数索引。
		static constexpr uint32_t kMaxDescriptorTables = 32;

		
		struct DescriptorTableCache
		{
			void Reset()
			{
				num_descriptors = 0;
				base_descriptor = nullptr;
			}

			uint32_t num_descriptors = 0;
			D3D12_CPU_DESCRIPTOR_HANDLE* base_descriptor = nullptr;
		};

		D12Device* device_;

		//有效的类型：
		//	D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
		//	D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER
		//这俩个类型的描述符需要绑定到gpu可见的heap
		D3D12_DESCRIPTOR_HEAP_TYPE heap_type_;

		uint32_t heap_size_;

		uint32_t descriptor_handle_increment_size_;

		//暂存所有的cpu可见的描述符
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> cpu_descriptor_handles_;

		DescriptorTableCache descriptor_table_cache_[kMaxDescriptorTables];

		// root signature对应的描述符表跟参数
		uint32_t descriptor_table_bit_mask_;

		// 需要的更新的root signature的描述符表的根参数索引的掩码
		uint32_t stale_descriptor_table_bit_mask_;

		std::queue<Handle<ID3D12DescriptorHeap>> descriptor_heap_pool_;
		std::queue<Handle<ID3D12DescriptorHeap>> available_descriptor_heaps_;

		ID3D12DescriptorHeap* current_descriptor_heap_;
		CD3DX12_GPU_DESCRIPTOR_HANDLE current_gpu_descriptor_handle_;
		CD3DX12_CPU_DESCRIPTOR_HANDLE current_cpu_descriptor_handle_;

		uint32_t num_free_handles_;
	};
}