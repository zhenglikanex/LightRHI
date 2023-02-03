#pragma once

#include <unordered_map>

#include "d3dx12.h"
#include "rhi/buffer.h"

#include "descriptor_allocator.h"

namespace light::rhi
{
	class D12Device;

	class D12Buffer final : public Buffer
	{
	public:
		D12Buffer(D12Device* device,const BufferDesc& desc);

		D3D12_CPU_DESCRIPTOR_HANDLE GetCBV();
		D3D12_CPU_DESCRIPTOR_HANDLE GetSBV(uint32_t offset, uint32_t byte_size);
		D3D12_CPU_DESCRIPTOR_HANDLE GetUBV(uint32_t offset,uint32_t byte_size);

		ID3D12Resource* GetNative() { return resource_.Get(); }
	private:
		D12Device* device_;
		Handle<ID3D12Resource> resource_;

		DescriptorAllocation cbv_;
		std::unordered_map<size_t, DescriptorAllocation> sbv_map_;
		std::unordered_map<size_t, DescriptorAllocation> ubv_map_;
	};
}