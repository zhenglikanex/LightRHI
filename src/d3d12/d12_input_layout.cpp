#include "d12_input_layout.h"
#include "d12_device.h"
#include "d12_convert.h"

namespace light::rhi
{
	D12InputLayout::D12InputLayout(D12Device* device, std::vector<VertexAttributeDesc> attributes)
		: InputLayout(std::move(attributes))
	{
		(void*)device;

		input_elements_.reserve(attributes_.size());

		for(auto& attribute : attributes_)
		{
			D3D12_INPUT_ELEMENT_DESC input_element {};
			input_element.SemanticName = attribute.semantic_name.c_str();
			input_element.SemanticIndex = attribute.semantic_index;
			input_element.Format = GetDxgiFormatMapping(attribute.format).srv_format;
			input_element.AlignedByteOffset = attribute.offset;
			input_element.InputSlot = attribute.slot;
			if(attribute.is_instance)
			{
				input_element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
				input_element.InstanceDataStepRate = 1;
			}
			else
			{
				input_element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				input_element.InstanceDataStepRate = 0;
			}

			input_elements_.emplace_back(input_element);
		}
	}

	D3D12_INPUT_ELEMENT_DESC* D12InputLayout::GetInputElements()
	{
		return input_elements_.data();
	}

	uint32_t D12InputLayout::NumElements() const
	{
		return input_elements_.size();
	}
}
