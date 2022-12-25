#pragma once

#include "rhi/device.h"

#include "d12_convert.h"
#include "d12_command_list.h"
#include "d12_command_queue.h"
#include "d12_buffer.h"

namespace light::rhi
{
	constexpr uint64_t kConstantAlignSize = 256ull;

	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw std::exception();
		}
	}

	class D12Device final : public Device
	{
	public:
		D12Device() = default;

		~D12Device() override = default;

		ID3D12Device* GetNative() noexcept { return device_; }

		BufferHandle CreateBuffer(BufferDesc desc) override;
		CommandListHandle CreateCommandList(CommandListType type) override;

	private:
		Handle<ID3D12Device> device_;
		Handle<D12CommandQueue> queues_[static_cast<size_t>(CommandListType::kCopy) + 1];
	};
}