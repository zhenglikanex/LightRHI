#include "d12_command_queue.h"
#include "d12_device.h"

#include <chrono>
#include <iostream>
#include "d3dcommon.h"
#include "auto_timer.h"
#if !defined(NO_D3D11_DEBUG_NAME) && ( defined(_DEBUG) || defined(PROFILE) )
#pragma comment(lib,"dxguid.lib")
#endif

namespace light::rhi
{
	// Set the name of a running thread (for debugging)
#pragma pack( push, 8 )
	typedef struct tagTHREADNAME_INFO
	{
		DWORD  dwType;      // Must be 0x1000.
		LPCSTR szName;      // Pointer to name (in user addr space).
		DWORD  dwThreadID;  // Thread ID (-1=caller thread).
		DWORD  dwFlags;     // Reserved for future use, must be zero.
	} THREADNAME_INFO;
#pragma pack( pop )

	// Set the name of an std::thread.
// Useful for debugging.
	const DWORD MS_VC_EXCEPTION = 0x406D1388;

	inline void SetThreadName(std::thread& thread, const char* threadName)
	{
		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = threadName;
		info.dwThreadID = ::GetThreadId(reinterpret_cast<HANDLE>(thread.native_handle()));
		info.dwFlags = 0;

		__try
		{
			::RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}

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

		const char* thread_name = nullptr;
		switch (type) {
		case CommandListType::kDirect:
			queue_->SetName(L"Direct Command Queue");
			thread_name = "Direct Command Queue";
			break;
		case CommandListType::kCompute:
			queue_->SetName(L"Compute Command Queue");
			thread_name = "Compute Command Queue";
			break;
		case CommandListType::kCopy:
			queue_->SetName(L"Copy Command Queue");
			thread_name = "Copy Command Queue";
			break;
		}

		command_thread_ = std::thread(&D12CommandQueue::ProcessCommandLists, this);

		SetThreadPriority(command_thread_.native_handle(), THREAD_PRIORITY_TIME_CRITICAL);
		SetThreadName(command_thread_, thread_name);
	}

	D12CommandQueue::~D12CommandQueue()
	{
		run_ = false;
		command_thread_.join();
	}

	CommandListHandle D12CommandQueue::GetCommandList()
	{
		Handle<CommandList> command_list = nullptr;
		//std::cout << available_command_lists_.Size() << std::endl;
		if(!available_command_lists_.TryPop(command_list))
		{
			//std::cout << "new commandlist\n";

			auto str ="new commandlist" + std::to_string(available_command_lists_.Size()) + "  +  " + std::to_string(flight_command_lists_.Size());
			AutoTimer timer(str);
			command_list = MakeHandle<D12CommandList>(device_, command_list_type_,this);
		}

		//std::cout << std::ios::hex << command_list.Get() << std::endl;

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
		while(!flight_command_lists_.Empty())
		{
			
		}

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
		std::vector<CommandListHandle> flight_command_lists;
		flight_command_lists.reserve(num * 2);

		std::vector<ID3D12CommandList*> d3d12_command_lists;
		d3d12_command_lists.reserve(num * 2);

		for (uint64_t i = 0; i < num; ++i)
		{
			auto pending_command_list = GetCommandList();
			if (command_lists->Close(pending_command_list))
			{
				auto d12_pending_command_list = CheckedCast<D12CommandList*>(pending_command_list.Get());

				d3d12_command_lists.push_back(d12_pending_command_list->GetD3D12GraphicsCommandList());
			}

			pending_command_list->Close();

			auto d12_command_list = CheckedCast<D12CommandList*>(&command_lists[i]);
			d3d12_command_lists.push_back(d12_command_list->GetD3D12GraphicsCommandList());

			flight_command_lists.push_back(&command_lists[i]);
			flight_command_lists.push_back(pending_command_list);
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
		while(run_)
		{
			CommandListEntry entry;
			while(flight_command_lists_.TryPop(entry))
			{
				WaitForFenceValue(entry.fence_value);
				entry.command_list->Reset();
				available_command_lists_.Push(entry.command_list);
			}
		}
	}
}
