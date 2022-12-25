#pragma once

#include <string>

#include "resource.h"
#include "types.h"

namespace light::rhi
{
	struct BufferDesc
	{
		BufferType type = BufferType::kUnknown;
		CpuAccess cpu_access = CpuAccess::kNone;
		ResourceState initial_state;
		
		bool is_uav = false;
		uint64_t byte = 0;

#ifdef _DEBUG
		std::wstring debug_name;
#endif
	};

	class Buffer : public Resource
	{
	public:
		explicit Buffer(const BufferDesc& desc)
			: desc_(desc)
		{
			
		}

		const BufferDesc& desc() const noexcept { return desc_; }
	protected:
		BufferDesc desc_;
	};

	using BufferHandle = Handle<Buffer>;
}