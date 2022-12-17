#pragma once

namespace light::rhi
{
	enum class ResourceState
	{
		kCommon,
	};

	enum class CpuAccess
	{
		kNone,		// 不在CPU端访问
		kRead,		// 在CPU端读取
		kWrite		// 在CPU端写入
	};

	enum class BufferType : uint8_t
	{
		kUnkown,
		kVertex,
		kIndex,
		kConstant,
	};
}