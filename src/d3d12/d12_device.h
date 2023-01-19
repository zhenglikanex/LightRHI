#pragma once

#include <unordered_map>

#include "rhi/device.h"

#include <dxgi1_5.h>
#include "d3dx12.h"
#include "d12_convert.h"
#include "d12_command_list.h"
#include "d12_command_queue.h"
#include "d12_buffer.h"
#include "d12_input_layout.h"
#include "d12_graphics_pipeline.h"
#include "d12_swap_chain.h"
#include "root_signature.h"


namespace light::rhi
{
	constexpr uint64_t kConstantAlignSize = 256ull;

	inline void ThrowIfFailed(HRESULT hr);
	

	class D12Device final : public Device
	{
	public:
		D12Device() = default;

		~D12Device() override = default;

		GraphicsApi GetGraphicsApi() const override { return GraphicsApi::kD3D12; }

		ID3D12Device* GetNative() noexcept { return device_; }

		SwapChainHandle CreateSwapChian(HWND hwnd);

		BufferHandle CreateBuffer(BufferDesc desc) override;

		TextureHandle CreateTexture(TextureDesc desc) override;

		TextureHandle CreateTextureForNative(TextureDesc desc, void* resource) override;

		InputLayoutHandle CreateInputLayout(std::vector<VertexAttributeDesc> attributes) override;

		GraphicsPipelineHandle CreateGraphicsPipeline(GraphicsPipelineDesc desc, const RenderTarget& render_target) override;

		CommandQueue* GetCommandQueue(CommandListType type) override;

		CommandList* GetCommandList(CommandListType type) override;

		IDXGIFactory5* GetDxgiFactory() { return dxgi_factory_; }

		RootSignatureHandle GetRootSignature(BindingLayout* binding_layout, bool allow_input_layout);

		void ReleaseRootSignature(const RootSignature* root_signature);

		uint32_t GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;
	private:
		Handle<ID3D12Device> device_;
		Handle<IDXGIFactory5> dxgi_factory_;
		Handle<D12CommandQueue> queues_[static_cast<size_t>(CommandListType::kCopy) + 1];
		std::unordered_map<size_t, RootSignature*> root_signature_cache_;
	};
}