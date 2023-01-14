#pragma once

#include <string>
#include <vector>

#include "resource.h"
#include "types.h"

namespace light::rhi
{
	struct VertexAttributeDesc
	{
		std::string semantic_name;
		uint32_t sematic_index;
		Format format;
		uint32_t slot;
		uint32_t offset;
		bool is_instance;
	};

	class InputLayout : public Resource
	{
	public:
		explicit InputLayout(std::vector<VertexAttributeDesc> attributes)
			: attributes_(std::move(attributes))
		{ }

		const std::vector<VertexAttributeDesc>& GetAttributes() const { return attributes_; }
	protected:
		std::vector<VertexAttributeDesc> attributes_;
	};

	using InputLayoutHandle = Handle<InputLayout>;

}