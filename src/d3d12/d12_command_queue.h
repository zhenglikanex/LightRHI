#pragma once

#include <vector>

#include "d3dx12.h"

#include "rhi/thread_safe_queue.hpp"
#include "rhi/command_queue.h"
#include "rhi/command_list.h"

#include "d12_command_list.h"

namespace light::rhi
{
	class D12Device;
		

	class D12CommandQueue final : public CommandQueue
	{
	public:
		D12CommandQueue(D12Device* device, CommandListType type);

		CommandListHandle GetCommandList() override;

		uint64_t Signal() override;

		bool IsFenceCompleted(uint64_t fence_value) override;

		void WaitForFenceValue(uint64_t fence_value) override;

		void Flush() override;

		uint64_t ExecuteCommandList(CommandList* command_list) override;

		uint64_t ExecuteCommandLists(uint64_t num, CommandList* command_lists) override;

	private:
		struct CommandListEntry
		{
			uint64_t fence_value;
			CommandListHandle command_list;
		};

		D12Device* device_;
		Handle<ID3D12CommandQueue> queue_;
		ThreadSafeQueue<Handle<D12CommandList>> available_command_lists_;
		ThreadSafeQueue<CommandListEntry> flight_command_lists_;
		Handle<ID3D12Fence> fence_;
		uint64_t fence_value_;
		std::mutex mutex_;
		std::condition_variable condition_;
	};

}
