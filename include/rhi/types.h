#pragma once

namespace light::rhi
{
	enum class ResourceState
	{
		kCommon,
	};

	enum class CpuAccess
	{
		kNone,		// ����CPU�˷���
		kRead,		// ��CPU�˶�ȡ
		kWrite		// ��CPU��д��
	};

	enum class BufferType : uint8_t
	{
		kUnkown,
		kVertex,
		kIndex,
		kConstant,
	};
}