#include "d3d12/d12_device.h"

using namespace light::rhi;

int main()
{
	DeviceHandle device = MakeHandle<D12Device>();

	BufferDesc desc;
	desc.byte = 10;
	desc.initial_state = ResourceState::kCopyDest;
	device->CreateBuffer(desc);
}