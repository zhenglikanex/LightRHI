#pragma once

#include "types.h"
#include "resource.h"
#include "command_list.h"

namespace light::rhi
{
	class CommandQueue : public Resource
	{
	public:
		explicit CommandQueue(CommandListType command_list_type)
			: command_list_type_(command_list_type)
		{
		}

		virtual CommandList* GetCommandList() = 0;

		virtual uint64_t ExecuteCommandList(CommandList* command_list) = 0;
		virtual uint64_t ExecuteCommandLists(uint64_t num, CommandList* command_lists) = 0;

		// 递增fence并发送信号
		virtual uint64_t Signal() = 0;

		virtual bool IsFenceCompleted(uint64_t fence_value) = 0;
		virtual void WaitForFenceValue(uint64_t fence_value) = 0;

		// 等待当前所有的FightCommandList结束
		virtual void Flush() = 0;

		virtual void ProcessCommandLists() = 0;
	protected:
		CommandListType command_list_type_;
	};

	using CommandQueueHandle = Handle<CommandQueue>;
}