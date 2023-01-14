#pragma once

#include <cstdint>
#include "base.h"

namespace light::rhi
{
	enum class ResourceStates
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

	enum class Format : uint8_t
	{
		UNKNOWN,

		R8_UINT,
		R8_SINT,
		R8_UNORM,
		R8_SNORM,
		RG8_UINT,
		RG8_SINT,
		RG8_UNORM,
		RG8_SNORM,
		R16_UINT,
		R16_SINT,
		R16_UNORM,
		R16_SNORM,
		R16_FLOAT,
		BGRA4_UNORM,
		B5G6R5_UNORM,
		B5G5R5A1_UNORM,
		RGBA8_UINT,
		RGBA8_SINT,
		RGBA8_UNORM,
		RGBA8_SNORM,
		BGRA8_UNORM,
		SRGBA8_UNORM,
		SBGRA8_UNORM,
		R10G10B10A2_UNORM,
		R11G11B10_FLOAT,
		RG16_UINT,
		RG16_SINT,
		RG16_UNORM,
		RG16_SNORM,
		RG16_FLOAT,
		R32_UINT,
		R32_SINT,
		R32_FLOAT,
		RGBA16_UINT,
		RGBA16_SINT,
		RGBA16_FLOAT,
		RGBA16_UNORM,
		RGBA16_SNORM,
		RG32_UINT,
		RG32_SINT,
		RG32_FLOAT,
		RGB32_UINT,
		RGB32_SINT,
		RGB32_FLOAT,
		RGBA32_UINT,
		RGBA32_SINT,
		RGBA32_FLOAT,

		D16,
		D24S8,
		X24G8_UINT,
		D32,
		D32S8,
		X32G8_UINT,

		BC1_UNORM,
		BC1_UNORM_SRGB,
		BC2_UNORM,
		BC2_UNORM_SRGB,
		BC3_UNORM,
		BC3_UNORM_SRGB,
		BC4_UNORM,
		BC4_SNORM,
		BC5_UNORM,
		BC5_SNORM,
		BC6H_UFLOAT,
		BC6H_SFLOAT,
		BC7_UNORM,
		BC7_UNORM_SRGB,

		COUNT,
	};

	enum class BindingParameterType : uint8_t
	{
		kDescriptorTable,
		kConstants,
		kConstantBufferView,
		kShaderResourceView,
		kUnorderAccessView
	};

	enum class ShaderVisibility : uint8_t
	{
		kAll = 0,
		kVertex = 1,
		kHull = 2,
		kDomain = 3,
		kGeometry = 4,
		kPixel = 5,
		kAmplification = 6,
		kMesh = 7
	};

	enum class DescriptorRangeType : uint8_t
	{
		kShaderResourceView,
		kUnorderAccessView,
		kConstantsBufferView,
		kSampler
	};

	enum class ClearFlags : uint8_t
	{
		kClearFlagDepth		= 0x1,
		kClearFlagStencil	= 0x2
	};

	RHI_ENUM_CLASS_FLAG_OPERATORS(ClearFlags);

	enum class PrimitiveTopology : uint8_t
	{
		kPointList,
		kLineList,
		kTriangleList,
		kTriangleStrip,
		kTriangleFan,
		kTriangleListWithAdjacency,
		kTriangleStripWithAdjacency,
		kPatchList
	};
	
	enum class FillMode : uint8_t
	{
		kWireframe = 2,
		kSolid = 3
	};

	enum class CullMode : uint8_t
	{
		kNone = 1,
		kFront = 2,
		kBack = 3
	};

	enum class ConservativeRasterizationMode : uint8_t
	{
		kOff = 0,
		kOn = 1
	};

