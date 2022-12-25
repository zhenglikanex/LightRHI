#include "d12_device.h"
#include "d12_buffer.h"

namespace light::rhi
{
	BufferHandle D12Device::CreateBuffer(BufferDesc desc)
	{
		if(desc.type == BufferType::kConstant)
		{
			desc.byte = Align(desc.byte, kConstantAlignSize);
		}

		return MakeHandle<D12Buffer>(this,desc);
	}

	CommandListHandle D12Device::CreateCommandList(CommandListType type)
	{
	 	return  queues_[static_cast<size_t>(type)]->GetCommandList();
	}
}
