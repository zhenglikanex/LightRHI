#include "d12_swap_chain.h"

#include <dxgi1_2.h>

#include "d12_device.h"
#include "d12_command_queue.h"

#include <wrl.h>

#include "auto_timer.h"

using Microsoft::WRL::ComPtr;

namespace light::rhi
{
	D12SwapChain::D12SwapChain(D12Device* device, HWND hwnd)
		: device_(device)
		, hwnd_(hwnd)
		, fence_values_{0}
	{
		command_queue_ = device_->GetCommandQueue(CommandListType::kDirect);

		auto d12_queue = CheckedCast<D12CommandQueue*>(command_queue_);

		RECT window_rect;
		::GetClientRect(hwnd_, &window_rect);

		width_ = window_rect.right - window_rect.left;
		height_ = window_rect.bottom - window_rect.top;

		bool allow_tearing = false;
		if (SUCCEEDED(device_->GetDxgiFactory()->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing, sizeof(bool))))
		{
			allow_tearing = true;
		}

		DXGI_SWAP_CHAIN_DESC1 desc{};
		desc.Width = width_;
		desc.Height = height_;
		desc.Format = GetDxgiFormatMapping(kBufferForamt).rtv_format;
		desc.Stereo = FALSE;
		desc.SampleDesc = { 1,0 };
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = kBufferCount;
		desc.Scaling = DXGI_SCALING_STRETCH;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		// It is recommended to always allow tearing if tearing support is available.
		desc.Flags = allow_tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		desc.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

		ComPtr<IDXGISwapChain1> swap_chain1;
		 device_->GetDxgiFactory()->CreateSwapChainForHwnd(
			d12_queue->GetNative(), 
			hwnd_,
			&desc, 
			nullptr, 
			nullptr, 
			&swap_chain1);

		ComPtr<IDXGISwapChain4> swap_chain4;
		ThrowIfFailed(swap_chain1.As(&swap_chain4));

		dxgi_swap_chain_ = swap_chain4.Get();

		current_back_buffer_index_ = dxgi_swap_chain_->GetCurrentBackBufferIndex();

		UpdateRenderTargetViews();
	}

	UINT D12SwapChain::Present()
	{
		auto command_list = command_queue_->GetCommandList();
		command_list->TransitionBarrier(back_buffer_textures_[current_back_buffer_index_], ResourceStates::kPresent);
		command_list->ExecuteCommandList();

		ThrowIfFailed(dxgi_swap_chain_->Present(0, 0));

		// 记录当前的同步量
		fence_values_[current_back_buffer_index_] = command_queue_->Signal();

		current_back_buffer_index_ = dxgi_swap_chain_->GetCurrentBackBufferIndex();

		// 等待上一帧
		command_queue_->WaitForFenceValue(fence_values_[current_back_buffer_index_]);

		return current_back_buffer_index_;
	}

	void D12SwapChain::Resize(uint32_t width, uint32_t height)
	{
		if(width_ != width || height_ != height)
		{
			width_ = std::max(1u, width);
			height_ = std::max(1u, height);

			device_->Flush();

			for (uint32_t i = 0; i < kBufferCount; ++i)
			{
				back_buffer_textures_[i].Reset();
			}

			DXGI_SWAP_CHAIN_DESC desc{};
			ThrowIfFailed(dxgi_swap_chain_->GetDesc(&desc));
			ThrowIfFailed(dxgi_swap_chain_->ResizeBuffers(kBufferCount, width_, height_, desc.BufferDesc.Format, desc.Flags));

			current_back_buffer_index_ = dxgi_swap_chain_->GetCurrentBackBufferIndex();

			UpdateRenderTargetViews();
		}
	}

	RenderTarget D12SwapChain::GetRenderTarget()
	{
		RenderTarget rt;
		rt.AttacthAttachment(AttachmentPoint::kColor0, back_buffer_textures_[current_back_buffer_index_]);
		return rt;
	}

	void D12SwapChain::UpdateRenderTargetViews()
	{
		for (uint32_t i = 0; i < kBufferCount; ++i)
		{
			Handle<ID3D12Resource> back_buffer;
			ThrowIfFailed(dxgi_swap_chain_->GetBuffer(i, IID_PPV_ARGS(&back_buffer)));

			TextureDesc desc{};
			desc.width = width_;
			desc.height = height_;
			desc.format = Format::RGBA8_UNORM;
#ifdef _DEBUG
			//todo
			//desc.debug_name = L"BackBuffer[" + std::to_wstring(i) + L"]";
#endif

			back_buffer_textures_[i] = device_->CreateTextureForNative(desc, back_buffer.Get());
		}
	}
}
