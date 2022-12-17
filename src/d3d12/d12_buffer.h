#pragma once

#include "d3dx12.h"
#include "rhi/buffer.h"

namespace light::rhi
{
	class D12Buffer final : public Buffer
	{
	public:
		Handle<ID3D12Resource> resource;

		explicit D12Buffer(const BufferDesc& desc);
	private:
	};
}