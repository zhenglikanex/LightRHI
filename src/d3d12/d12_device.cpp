#include "d12_device.h"
#include "d12_buffer.h"
#include "d12_texture.h"

#include <codecvt>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h> // For HRESULT
#include <comdef.h> // For _com_error class (used to decode HR result codes).

namespace light::rhi
{
	void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			_com_error err(hr);
			OutputDebugString(err.ErrorMessage());

			std::wstring msg = err.ErrorMessage();
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

			throw std::exception(converter.to_bytes(msg).c_str());
		}
	}

	SwapChainHandle D12Device::CreateSwapChian(HWND hwnd)
	{
		return MakeHandle<D12SwapChain>(this, hwnd);
	}

	BufferHandle D12Device::CreateBuffer(BufferDesc desc)
	{
		if(desc.type == BufferType::kConstant)
		{
			desc.byte = Align(desc.byte, kConstantAlignSize);
		}

		return MakeHandle<D12Buffer>(this,desc);
	}

	TextureHandle D12Device::CreateTexture(TextureDesc desc)
	{
		return MakeHandle<D12Texture>(this, desc);
	}

	TextureHandle D12Device::CreateTextureForNative(TextureDesc desc, void* resource)
	{
		return MakeHandle<D12Texture>(this, desc, static_cast<ID3D12Resource*>(resource));
	}

	InputLayoutHandle D12Device::CreateInputLayout(std::vector<VertexAttributeDesc> attributes)
	{
		return MakeHandle<D12InputLayout>(this, std::move(attributes));
	}

	GraphicsPipelineHandle D12Device::CreateGraphicsPipeline(GraphicsPipelineDesc desc, const RenderTarget& render_target)
	{
		return MakeHandle<D12GraphicsPipeline>(this, desc, render_target, GetRootSignature(desc.binding_layout,desc.input_layout != nullptr));
	}

	CommandQueue* D12Device::GetCommandQueue(CommandListType type)
	{
		return queues_[static_cast<uint32_t>(type)];
	}

	CommandList* D12Device::GetCommandList(CommandListType type)
	{
	 	return  queues_[static_cast<size_t>(type)]->GetCommandList();
	}

	RootSignatureHandle D12Device::GetRootSignature(BindingLayout* binding_layout, bool allow_input_layout)
	{
		size_t hash = 0;
		HashCombine(hash, binding_layout);
		HashCombine(hash, allow_input_layout ? 1 : 0);

		if(auto it = root_signature_cache_.find(hash); it != root_signature_cache_.end())
		{
			RootSignatureHandle handle(it->second);
			return handle;
		} 

		return MakeHandle<RootSignature>(this, hash, binding_layout, allow_input_layout);
	}

	void D12Device::ReleaseRootSignature(const RootSignature* root_signature)
	{
		if(root_signature)
		{
			root_signature_cache_.erase(root_signature->GetHash());
		}
	}

	uint32_t D12Device::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const
	{
		return device_->GetDescriptorHandleIncrementSize(type);
	}
}
