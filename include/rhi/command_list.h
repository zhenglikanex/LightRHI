#pragma once

#include "types.h"
#include "resource.h"

namespace light::rhi
{
	class Buffer;
	class CommandQueue;

	class CommandList : public Resource
	{
	public:
		explicit CommandList(CommandListType type)
			: type_(type)
		{
		}

		virtual void WriteBuffer(Buffer* buffer, const uint8_t* data, uint64_t size) = 0;

		virtual void SetVertexBuffer(Buffer* buffer) = 0;

		virtual void SetIndexBuffer(Buffer* buffer) = 0;

		virtual bool Close(CommandList* pending_command_list) = 0;

		virtual void Close() = 0;
	protected:
		CommandListType type_;
	};

	using CommandListHandle = Handle<CommandList>;
}
