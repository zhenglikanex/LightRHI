#include "d12_command_queue.h"
#include "d12_device.h"

namespace light::rhi
{
	D12CommandQueue::D12CommandQueue(D12Device* device, CommandListType type)
		: CommandQueue(type)
		, device_(device)
		, fence_value_(0)
		, run_(true)
	{
		D3D12_COMMAND_QUEUE_DESC desc{};
		desc.Type = ConvertCommandListType(type);
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.NodeMask = 0;

		ThrowIfFailed(device_->GetNative()->CreateCommandQueue(&desc, IID_PPV_ARGS(&queue_)));
		ThrowIfFailed(device_->GetNative()->CreateFence(fence_value_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)));

		switch (type) {
		case CommandListType::kDirect:
			queue_->SetName(L"Direct Command Queue");
			break;
		case CommandListType::kCompute:
			queue_->SetName(L"Compute Command Queue");
			break;
		case CommandListType::kCopy:
			queue_->SetName(L"Copy Command Queue");
			break;
		}

		command_thread_ = std::thread(&D12CommandQueue::ProcessCommandLists, this);
	}

	CommandList* D12CommandQueue::GetCommandList()
	{
		Handle<CommandList> command_list = nullptr;
		if(!available_command_lists_.TryPop(command_list))
		{
			command_list = MakeHandle<D12CommandList>(device_, command_list_type_,this);
		}

		return command_list;
	}

	uint64_t D12CommandQueue::Signal()
	{
		uint64_t fence_value = ++fence_value_;
		queue_->Signal(fence_.Get(), fence_value);
		return fence_value;
	}

	bool D12CommandQueue::IsFenceCompleted(uint64_t fence_value)
	{
		return fence_->GetCompletedValue() >= fence_value;
	}

	void D12CommandQueue::WaitForFenceValue(uint64_t fence_value)
	{
		if(!IsFenceCompleted(fence_value))
		{
			//阻塞到queue设置fence完成
			HANDLE event = ::CreateEvent(NULL, FALSE, FALSE, NULL);

			fence_->SetEventOnCompletion(fence_value, event);

			::WaitForSingleObject(event,-1);

			::CloseHandle(event);
		}
	}

	void D12CommandQueue::Flush()
	{
		std::unique_lock<std::mutex> lock;
		// 等待提交线程
		condition_.wait(lock, [this] {return flight_command_lists_.Empty(); });

		WaitForFenceValue(fence_value_);
	}

	uint64_t D12CommandQueue::ExecuteCommandList(CommandList* command_list)
	{
		return ExecuteCommandLists(1, command_list);
	}

	uint64_t D12CommandQueue::ExecuteCommandLists(uint64_t num, CommandList* command_lists)
	{
		std::unique_lock<std::mutex> lock(ResourceStateTracker::s_global_mutex);

		// 等待上传到fight_command_lists列表
		std::vector<CommandList*> flight_command_lists;
		flight_command_lists.reserve(num * 2);

		std::vector<ID3D12CommandList*> d3d12_command_lists;
		d3d12_command_lists.reserve(num * 2);

		for (uint64_t i = 0; i < num; ++i)
		{
			auto pending_command_list = GetCommandList();
			if (command_lists->Close(pending_command_list))
			{
				auto d12_pending_command_list = CheckedCast<D12CommandList*>(pending_command_list);
				d12_pending_command_list->Close();

				d3d12_command_lists.push_back(d12_pending_command_list->GetD3D12GraphicsCommandList());
			}

			auto d12_command_list = CheckedCast<D12CommandList*>(&command_lists[i]);
			d3d12_command_lists.push_back(d12_command_list->GetD3D12GraphicsCommandList());

			flight_command_lists.push_back(&command_lists[i]);
		}

		queue_->ExecuteCommandLists(static_cast<UINT>(d3d12_command_lists.size()), d3d12_command_lists.data());

		uint64_t fence_value = Signal();

		lock.unlock();

		// 记录执行中的command_list
		for(auto& command_list : flight_command_lists)
		{
			flight_command_lists_.Push(CommandListEntry{ fence_value,command_list });
		}

		return fence_value;
	}

	void D12CommandQueue::ProcessCommandLists()
	{
		std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);

		while(run_)
		{
			CommandListEntry entry;
			lock.lock();

			while(flight_command_lists_.TryPop(entry))
			{
				WaitForFenceValue(entry.fence_value);

				entry.command_list->Reset();

				available_command_lists_.Push(entry.command_list);
			}

			lock.unlock();

			condition_.notify_one();

			std::this_thread::yield();
		}
	}
}
