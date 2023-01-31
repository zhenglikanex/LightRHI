#include "d12_buffer.h"

#include "d12_device.h"

namespace light::rhi
{
	D12Buffer::D12Buffer(D12Device* device, const BufferDesc& desc)
		: Buffer(desc)
		, device_(device)
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
		// todo
		//resource_->SetName( desc.debug_name.c_str());
#endif
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D12Buffer::GetCBV()
	{
		if(cbv_.IsNull())
		{
			cbv_ = device_->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
		}

		if(cbv_.IsValid())
		{
			D3D12_CONSTANT_BUFFER_VIEW_DESC desc{};
			desc.BufferLocation = resource_->GetGPUVirtualAddress();
			desc.SizeInBytes = desc_.byte;

			device_->GetNative()->CreateConstantBufferView(&desc, cbv_.GetDescriptorHandle());
		}

		return cbv_.GetDescriptorHandle();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D12Buffer::GetSBV(uint32_t offset)
	{
		return GetSBV(offset, desc_.byte);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D12Buffer::GetSBV(uint32_t offset, uint32_t byte_size)
	{
		size_t hash = 0;

		HashCombine(hash, offset);
		HashCombine(hash, byte_size);

		auto it = sbv_map_.find(hash);
		if(it != sbv_map_.end())
		{
			return it->second.GetDescriptorHandle();
		}

		DescriptorAllocation descriptor_allocation = device_->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);

		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
		srv_desc.Format = DXGI_FORMAT_UNKNOWN;
		srv_desc.Buffer.FirstElement = offset / desc_.stride;
		srv_desc.Buffer.NumElements = byte_size / desc_.stride;
		srv_desc.Buffer.StructureByteStride = desc_.stride;

		device_->GetNative()->CreateShaderResourceView(resource_, &srv_desc, descriptor_allocation.GetDescriptorHandle());

		sbv_map_.emplace(hash, std::move(descriptor_allocation));

		return sbv_map_[hash].GetDescriptorHandle();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D12Buffer::GetUBV()
	{
		
	}

	void D12Buffer::CreateCBV()
	{
	}

	void D12Buffer::CreateSBV()
	{
	}

	void D12Buffer::CreateUBV()
	{
	}
}
