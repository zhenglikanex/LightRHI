#pragma once

#include <string>

#include "resource.h"
#include "types.h"

namespace light::rhi
{
	struct BufferDesc
	{
		BufferType type = BufferType::kUnkown;
		CpuAccess cpu_access = CpuAccess::kNone;
		
		bool is_uav;
		uint64_t byte = 0;

		std::string debug_name;
	};

	class Buffer : public Resource
	{
	public:
		explicit Buffer(const BufferDesc desc)
			: desc_(desc)
		{
			
		}

		const BufferDesc& desc() const noexcept { return desc_; }
	protected:
		BufferDesc desc_;
	};

	using BufferHandle = Handle<Buffer>;
}