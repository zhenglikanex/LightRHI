#pragma once

#include <vector>

#include "resource.h"

namespace light::rhi
{
	struct ShaderDesc
	{
		ShaderType type = ShaderType::kNone;
	};

	class Shader : public Resource
	{
	public:
		Shader(const ShaderDesc& desc, std::vector<char> bytecode)
			: desc_(desc)
			, bytecode_(std::move(bytecode))
		{
		}

		const ShaderDesc& GetDesc() const { return desc_; }
		const std::vector<char>& GetBytecode() const { return bytecode_; }
	protected:
		ShaderDesc desc_;
		std::vector<char> bytecode_;
	};

	using ShaderHandle = Handle<Shader>;
}