	struct RasterizerDesc
	{
		FillMode fill_mode = FillMode::kSolid;
		CullMode cull_mode = CullMode::kBack;
		bool front_counter_clockwise = false;
		int32_t depth_bias = 0;
		float depth_bias_clamp = 0.0;
		float slope_scaled_depth_bias = 0.0f;
		bool depth_clip_enable = true;
		bool multisample_enable = false;
		bool antialiased_line_enable = false;
		uint32_t forced_sample_count = 0;
		ConservativeRasterizationMode conservative_raster = ConservativeRasterizationMode::kOff;
	};

	enum class BlendFactor : uint8_t
	{
		kZero = 1,
		kOne = 2,
		kSrcColor = 3,
		kInvSrcColor = 4,
		kSrcAlpha = 5,
		kInvSrcAlpha = 6,
		kDestAlpha = 7,
		kInvDestAlpha = 8,
		kDestColor = 9,
		kInvDestColor = 10,
		kSrcAlphaSat = 11,
		kBlendFactor = 14,
		kInvBlendFactor = 15,
		kSrc1Color = 16,
		kInvSrc1Color = 17,
		kSrc1Alpha = 18,
		kInvSrc1Alpha = 19
	};

	enum class BlendOp : uint8_t
	{
		kAdd = 1,
		kSubtract = 2,
		kRevSubtract = 3,
		kMin = 4,
		kMax = 5
	};

	enum class ColorMask : uint8_t
	{
		kRed = 1,
		kGreen = 2,
		kBlue = 4,
		kAlpha = 8,
		kAll = 0xF
	};	

	struct BlendDesc
	{
		struct RenderTargetBlendDesc
		{
			bool blend_enable = false;
			BlendFactor src_blend = BlendFactor::kOne;
			BlendFactor dest_blend = BlendFactor::kZero;
			BlendOp blend_op = BlendOp::kAdd;
			BlendFactor src_blend_alpha = BlendFactor::kOne;
			BlendFactor dest_blend_alpha = BlendFactor::kZero;
			BlendOp blend_op_alpha = BlendOp::kAdd;
			ColorMask render_target_write_mask = ColorMask::kAll;
		};

		bool alpha_to_coverage_enable = false;
		bool independent_blend_enable = false;

		RenderTargetBlendDesc render_target[8] = {};
	};

	enum class DepthWriteMask
	{
		kZero = 0,
		kAll = 1
	};

	
	enum class ComparisonFunc
	{
		kNever = 1,
		kLess = 2,
		kEqual = 3,
		kLessEqual = 4,
		kGreater = 5,
		kNotEqual = 6,
		kGreaterEqual = 7,
		kAlways = 8
	};

	enum class StencilOp : uint8_t
	{
		kKeep = 1,
		kZero = 2,
		kReplace = 3,
		kIncrSat = 4,
		kDecrSat = 5,
		kInvert = 6,
		kIncr = 7,
		kDecr = 8
	};

	struct DepthStencilDesc
	{
		struct DepthStencilOpDesc
		{
			StencilOp stencil_fail_op = StencilOp::kKeep;
			StencilOp stencil_depth_fail_op = StencilOp::kKeep;
			StencilOp stencil_pass_op = StencilOp::kKeep;
			ComparisonFunc stencil_func = ComparisonFunc::kAlways;
		};

		bool depth_enable = true;
		DepthWriteMask depth_write_mask = DepthWriteMask::kAll;
		ComparisonFunc depth_func = ComparisonFunc::kLess;
		bool stencil_enable = false;
		uint8_t stencil_read_mask = 0xff;
		uint8_t stencil_write_mask = 0xff;
		DepthStencilOpDesc front_face;
		DepthStencilOpDesc back_face;
	};

	struct Size
	{
		float width = 0.0;
		float height = 0.0;
	};

	struct Viewport
	{
		float top_left_x;
		float top_left_y;
		float width;
		float height;
		float min_depth;
		float max_depth;
	};

	struct Rect
	{
		uint32_t left;
		uint32_t top;
		uint32_t right;
		uint32_t bottom;
	};

	struct SampleDesc
	{
		uint32_t count;
		uint32_t quality;
	};
}
