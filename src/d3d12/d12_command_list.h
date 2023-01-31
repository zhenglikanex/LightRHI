#pragma once

#include <vector>

#include "rhi/command_list.h"

#include "upload_buffer.h"
#include "resource_state_tracker.h"
#include "dynamic_descriptor_heap.h"

#include "d3dx12.h"

namespace light::rhi
{
	class D12Device;
	class D12CommandQueue;

	class D12CommandList final : public CommandList
	{
	public:
		D12CommandList(D12Device* device,CommandListType type,CommandQueue* queue);
		~D12CommandList() override;

		void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type,ID3D12DescriptorHeap* heap);

		void TransitionBarrier(Buffer* buffer, ResourceStates state_afeter, uint32_t subresource = ~0, bool flush_barriers = false,
		                       bool permanent = true) override;

		void TransitionBarrier(Texture* texture, ResourceStates state_afeter, uint32_t subresource = ~0, bool flush_barriers = false,
		                       bool permanent = true) override;

		void ClearTexture(Texture* texture, const float* clear_value) override;

		void ClearDepthStencilTexture(Texture* texture, ClearFlags clear_flags, float depth, uint8_t stencil) override;

		void WriteBuffer(Buffer* buffer, const uint8_t* data, uint64_t size, uint64_t dest_offset_bytes = 0) override;

		void SetGraphicsDynamicConstantBuffer(uint32_t parameter_index, size_t bytes, const void* data) override;

		void SetGraphics32BitConstants(uint32_t parameter_index, uint32_t num_constants,const void* constants) override;

		void SetGraphicsPipeline(GraphicsPipeline* pso) override;

		void SetVertexBuffer(uint32_t slot, Buffer* buffer) override;

		void SetIndexBuffer(Buffer* buffer) override;

		void SetRednerTarget(const RenderTarget& render_target) override;

		void SetViewport(const Viewport& viewport) override;

		void SetViewports(const std::vector<Viewport>& viewports) override;

		void SetScissorRect(const Rect& rect) override;

		void SetScissorRects(const std::vector<Rect>& rects) override;

		void ExecuteCommandList() override;

		bool Close(CommandList* pending_command_list) override;

		void Close() override;

		void Reset() override;

		void DrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t start_index, int32_t base_vertex,
			uint32_t start_instance) override;

		ID3D12GraphicsCommandList* GetD3D12GraphicsCommandList() { return d3d12_command_list_; }

	protected:
		void CommitDescriptorHeaps();

		// �Զ�׷��ʹ���е���Դ��������
		void TrackResource(Resource* resource) override;

		void FlushResourceBarriers() override;
		
	private:
		D12Device* device_;
		Handle<ID3D12CommandAllocator> d3d12_command_allocator_;
		Handle<ID3D12GraphicsCommandList> d3d12_command_list_;
		std::vector<ResourceHandle> track_resources_;
		std::vector<Handle<ID3D12Resource>> track_upload_resources_;
		UploadBuffer upload_buffer_;
		ResourceStateTracker resource_state_tracker_;
		std::unique_ptr<DynamicDescriptorHeap> dynamic_descriptor_heaps_[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
		GraphicsPipeline* current_pso_;
		ID3D12DescriptorHeap* descriptr_heaps_[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
	};

}
