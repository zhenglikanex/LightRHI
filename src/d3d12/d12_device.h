#pragma once

#include "rhi/device.h"
#include "d3dx12.h"

namespace light::rhi
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw std::exception();
		}
	}

	class D12Device : public Device
	{
	public:
		D12Device() = default;

		~D12Device() override;
		BufferHandle CreateBuffer(const BufferDesc& desc) override;
	private:
		Handle<ID3D12Device> device_;
	};
}