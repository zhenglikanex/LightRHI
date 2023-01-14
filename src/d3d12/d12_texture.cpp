#include "d12_texture.h"

namespace light::rhi
{
	D12Texture::D12Texture(D12Device* device, const TextureDesc& desc)
		: Texture(desc)
		, device_(device)
		, resource_(nullptr)
	{
	}

	D12Texture::D12Texture(D12Device* device, const TextureDesc& desc, ID3D12Resource* native)
		: Texture(desc)
		, device_(device)
		, resource_(native)
	{

	}
}
