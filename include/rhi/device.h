#pragma once

#include "base.h"
#include "resource.h"
#include "buffer.h"
#include "command_list.h"
#include "types.h"

namespace light::rhi
{
	class Device : Resource
	{
	public:
		virtual ~Device() = default;

		virtual BufferHandle CreateBuffer(BufferDesc desc) = 0;

		virtual CommandListHandle CreateCommandList(CommandListType type) = 0;
	private:
	};

	using DeviceHandle = Handle<Device>;
}