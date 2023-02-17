#pragma once

#include <unordered_map>

#include "rhi/device.h"

#include "d12_convert.h"
#include "d12_command_list.h"
#include "d12_command_queue.h"
#include "d12_buffer.h"
#include "d12_input_layout.h"
#include "d12_graphics_pipeline.h"
#include "d12_swap_chain.h"
#include "root_signature.h"
#include "descriptor_allocator.h"

#include <dxgi1_5.h>
#include <wrl/client.h>

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"D3D12.lib")
#pragma comment(lib,"dxgi.lib")

namespace light::rhi
{
	constexpr uint32_t kConstantAlignSize = 256ull;

	inline void ThrowIfFailed(HRESULT hr);
	

	class D12Device final : public Device
	{
	public:
		D12Device();

		~D12Device() override;

		GraphicsApi GetGraphicsApi() const override { return GraphicsApi::kD3D12; }

		ID3D12Device* GetNative() noexcept { return device_; }

		SwapChainHandle CreateSwapChian(HWND hwnd);

		ShaderHandle CreateShader(ShaderType type, const std::string& filename,const std::string& entrypoint, const std::string& target) override;

		BufferHandle CreateBuffer(BufferDesc desc) override;

		TextureHandle CreateTexture(const TextureDesc& desc) override;

		TextureHandle CreateTextureForNative(const TextureDesc& desc, void* resource) override;

		InputLayoutHandle CreateInputLayout(std::vector<VertexAttributeDesc> attributes) override;

		GraphicsPipelineHandle CreateGraphicsPipeline(GraphicsPipelineDesc desc, const RenderTarget& render_target) override;

		CommandQueue* GetCommandQueue(CommandListType type) override;

		CommandListHandle GetCommandList(CommandListType type) override;

		void Flush() override;

		IDXGIFactory5* GetDxgiFactory() { return dxgi_factory_.Get(); }

		RootSignatureHandle GetRootSignature(BindingLayout* binding_layout, bool allow_input_layout);

		DescriptorAllocation AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t num_descriptors);


		void ReleaseRootSignature(const RootSignature* root_signature);

		void ReleaseStaleDescriptors();

		uint32_t GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;
	private:
		Handle<ID3D12Device> device_;
		Microsoft::WRL::ComPtr<IDXGIFactory5> dxgi_factory_;
		std::array<Handle<D12CommandQueue>, static_cast<size_t>(CommandListType::kCopy) + 1> queues_;
		std::unordered_map<size_t, RootSignature*> root_signature_cache_;
		std::array<std::unique_ptr<DescriptorAllocator>, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> descriptor_allocators_;
	};
}