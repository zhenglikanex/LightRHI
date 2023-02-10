#pragma once

#include "rhi/types.h"
#include "d3dx12.h"

namespace light::rhi
{
	inline D3D12_RESOURCE_STATES ConvertResourceState(ResourceStates state)
	{
		return static_cast<D3D12_RESOURCE_STATES>(state);
	}

	inline D3D12_COMMAND_LIST_TYPE ConvertCommandListType(CommandListType type)
	{
		switch (type) {
		case CommandListType::kDirect: return D3D12_COMMAND_LIST_TYPE_DIRECT;
		case CommandListType::kCompute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		case CommandListType::kCopy: return D3D12_COMMAND_LIST_TYPE_COPY;
		}
		return {};
	}

	inline D3D12_CLEAR_FLAGS ConvertClearFlags(ClearFlags flags)
	{
		return static_cast<D3D12_CLEAR_FLAGS>(flags);
	}

	inline D3D12_VIEWPORT ConvertViewport(const Viewport& viewport)
	{
		D3D12_VIEWPORT d12_viewport{};

		d12_viewport.Width = viewport.width;
		d12_viewport.Height = viewport.height;
		d12_viewport.MinDepth = viewport.min_depth;
		d12_viewport.MaxDepth = viewport.max_depth;
		d12_viewport.TopLeftX = viewport.top_left_x;
		d12_viewport.TopLeftY = viewport.top_left_y;

		return d12_viewport;
	}

	inline D3D12_RECT ConvertRect(const Rect& rect)
	{
		D3D12_RECT d12_rect;
		d12_rect.bottom = rect.bottom;
		d12_rect.top = rect.top;
		d12_rect.left = rect.left;
		d12_rect.right = rect.right;

		return d12_rect;
	}

	inline D3D12_PRIMITIVE_TOPOLOGY_TYPE ConvertPrimitiveTopology(PrimitiveTopology primitive)
	{
		switch (primitive) {
		case PrimitiveTopology::kPointList: 
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		case PrimitiveTopology::kLineList: 
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case PrimitiveTopology::kTriangleList: 
		case PrimitiveTopology::kTriangleStrip: 
		case PrimitiveTopology::kTriangleFan: 
		case PrimitiveTopology::kTriangleListWithAdjacency: 
		case PrimitiveTopology::kTriangleStripWithAdjacency: 
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		case PrimitiveTopology::kPatchList: 
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
		default:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
		}
	}

	inline D3D12_RASTERIZER_DESC ConvertRasterizeDesc(const RasterizerDesc& desc)
	{
		D3D12_RASTERIZER_DESC d12_desc{};
		d12_desc.FillMode = static_cast<D3D12_FILL_MODE>(desc.fill_mode);
		d12_desc.CullMode = static_cast<D3D12_CULL_MODE>(desc.cull_mode);
		d12_desc.FrontCounterClockwise = desc.front_counter_clockwise;
		d12_desc.DepthBias = desc.depth_bias;
		d12_desc.SlopeScaledDepthBias = desc.slope_scaled_depth_bias;
		d12_desc.DepthClipEnable = desc.depth_clip_enable;
		d12_desc.MultisampleEnable = desc.multisample_enable;
		d12_desc.AntialiasedLineEnable = desc.antialiased_line_enable;
		d12_desc.ForcedSampleCount = desc.forced_sample_count;
		d12_desc.ConservativeRaster = static_cast<D3D12_CONSERVATIVE_RASTERIZATION_MODE>(desc.conservative_raster);
		return d12_desc;
	}

	inline D3D12_BLEND_DESC ConvertBlendDesc(const BlendDesc& desc)
	{
		D3D12_BLEND_DESC d12_desc{};

		d12_desc.AlphaToCoverageEnable = desc.alpha_to_coverage_enable;
		d12_desc.IndependentBlendEnable = desc.independent_blend_enable;

		for (uint32_t i = 0; i < 8; ++i)
		{
			auto& out = d12_desc.RenderTarget[i];
			auto& in = desc.render_target[i];

			out.BlendEnable = in.blend_enable;
			out.LogicOpEnable = false;
			out.SrcBlend = static_cast<D3D12_BLEND>(in.src_blend);
			out.DestBlend = static_cast<D3D12_BLEND>(in.dest_blend);
			out.BlendOp = static_cast<D3D12_BLEND_OP>(in.blend_op);
			out.SrcBlendAlpha = static_cast<D3D12_BLEND>(in.src_blend_alpha);
			out.DestBlendAlpha = static_cast<D3D12_BLEND>(in.dest_blend_alpha);
			out.LogicOp = D3D12_LOGIC_OP_NOOP;
			out.RenderTargetWriteMask = static_cast<uint8_t>(in.render_target_write_mask);
		}

		return d12_desc;
	}

	inline D3D12_DEPTH_STENCILOP_DESC ConvertDepthStencilOpDesc(const DepthStencilDesc::DepthStencilOpDesc& in)
	{
		D3D12_DEPTH_STENCILOP_DESC out{};
		out.StencilFailOp = static_cast<D3D12_STENCIL_OP>(in.stencil_fail_op);
		out.StencilDepthFailOp = static_cast<D3D12_STENCIL_OP>(in.stencil_depth_fail_op);
		out.StencilPassOp = static_cast<D3D12_STENCIL_OP>(in.stencil_pass_op);
		out.StencilFunc = static_cast<D3D12_COMPARISON_FUNC>(in.stencil_func);

		return out;
	}

	inline D3D12_DEPTH_STENCIL_DESC ConvertDepthStencilDesc(const DepthStencilDesc& in)
	{
		D3D12_DEPTH_STENCIL_DESC out{};
		out.DepthEnable = in.depth_enable;
		out.DepthWriteMask = static_cast<D3D12_DEPTH_WRITE_MASK>(in.depth_write_mask);
		out.DepthFunc = static_cast<D3D12_COMPARISON_FUNC>(in.depth_func);
		out.StencilEnable = in.stencil_enable;
		out.StencilReadMask = in.stencil_read_mask;
		out.StencilWriteMask = in.stencil_write_mask;
		out.FrontFace = ConvertDepthStencilOpDesc(in.front_face);
		out.BackFace = ConvertDepthStencilOpDesc(in.back_face);

		return out;
	}

	inline D3D12_SHADER_VISIBILITY ConvertShaderVisibility(ShaderVisibility visibility)
	{
		
		return static_cast<D3D12_SHADER_VISIBILITY>(visibility);
	}

	inline D3D12_DESCRIPTOR_RANGE_TYPE ConvertDescriptorRangeType(DescriptorRangeType type)
	{
		return static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(type);
	}

	inline D3D12_RESOURCE_STATES ConvertResourceStates(ResourceStates states)
	{
		return static_cast<D3D12_RESOURCE_STATES>(states);
	}

	struct DxgiFormatMapping
	{
		Format abstract_format;
		DXGI_FORMAT resource_format;
		DXGI_FORMAT srv_format;
		DXGI_FORMAT rtv_format;
	};

	const DxgiFormatMapping& GetDxgiFormatMapping(Format abstract_format);
}
