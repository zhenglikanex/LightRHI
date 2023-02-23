#pragma once

#include <d3d12.h>

#include "rhi/graphics_pipeline.h"

#include "root_signature.h"

namespace light::rhi
{
	class D12Device;

	class D12GraphicsPipeline final : public GraphicsPipeline
	{
	public:
		D12GraphicsPipeline(D12Device* device, const GraphicsPipelineDesc& desc, const RenderTarget& render_target, RootSignature* root_signature);

		ID3D12PipelineState* GetNative() { return pipeline_state_; }
		RootSignature* GetRootSignature() { return root_signature_; }
	private:
		Handle<ID3D12PipelineState> pipeline_state_;
		RootSignatureHandle root_signature_;
	};
}