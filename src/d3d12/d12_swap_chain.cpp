#include "d12_swap_chain.h"

#include <dxgi1_2.h>

#include "d12_device.h"
#include "d12_command_queue.h"

#include <wrl.h>

using Microsoft::WRL::ComPtr;

#undef max;
#undef min;

namespace light::rhi
{
	D12SwapChain::D12SwapChain(D12Device* device, HWND hwnd)
		: device_(device)
		, hwnd_(hwnd)
	{
		RECT window_rect;
		::GetClientRect(hwnd_, &window_rect);


		width_ = window_rect.right - window_rect.left;
		height_ = window_rect.top - window_rect.bottom;

		DXGI_SWAP_CHAIN_DESC1 desc{};
		desc.Width = width_;
		desc.Height = height_;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Stereo = FALSE;
		desc.SampleDesc = { 1,0 };
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = kBufferCount;
		desc.Scaling = DXGI_SCALING_STRETCH;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		// It is recommended to always allow tearing if tearing support is available.
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

		CommandQueue* queue = device_->GetCommandQueue(CommandListType::kDirect);
		auto d12_queue = CheckedCast<D12CommandQueue*>(queue);

		ComPtr<IDXGISwapChain1> swap_chain1;
		ThrowIfFailed( device_->GetDxgiFactory()->CreateSwapChainForHwnd(
			d12_queue->GetNative(), 
			hwnd_,
			&desc, 
			nullptr, 
			nullptr, 
			&swap_chain1));

		ComPtr<IDXGISwapChain4> swap_chain4;
		ThrowIfFailed(swap_chain1.As(&swap_chain4));

		dxgi_swap_chain_ = swap_chain4.Get();

		current_back_buffer_index_ = dxgi_swap_chain_->GetCurrentBackBufferIndex();

		UpdateRenderTargetViews();
	}

	UINT D12SwapChain::Present()
	{
		auto queue = device_->GetCommandQueue(CommandListType::kDirect);
		auto command_list = queue->GetCommandList();

		//todo
		queue->ExecuteCommandList(command_list);

		ThrowIfFailed(dxgi_swap_chain_->Present(0, 0));

		// 记录当前的同步量
		fence_values_[current_back_buffer_index_] = queue->Signal();

		current_back_buffer_index_ = dxgi_swap_chain_->GetCurrentBackBufferIndex();

		// 等待上一帧完成
		queue->WaitForFenceValue(fence_values_[current_back_buffer_index_]);

		return current_back_buffer_index_;
	}

	void D12SwapChain::Resize(uint32_t width, uint32_t height)
	{
		if(width_ != width || height_ != height)
		{
			width_ = std::max(1u, width);
			height_ = std::max(1u, height);

			for (uint32_t i = 0; i < kBufferCount; ++i)
			{
				back_buffer_textures_[i].Reset();
			}

			DXGI_SWAP_CHAIN_DESC desc{};
			ThrowIfFailed(dxgi_swap_chain_->GetDesc(&desc));
			ThrowIfFailed(dxgi_swap_chain_->ResizeBuffers(kBufferCount, width_, height_, desc.BufferDesc.Format, desc.Flags));

			current_back_buffer_index_ = dxgi_swap_chain_->GetCurrentBackBufferIndex();
		}
	}

	RenderTarget D12SwapChain::GetRenderTarget()
	{
		RenderTarget rt;
		rt.AttacthTexture(AttachmentPoint::kColor0, back_buffer_textures_[current_back_buffer_index_]);
		return rt;
	}

	void D12SwapChain::UpdateRenderTargetViews()
	{
		for (uint32_t i = 0; i < kBufferCount; ++i)
		{
			Handle<ID3D12Resource> back_buffer;
			ThrowIfFailed(dxgi_swap_chain_->GetBuffer(i, IID_PPV_ARGS(&back_buffer)));

			// todo:
			TextureDesc desc{};
			desc.width = width_;
			desc.height = height_;
			//GetDesc.format = 
#ifdef _DEBUG
			desc.debug_name = L"BackBuffer[" + std::to_wstring(i) + L"]";
#endif

			back_buffer_textures_[i] = device_->CreateTextureForNative(desc, back_buffer.Get());
		}
	}
}
