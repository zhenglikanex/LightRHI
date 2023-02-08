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
		virtual TextureHandle CreateTexture(const TextureDesc& desc) = 0;
		virtual TextureHandle CreateTextureForNative(const TextureDesc& desc, void* resource) = 0;
		virtual InputLayoutHandle CreateInputLayout(std::vector<VertexAttributeDesc> attributes) = 0;
		virtual GraphicsPipelineHandle CreateGraphicsPipeline(GraphicsPipelineDesc desc, const RenderTarget& render_target) = 0;

		virtual CommandQueue* GetCommandQueue(CommandListType type) = 0;
		virtual CommandListHandle GetCommandList(CommandListType type) = 0;

		virtual void Flush() = 0;
	};

	using DeviceHandle = Handle<Device>;

	//------------------------------------------------------------------------------------------------
	inline uint32_t CalcSubresource(uint32_t mip_level, uint32_t array_slice, uint32_t mip_levels)
	{
		return mip_level + array_slice * mip_levels;
	}
}