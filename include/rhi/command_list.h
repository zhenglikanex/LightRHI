#pragma once

#include <vector>

#include "types.h"
#include "resource.h"
#include "render_target.h"

namespace light::rhi
{
	class Buffer;
	class Texture;
	class GraphicsPipeline;
	class CommandQueue;

	class CommandList : public Resource
	{
	public:
		CommandList(CommandListType type,CommandQueue* queue)
			: type_(type)
			, queue_(queue)
		{

		}

		virtual void TransitionBarrier(Buffer* buffer, ResourceStates state_afeter, uint32_t subresource = ~0, bool flush_barriers = false, bool permanent = true) = 0;

		virtual void TransitionBarrier(Texture* texture, ResourceStates state_afeter, uint32_t subresource = ~0, bool flush_barriers = false, bool permanent = true) = 0;

		virtual void ClearTexture(Texture* texture, const float* clear_value) = 0;

		virtual void ClearDepthStencilTexture(Texture* texture, ClearFlags clear_flags,float depth,uint8_t stencil) = 0;

		virtual void WriteBuffer(Buffer* buffer, const uint8_t* data, uint64_t size, uint64_t dest_offset_bytes = 0) = 0;

		virtual void SetGraphicsDynamicConstantBuffer(uint32_t parameter_index, size_t bytes, const void* data) = 0;
		template<class T>
		void SetGraphicsDynamicConstantBuffer(uint32_t parameter_index,const T& data)
		{
			SetGraphicsDynamicConstantBuffer(parameter_index, sizeof(T), &data);
		}

		virtual void SetGraphics32BitConstants(uint32_t parameter_index, uint32_t num_constants, const void* constants) = 0;
		template<class T>
		void SetGraphics32BitConstants(uint32_t parameter_index,const T& constants)
		{
			static_assert(sizeof(T) % sizeof(uint32_t) == 0, "Size of type must be a multiple of 4 bytes");
			SetGraphics32BitConstants(parameter_index, sizeof(T) / sizeof(uint32_t), &constants);
		}

		virtual void SetBufferView(uint32_t parameter_index, Buffer* buffer,uint32_t offset = 0, ResourceStates state_after = ResourceStates::kVertexAndConstantBuffer) = 0;

		virtual void SetConstantBufferView(uint32_t parameter_index, uint32_t descriptor_offset, Buffer* buffer, uint32_t offset = 0, ResourceStates state_after = ResourceStates::kVertexAndConstantBuffer) = 0;

		virtual void SetConstantBufferView(uint32_t parameter_index, uint32_t descriptor_offset, Buffer* buffer, uint32_t offset, uint32_t byte_size, ResourceStates state_after = ResourceStates::kVertexAndConstantBuffer) = 0;

		virtual void SetStructBufferView(uint32_t parameter_index, uint32_t descriptor_offset, Buffer* buffer, uint32_t offset = 0, ResourceStates state_after = ResourceStates::kGenericRead) = 0;

		virtual void SetStructBufferView(uint32_t parameter_index, uint32_t descriptor_offset, Buffer* buffer, uint32_t offset, uint32_t byte_size, ResourceStates state_after = ResourceStates::kGenericRead) = 0;

		virtual void SetUnoderedAccessBufferView(uint32_t parameter_index, uint32_t descriptor_offset, Buffer* buffer, uint32_t offset = 0, ResourceStates state_after = ResourceStates::kUnorderedAccess) = 0;

		virtual void SetUnoderedAccessBufferView(uint32_t parameter_index, uint32_t descriptor_offset, Buffer* buffer, uint32_t offset, uint32_t byte_size, ResourceStates state_after = ResourceStates::kUnorderedAccess) = 0;

		//virtual void SetShaderResourceView(uint32_t parameter_index,Texture* texture,ResourceStates state_after = ResourceStates::)

		virtual void SetGraphicsPipeline(GraphicsPipeline* pso) = 0;

		virtual void SetVertexBuffer(uint32_t slot, Buffer* buffer) = 0;

		virtual void SetIndexBuffer(Buffer* buffer) = 0;

		virtual void SetRenderTarget(const RenderTarget& target) = 0;

		virtual void SetViewport(const Viewport& viewport) = 0;

		virtual void SetViewports(const std::vector<Viewport>& viewports) = 0;

		virtual void SetScissorRect(const Rect& rect) = 0;

		virtual void SetScissorRects(const std::vector<Rect>& rects) = 0;

		virtual void ExecuteCommandList() = 0;

		virtual bool Close(CommandList* pending_command_list) = 0;

		virtual void Close() = 0;

		virtual void Reset() = 0;

		virtual void DrawIndexed(uint32_t index_count,uint32_t instance_count,uint32_t start_index,int32_t base_vertex,uint32_t start_instance) = 0;

	protected:

		virtual void TrackResource(Resource* resource) = 0;

		virtual void FlushResourceBarriers() = 0;

		CommandListType type_;
		CommandQueue* queue_;
	};

	using CommandListHandle = Handle<CommandList>;
}
