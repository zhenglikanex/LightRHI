#pragma once

#include <vector>

#include "resource.h"

namespace light::rhi
{
	class Shader : public Resource
	{
	public:

		const std::vector<char>& GetBytecode() const { return bytecode_; }
	private:
		std::vector<char> bytecode_;
	};

	using ShaderHandle = Handle<Shader>;
}