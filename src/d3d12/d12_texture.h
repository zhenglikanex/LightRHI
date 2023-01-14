#pragma once

#include "rhi/texture.h"

#include "d3dx12.h"

namespace light::rhi
{
	class D12Device;

	class D12Texture final : public Texture
	{
	public:
		D12Texture(D12Device* device, const TextureDesc& desc);

		D12Texture(D12Device* device, const TextureDesc& desc, ID3D12Resource* native);

		D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const;
		D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const;
		D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const;

		ID3D12Resource* GetNative() { return resource_; }
	private:
		D12Device* device_;
		Handle<ID3D12Resource> resource_;
	};
}
