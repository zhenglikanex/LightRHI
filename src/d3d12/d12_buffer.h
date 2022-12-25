#pragma once

#include "d3dx12.h"
#include "rhi/buffer.h"

namespace light::rhi
{
	class D12Device;

	class D12Buffer final : public Buffer
	{
	public:
		D12Buffer(D12Device* device,const BufferDesc& desc);

		ID3D12Resource* GetNative() { return resource_.Get(); }

	private:
		Handle<ID3D12Resource> resource_;
	};
}