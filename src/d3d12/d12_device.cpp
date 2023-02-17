#include "d12_device.h"
#include "d12_buffer.h"
#include "d12_texture.h"

#include <codecvt>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h> // For HRESULT
#include <comdef.h> // For _com_error class (used to decode HR result codes).
#include "d3dx12.h"
#include <D3Dcompiler.h>

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

	D12Device::D12Device()
	{
#if defined(DEBUG) || defined(_DEBUG)
		{
			Handle<ID3D12Debug> debug_controller;
			ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller)));
			debug_controller->EnableDebugLayer();
		}
#endif

		Microsoft::WRL::ComPtr<IDXGIFactory> dxgi_factory;
		ThrowIfFailed(CreateDXGIFactory(IID_PPV_ARGS(&dxgi_factory)));

		ThrowIfFailed(dxgi_factory.As(&dxgi_factory_));

		//try to create hardware device
		HRESULT rt = D3D12CreateDevice(
			nullptr,			// default adapter
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&device_));

		// Fallback to WARP device
		if (FAILED(rt))
		{
			// 回退到软渲染器
			Handle<IDXGIAdapter> warp_adapter;
			ThrowIfFailed(dxgi_factory_->EnumWarpAdapter(IID_PPV_ARGS(&warp_adapter)));

			ThrowIfFailed(D3D12CreateDevice(warp_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device_)));
		}

		queues_[static_cast<size_t>(CommandListType::kDirect)] = MakeHandle<D12CommandQueue>(this, CommandListType::kDirect);
		/*queues_[static_cast<size_t>(CommandListType::kCompute)] = MakeHandle<D12CommandQueue>(this, CommandListType::kCompute);
		queues_[static_cast<size_t>(CommandListType::kCopy)] = MakeHandle<D12CommandQueue>(this, CommandListType::kCopy);*/

		ThrowIfFailed(device_->GetDeviceRemovedReason());

		for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
		{
			descriptor_allocators_[i] = 
				std::make_unique<DescriptorAllocator>(this, static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
		}
	}

	D12Device::~D12Device()
	{
		Flush();
	}

	SwapChainHandle D12Device::CreateSwapChian(HWND hwnd)
	{
		return MakeHandle<D12SwapChain>(this, hwnd);
	}

	ShaderHandle D12Device::CreateShader(ShaderType type, const std::string& filename, const std::string& entrypoint, const std::string& target)
	{
		UINT compile_flags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
		compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		std::wstring wfilename(filename.cbegin(), filename.cend());

		HRESULT hr = S_OK;
		Handle<ID3DBlob> byte_code = nullptr;
		Handle<ID3DBlob> errors;
		// todo:加入shader_macro
		hr = D3DCompileFromFile(wfilename.c_str(),nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entrypoint.c_str(), target.c_str(), compile_flags, 0, &byte_code, &errors);

		if (errors != nullptr)
			OutputDebugStringA((char*)errors->GetBufferPointer());

		ThrowIfFailed(hr);

		std::vector<char> vbyte_code(byte_code->GetBufferSize());
		memcpy(vbyte_code.data(), byte_code->GetBufferPointer(), vbyte_code.size());

		return Device::CreateShader(type, std::move(vbyte_code));
	}

	BufferHandle D12Device::CreateBuffer(BufferDesc desc)
	{
		if(desc.type == BufferType::kConstant)
		{
			desc.byte = Align(desc.byte, kConstantAlignSize);
		}

		return MakeHandle<D12Buffer>(this,desc);
	}

	TextureHandle D12Device::CreateTexture(const TextureDesc& desc)
	{
		return MakeHandle<D12Texture>(this, desc);
	}

	TextureHandle D12Device::CreateTextureForNative(const TextureDesc& desc, void* resource)
	{
		auto tex = MakeHandle<D12Texture>(this, desc, static_cast<ID3D12Resource*>(resource));
		return tex;
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

	CommandListHandle D12Device::GetCommandList(CommandListType type)
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

	DescriptorAllocation D12Device::AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t num_descriptors)
	{
		return descriptor_allocators_[type]->Allocate(num_descriptors);
	}

	void D12Device::Flush()
	{
		for(auto queue : queues_)
		{
			if(queue)
			{
				queue->Flush();
			}
		}
	}

	void D12Device::ReleaseRootSignature(const RootSignature* root_signature)
	{
		if(root_signature)
		{
			root_signature_cache_.erase(root_signature->GetHash());
		}
	}

	void D12Device::ReleaseStaleDescriptors()
	{
		for(auto& descriptor_allocator: descriptor_allocators_)
		{
			descriptor_allocator->ReleaseStaleDescriptors();
		}
	}

	uint32_t D12Device::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const
	{
		return device_->GetDescriptorHandleIncrementSize(type);
	}
}
