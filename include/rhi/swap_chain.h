#pragma once

#include "types.h"
#include "base.h"
#include "resource.h"
#include "render_target.h"

namespace light::rhi
{
	class Device;

	class SwapChain : public Resource
	{
	public:

		virtual uint32_t Present() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual RenderTarget GetRenderTarget() = 0;
	};

	using SwapChainHandle = Handle<SwapChain>;
}