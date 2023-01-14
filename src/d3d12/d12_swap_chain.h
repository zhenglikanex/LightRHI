#pragma once

#include "Windows.h"
#include "rhi/swap_chain.h"

#include <dxgi1_5.h>
#include "d3dx12.h"

#include "d12_texture.h"

namespace light::rhi
{
	class D12Device;

	class D12SwapChain final : public SwapChain
	{
	public:
		static constexpr uint32_t kBufferCount = 2;

		D12SwapChain(D12Device* device, HWND hwnd);

		uint32_t Present() override;

		void Resize(uint32_t width, uint32_t height) override;

		RenderTarget GetRenderTarget() override;
	private:
		void UpdateRenderTargetViews();
	
		D12Device* device_;
		HWND hwnd_;
		uint32_t width_;
		uint32_t height_;
		Handle<IDXGISwapChain4> dxgi_swap_chain_;
		Handle<D12Texture> back_buffer_textures_[kBufferCount];
		uint32_t current_back_buffer_index_;
		uint64_t fence_values_[kBufferCount];
	};
}