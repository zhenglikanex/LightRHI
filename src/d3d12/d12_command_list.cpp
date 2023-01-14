#include "d12_command_list.h"

#include <array>

#include "d12_device.h"
#include "d12_texture.h"

namespace light::rhi
{
	D12CommandList::D12CommandList(D12Device* device, CommandListType type,CommandQueue* queue)
		: CommandList(type,queue)
		, device_(device)
		, upload_buffer_(device_)
		, current_pso_(nullptr)
	{
		ThrowIfFailed(device_->GetNative()->CreateCommandAllocator(ConvertCommandListType(type), IID_PPV_ARGS(&d3d12_command_allocator_)));
		ThrowIfFailed(device_->GetNative()->CreateCommandList(
			0, 
			ConvertCommandListType(type), 
			d3d12_command_allocator_.Get(), 
			nullptr, 
			IID_PPV_ARGS(&d3d12_command_list_)));

	}

	D12CommandList::~D12CommandList()
	{

	}

	void D12CommandList::TransitionBarrier(Buffer* buffer, ResourceStates state_afeter, uint32_t subresource,
	                                       bool flush_barriers, bool permanent)
	{
		if(buffer->IsPermanentState())
		{
			return;
		}

		auto d12_buffer = CheckedCast<D12Buffer*>(buffer);
		resource_state_tracker_.ResourceBarrier(CD3DX12_RESOURCE_BARRIER::Transition(
			d12_buffer->GetNative(),
			D3D12_RESOURCE_STATE_COMMON,
			ConvertResourceStates(state_afeter), subresource));
	}

	void D12CommandList::TransitionBarrier(Texture* texture, ResourceStates state_afeter, uint32_t subresource,
	                                       bool flush_barriers, bool permanent)
	{
		if (texture->IsPermanentState())
		{
			return;
		}

		auto d12_texture = CheckedCast<D12Texture*>(texture);
		resource_state_tracker_.ResourceBarrier(CD3DX12_RESOURCE_BARRIER::Transition(
			d12_texture->GetNative(),
			D3D12_RESOURCE_STATE_COMMON,
			ConvertResourceStates(state_afeter), subresource));
	}

	void D12CommandList::ClearTexture(Texture* texture, const float* clear_value)
	{
		auto d12_texture = CheckedCast<D12Texture*>(texture);

		TransitionBarrier(d12_texture, ResourceStates::kRenderTarget);

		d3d12_command_list_->ClearRenderTargetView(d12_texture->GetRTV(), clear_value, 0, nullptr);

		TrackResource(d12_texture);
	}

	void D12CommandList::ClearDepthStencilTexture(Texture* texture, ClearFlags clear_flags, float depth,
		uint8_t stencil)
	{
		auto d12_texture = CheckedCast<D12Texture*>(texture);

		TransitionBarrier(d12_texture, ResourceStates::kDepthWrite);

		d3d12_command_list_->ClearDepthStencilView(d12_texture->GetDSV(), ConvertClearFlags(clear_flags), depth, stencil, 0, nullptr);

		TrackResource(d12_texture);
	}

	void D12CommandList::WriteBuffer(Buffer* buffer, const uint8_t* data, uint64_t size, uint64_t dest_offset_bytes)
	{
		if (size == 0)
		{
			return;
		}

		auto d12_buffer = CheckedCast<D12Buffer*>(buffer);

		UploadBuffer::Allocation allocation = upload_buffer_.Allocate(size, 1);

		memcpy(allocation.cpu, data, size);

		//todo:
		TransitionBarrier(buffer, ResourceStates::kCopyDest);

		d3d12_command_list_->CopyBufferRegion(
			d12_buffer->GetNative(), 
			dest_offset_bytes,
			allocation.upload_resource, 
			allocation.offset, size);
	}

