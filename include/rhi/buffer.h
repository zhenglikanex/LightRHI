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
		Format format = Format::UNKNOWN;
		bool is_uav = false;
		uint32_t stride = 0;
		uint32_t size_in_bytes = 0;

		std::string debug_name;
	};

	class Buffer : public Resource
	{
	public:
		explicit Buffer(const BufferDesc& desc)
			: desc_(desc)
			, permanent_state_(false)
		{
			
		}

		const BufferDesc& GetDesc() const noexcept { return desc_; }

		void SetPermanentState(bool value) { permanent_state_ = value; }

		bool IsPermanentState() const { return permanent_state_; }
	protected:
		BufferDesc desc_;
		bool permanent_state_;
	};

	using BufferHandle = Handle<Buffer>;
}