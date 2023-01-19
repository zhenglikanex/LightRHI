#pragma once

#include "base.h"
#include "resource.h"
#include "buffer.h"
#include "texture.h"
#include "graphics_pipeline.h"
#include "render_target.h"
#include "command_queue.h"
#include "command_list.h"
#include "types.h"

namespace light::rhi
{
	class Device : public Resource
	{
	public:
		virtual GraphicsApi GetGraphicsApi() const = 0;

		virtual BufferHandle CreateBuffer(BufferDesc desc) = 0;
		virtual TextureHandle CreateTexture(TextureDesc desc) = 0;
		virtual TextureHandle CreateTextureForNative(TextureDesc desc, void* resource) = 0;
		virtual InputLayoutHandle CreateInputLayout(std::vector<VertexAttributeDesc> attributes) = 0;
		virtual GraphicsPipelineHandle CreateGraphicsPipeline(GraphicsPipelineDesc desc, const RenderTarget& render_target) = 0;

		virtual CommandQueue* GetCommandQueue(CommandListType type) = 0;
		virtual CommandList* GetCommandList(CommandListType type) = 0;
	};

	using DeviceHandle = Handle<Device>;
}