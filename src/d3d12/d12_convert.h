#pragma once

#include "rhi/types.h"
#include "d3dx12.h"

namespace light::rhi
{
	inline D3D12_RESOURCE_STATES ConvertResourceState(ResourceState state)
	{
		return static_cast<D3D12_RESOURCE_STATES>(state);
	}

	inline D3D12_COMMAND_LIST_TYPE ConvertCommandListType(CommandListType type)
	{
		switch (type) {
		case CommandListType::kDirect: return D3D12_COMMAND_LIST_TYPE_DIRECT;
		case CommandListType::kCompute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		case CommandListType::kCopy: return D3D12_COMMAND_LIST_TYPE_COPY;
		}
		return {};
	}
}
