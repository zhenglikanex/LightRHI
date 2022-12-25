#include "d12_command_list.h"

#include "d12_device.h"

namespace light::rhi
{
	D12CommandList::D12CommandList(D12Device* device, CommandListType type)
		: CommandList(type)
		, device_(device)
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

	void D12CommandList::WriteBuffer(Buffer* buffer, const uint8_t* data, uint64_t size)
	{

	}

	void D12CommandList::SetVertexBuffer(Buffer* buffer)
	{
		//d3d12_command_list_->IASetVertexBuffers()
	}

	void D12CommandList::SetIndexBuffer(Buffer* buffer)
	{
		auto d12_buffer = CheckedCast<D12Buffer*>(buffer);

		D3D12_INDEX_BUFFER_VIEW view{};
		view.BufferLocation = d12_buffer->GetNative()->GetGPUVirtualAddress();
		view.SizeInBytes = static_cast<UINT>(d12_buffer->desc().byte);
		//view.Format = 
		d3d12_command_list_->IASetIndexBuffer(&view);
	}

	bool D12CommandList::Close(CommandList* pending_command_list)
	{

	}

	void D12CommandList::Close()
	{
		d3d12_command_list_->Close();
	}
}
