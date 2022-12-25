#include "d12_buffer.h"

#include "d12_device.h"

namespace light::rhi
{
	D12Buffer::D12Buffer(D12Device* device, const BufferDesc& desc)
		: Buffer(desc)
	{
		D3D12_RESOURCE_DESC d12_desc = CD3DX12_RESOURCE_DESC::Buffer(desc.byte, desc.is_uav ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE, 0);

		D3D12_HEAP_TYPE heap_type = {};

		switch (desc.cpu_access) {
		case CpuAccess::kNone:
			heap_type = D3D12_HEAP_TYPE_DEFAULT;
			break;
		case CpuAccess::kRead:
			heap_type = D3D12_HEAP_TYPE_READBACK;
			break;
		case CpuAccess::kWrite:
			heap_type = D3D12_HEAP_TYPE_UPLOAD;
			break;
		}

		D3D12_HEAP_PROPERTIES heap_properties = CD3DX12_HEAP_PROPERTIES(heap_type);

		ThrowIfFailed(device->GetNative()->CreateCommittedResource(
			&heap_properties,
			D3D12_HEAP_FLAG_NONE,
			&d12_desc,
			ConvertResourceState(desc.initial_state),
			nullptr,
			IID_PPV_ARGS(&resource_)));

#ifdef _DEBUG
		resource_->SetName(desc.debug_name.c_str());
#endif
	}
}
