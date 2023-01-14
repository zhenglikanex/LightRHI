#pragma once

#include "rhi/input_layout.h"

namespace light::rhi
{
	class D12Device;

	class D12InputLayout final : public InputLayout
	{
	public:
		D12InputLayout(D12Device* device, std::vector<VertexAttributeDesc> attributes);

		~D12InputLayout() override = default;

		D3D12_INPUT_ELEMENT_DESC* GetInputElements();

		uint32_t NumElements() const;
	private:
		std::vector<D3D12_INPUT_ELEMENT_DESC> input_elements_;
	};
}