	void D12CommandList::SetGraphicsDynamicConstantBuffer(uint32_t parameter_index, size_t bytes, const void* data)
	{
		UploadBuffer::Allocation allocation = upload_buffer_.Allocate(bytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

		memcpy(allocation.cpu, data, bytes);

		d3d12_command_list_->SetGraphicsRootConstantBufferView(parameter_index, allocation.gpu);
	}

	void D12CommandList::SetGraphics32BitConstants(uint32_t parameter_index, uint32_t num_constants,
		const void* constants)
	{
		d3d12_command_list_->SetGraphicsRoot32BitConstants(parameter_index, num_constants, constants, 0);
	}

	void D12CommandList::SetGraphicsPipeline(GraphicsPipeline* pso)
	{
		if(current_pso_ != pso)
		{
			auto d12_pso = CheckedCast<D12GraphicsPipeline*>(pso);

			d3d12_command_list_->SetGraphicsRootSignature(d12_pso->GetRootSignature());

			d3d12_command_list_->SetPipelineState(d12_pso->GetNative());

			TrackResource(pso);
		}
	}

	void D12CommandList::SetVertexBuffer(uint32_t slot, Buffer* buffer)
	{
		const BufferDesc& desc = buffer->GetDesc();

		CHECK(desc.type == BufferType::kVertex, "buffer的Type不是VertexBuffer");

		auto d12_buffer = CheckedCast<D12Buffer*>(buffer);

		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = d12_buffer->GetNative()->GetGPUVirtualAddress();
		view.SizeInBytes = desc.byte;
		view.StrideInBytes = desc.stride;

		d3d12_command_list_->IASetVertexBuffers(slot, 1, &view);
	}

	void D12CommandList::SetIndexBuffer(Buffer* buffer)
	{
		const BufferDesc& desc = buffer->GetDesc();

		CHECK(desc.type == BufferType::kIndex,"buffer的Type不是IndexBuffer");

		auto d12_buffer = CheckedCast<D12Buffer*>(buffer);

		D3D12_INDEX_BUFFER_VIEW view{};
		view.BufferLocation = d12_buffer->GetNative()->GetGPUVirtualAddress();
		view.SizeInBytes = static_cast<UINT>(desc.byte);
		//view.Format = 
		d3d12_command_list_->IASetIndexBuffer(&view);
	}

	void D12CommandList::SetRednerTarget(const RenderTarget& render_target)
	{
		std::array<D3D12_CPU_DESCRIPTOR_HANDLE,static_cast<uint32_t>(AttachmentPoint::kNumAttachmentPoints)> render_target_descriptors{};
		uint32_t num_render_target = 0;
		const auto& textures = render_target.GetTextures();

		//查找所有color target
		for (uint32_t i = 0; i < static_cast<uint32_t>(AttachmentPoint::kDepthStencil); ++i)
		{
			const auto& texture = textures[i];
			if (texture)
			{
				auto d12_texture = CheckedCast<D12Texture*>(texture);

				TransitionBarrier(d12_texture, ResourceStates::kRenderTarget);

				render_target_descriptors[num_render_target++] = d12_texture->GetRTV();

				TrackResource(d12_texture);
			}
		}

		D3D12_CPU_DESCRIPTOR_HANDLE depth_stencil_descriptor{0};

		const auto& depth_texture = render_target.GetTexture(AttachmentPoint::kDepthStencil);
		if(depth_texture)
		{
			auto d12_depth_texture = CheckedCast<D12Texture*>(depth_texture);

			TransitionBarrier(d12_depth_texture, ResourceStates::kDepthWrite);

			depth_stencil_descriptor = d12_depth_texture->GetDSV();

			TrackResource(d12_depth_texture);
		}

		D3D12_CPU_DESCRIPTOR_HANDLE* dsv = depth_stencil_descriptor.ptr != 0 ? &depth_stencil_descriptor : nullptr;
		d3d12_command_list_->OMSetRenderTargets(num_render_target, render_target_descriptors.data(), false, dsv);
	}

	void D12CommandList::SetViewport(const Viewport& viewport)
	{
		SetViewport({ viewport });
	}

	void D12CommandList::SetViewports(const std::vector<Viewport>& viewports)
	{
		std::vector<D3D12_VIEWPORT> d12_viewports;
		d12_viewports.reserve(viewports.size());

		for (auto& viewport : viewports)
		{
			d12_viewports.emplace_back(ConvertViewport(viewport));
		}

		d3d12_command_list_->RSSetViewports(d12_viewports.size(), d12_viewports.data());
	}

	void D12CommandList::SetScissorRect(const Rect& rect)
	{
		SetScissorRects({ rect });
	}

	void D12CommandList::SetScissorRects(const std::vector<Rect>& rects)
	{
		std::vector<D3D12_RECT> d12_rects;
		d12_rects.reserve(rects.size());

		for (auto& rect : rects)
		{
			d12_rects.emplace_back(ConvertRect(rect));
		}

		d3d12_command_list_->RSSetScissorRects(d12_rects.size(), d12_rects.data());
	}

	void D12CommandList::ExecuteCommandList()
	{
		queue_->ExecuteCommandList(this);
	}

	bool D12CommandList::Close(CommandList* pending_command_list)
	{
		
	}

	void D12CommandList::Close()
	{
		d3d12_command_list_->Close();
	}

	void D12CommandList::Reset()
	{
		ThrowIfFailed(d3d12_command_allocator_->Reset());
		ThrowIfFailed(d3d12_command_list_->Reset(d3d12_command_allocator_.Get(),nullptr));

		track_resources_.clear();

		current_pso_ = nullptr;
	}

	void D12CommandList::DrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t start_index,
	                                 int32_t base_vertex, uint32_t start_instance)
	{
		d3d12_command_list_->DrawIndexedInstanced(index_count, instance_count, start_index, base_vertex, start_instance);
	}

	void D12CommandList::TrackResource(Resource* resource)
	{
		track_resources_.emplace_back(resource);
	}
}
