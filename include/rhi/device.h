#pragma once

#include "base.h"
#include "resource.h"
#include "buffer.h"

namespace light::rhi
{
	class Device : Resource
	{
	public:
		virtual ~Device() = default;

		virtual BufferHandle CreateBuffer(const BufferDesc& desc) = 0;
	private:
	};

	using DeviceHandle = Handle<Device>;
}