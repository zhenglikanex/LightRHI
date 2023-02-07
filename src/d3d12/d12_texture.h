#pragma once

#include <unordered_map>

#include "rhi/types.h"
#include "rhi/texture.h"

#include "descriptor_allocator.h"
#include "d3dx12.h"

namespace light::rhi
{
	class D12Device;

	class D12Texture final : public Texture
	{
	public:
		D12Texture(D12Device* device, const TextureDesc& desc);

		D12Texture(D12Device* device, const TextureDesc& desc, ID3D12Resource* native);

		~D12Texture()
		{
			int a = 10;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE GetRTV();
		D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(Format format, uint32_t mip_level,uint32_t array_slice,uint32_t num_array_slices);
		D3D12_CPU_DESCRIPTOR_HANDLE GetDSV();
		D3D12_CPU_DESCRIPTOR_HANDLE GetDSV(uint32_t mip_level, uint32_t array_slice, uint32_t num_array_slices);
		D3D12_CPU_DESCRIPTOR_HANDLE GetSRV(Format format,TextureDimension dimension, uint32_t mip_level,uint32_t num_mip_levels, uint32_t array_slice, uint32_t num_array_slices);

		ID3D12Resource* GetNative() { return resource_; }
	private:

		D12Device* device_;
		Handle<ID3D12Resource> resource_;
		std::unordered_map<size_t, DescriptorAllocation> rtv_map_;
		std::unordered_map<size_t, DescriptorAllocation> dsv_map_;
		std::unordered_map<size_t, DescriptorAllocation> srv_map_;
	};
}
