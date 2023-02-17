#include "d12_graphics_pipeline.h"

#include "d12_device.h"
#include "d12_input_layout.h"


namespace light::rhi
{
	D12GraphicsPipeline::D12GraphicsPipeline(D12Device* device, const GraphicsPipelineDesc& desc, const RenderTarget& render_target, RootSignature* root_signature)
		: GraphicsPipeline(desc, render_target)
		, root_signature_(root_signature)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc{};

		ZeroMemory(&pso_desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

		auto input_layout = CheckedCast<D12InputLayout*>(desc.input_layout.Get());
		pso_desc.InputLayout = { input_layout->GetInputElements(),input_layout->NumElements() };

		pso_desc.pRootSignature = root_signature->GetNative();

		if (desc.vs)
		{
			auto& bytecode = desc.vs->GetBytecode();
			pso_desc.VS = { bytecode.data(),bytecode.size() };
		}

		if (desc.ps)
		{
			auto& bytecode = desc.ps->GetBytecode();
			pso_desc.PS = { bytecode.data(),bytecode.size() };
		}

		if (desc.hs)
		{
			auto& bytecode = desc.hs->GetBytecode();
			pso_desc.HS = { bytecode.data(),bytecode.size() };
		}

		if (desc.ds)
		{
			auto& bytecode = desc.ds->GetBytecode();
			pso_desc.DS = { bytecode.data(),bytecode.size() };
		}

		if (desc.gs)
		{
			auto& bytecode = desc.gs->GetBytecode();
			pso_desc.GS = { bytecode.data(),bytecode.size() };
		}

		switch (desc.primitive_type)
		{
		case PrimitiveTopology::kPointList:
			pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
			break;
		case PrimitiveTopology::kLineList:
			pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			break;
		case PrimitiveTopology::kTriangleList:
		case PrimitiveTopology::kTriangleStrip:
			pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			break;
		case PrimitiveTopology::kPatchList:
			pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
			break;
		}

		pso_desc.RasterizerState = ConvertRasterizeDesc(desc.rasterizer_state);
		pso_desc.BlendState = ConvertBlendDesc(desc.blend_state);
		pso_desc.DepthStencilState = ConvertDepthStencilDesc(desc.depth_stencil_state);
		pso_desc.SampleMask = UINT_MAX;
		pso_desc.NumRenderTargets = render_target_.GetNumColors();

		const auto& attachments = render_target_.GetAttachments();
		for (uint32_t i = 0; i < pso_desc.NumRenderTargets; ++i)
		{
			if (attachments[i].texture)
			{
				pso_desc.RTVFormats[i] = GetDxgiFormatMapping(attachments[i].texture->GetDesc().format).rtv_format;
			}
		}

		const auto& depth_attachment = render_target_.GetAttachment(AttachmentPoint::kDepthStencil);
		if (depth_attachment.texture)
		{
			pso_desc.DSVFormat = GetDxgiFormatMapping(depth_attachment.texture->GetDesc().format).rtv_format;
		}

		SampleDesc sample_desc = render_target_.GetSampleDesc();
		pso_desc.SampleDesc.Count = sample_desc.count;
		pso_desc.SampleDesc.Quality = sample_desc.quality;

		ThrowIfFailed(device->GetNative()->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pipeline_state_)));
	}
}