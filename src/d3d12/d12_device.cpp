#include "d12_device.h"
#include "d12_buffer.h"

namespace light::rhi
{
	BufferHandle D12Device::CreateBuffer(const BufferDesc& desc)
	{
		Handle<D12Buffer> buffer(new D12Buffer(desc));

		D3D12_RESOURCE_DESC d12_desc = CD3DX12_RESOURCE_DESC::Buffer(desc.byte, desc.is_uav ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE, desc.algment);
		D3D12_HEAP_PROPERTIES heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		switch (desc.cpu_access) {
		case CpuAccess::kNone:
			break;
		case CpuAccess::kRead:
			heap_properties.Type = D3D12_HEAP_TYPE_READBACK;
			break;
		case CpuAccess::kWrite:
			heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
			break;
		}

		ThrowIfFailed(device_->CreateCommittedResource(
			&heap_properties,
			D3D12_HEAP_FLAG_NONE,
			&d12_desc, 
			D3D12_RESOURCE_STATE_COMMON, 
			nullptr, 
			IID_PPV_ARGS(&buffer->resource)));

		return buffer;
	}
}