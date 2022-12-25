#pragma once

#include "rhi/command_list.h"

#include "d3dx12.h"

namespace light::rhi
{
	class D12Device;
	class D12CommandQueue;

	class D12CommandList final : public CommandList
	{
	public:
		D12CommandList(D12Device* device,CommandListType type);
		~D12CommandList() override;

		void WriteBuffer(Buffer* buffer, const uint8_t* data, uint64_t size) override;
		void SetVertexBuffer(Buffer* buffer) override;
		void SetIndexBuffer(Buffer* buffer) override;

		ID3D12GraphicsCommandList* GetD3D12GraphicsCommandList() { return d3d12_command_list_; }
		
		bool Close(CommandList* pending_command_list) override;
		void Close() override;

	private:
		D12Device* device_;
		Handle<ID3D12CommandAllocator> d3d12_command_allocator_;
		Handle<ID3D12GraphicsCommandList> d3d12_command_list_;
	};

}
