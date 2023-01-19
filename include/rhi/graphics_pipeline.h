#pragma once

#include "base.h"
#include "types.h"
#include "resource.h"
#include "render_target.h"
#include "shader.h"
#include "input_layout.h"
#include "binding_layout.h"

namespace light::rhi
{
	struct GraphicsPipelineDesc
	{
		PrimitiveTopology primitive_type = PrimitiveTopology::kTriangleList;
		InputLayoutHandle input_layout;
		BindingLayoutHandle binding_layout;

		ShaderHandle vs;
		ShaderHandle ps;
		ShaderHandle ds;
		ShaderHandle hs;
		ShaderHandle gs;

		RasterizerDesc rasterizer_state;
		BlendDesc blend_state;
		DepthStencilDesc depth_stencil_state;
	};

	class GraphicsPipeline : public Resource
	{
	public:
		GraphicsPipeline(const GraphicsPipelineDesc& desc,const RenderTarget& render_target)
			: desc_(desc)
			, render_target_(render_target)
		{
			
		}

		const GraphicsPipelineDesc& GetDesc() const { return desc_; }
	protected:
		GraphicsPipelineDesc desc_;
		RenderTarget render_target_;
	};

	using GraphicsPipelineHandle = Handle<GraphicsPipeline>;
	
}