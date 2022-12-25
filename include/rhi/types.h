#pragma once

#include <cstdint>

namespace light::rhi
{
	enum class ResourceState
	{
		kCommon = 0,
		kVertexAndConstantBuffer = 0x1,
		kIndexBuffer = 0x2,
		kRenderTarget = 0x4,
		kUnorderedAccess = 0x8,
		kDepthWrite = 0x10,
		kDepthRead = 0x20,
		kNonPixelShaderResource = 0x40,
		kPixelShaderResource = 0x80,
		kStreamOut = 0x100,
		kIndirectArgument = 0x200,
		kCopyDest = 0x400,
		kCopySource = 0x800,
		kResolveDest = 0x1000,
		kResolveSource = 0x2000,
		kRaytracingAccelerationStructure = 0x400000,
		kShadingRateSource = 0x1000000,
		kGenericRead = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
		kPresent = 0,
		kPredication = 0x200,
		kVideoDecodeRead = 0x10000,
		kVideoDecodeWrite = 0x20000,
		kVideoProcessRead = 0x40000,
		kVideoProcessWrite = 0x80000,
		kVideoEncodeRead = 0x200000,
		kVideoEncodeWrite = 0x800000
	};

	enum class CpuAccess : uint8_t
	{
		kNone,		// 不在CPU端访问
		kRead,		// 在CPU端读取
		kWrite		// 在CPU端写入
	};

	enum class BufferType : uint8_t
	{
		kUnknown,
		kVertex,
		kIndex,
		kConstant,
	};

	enum class CommandListType : uint8_t
	{
		kDirect = 0,
		kCompute = 1,
		kCopy = 2,
	};
}
