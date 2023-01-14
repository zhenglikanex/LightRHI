#include "rhi/render_target.h"

#undef max;
#undef min;

namespace light::rhi
{
	void RenderTarget::AttacthTexture(AttachmentPoint attachment_point, TextureHandle texture)
	{
		textures_[static_cast<uint32_t>(attachment_point)] = texture;
	}

	TextureHandle RenderTarget::GetTexture(AttachmentPoint attachment_point) const
	{
		return textures_[static_cast<uint32_t>(attachment_point)];
	}

	uint32_t RenderTarget::GetWidth() const
	{
		uint32_t width = 0;
		for (auto& texture : textures_)
		{
			if (texture)
			{
				const TextureDesc& desc = texture->GetDesc();
				width = std::max(width, desc.width);
			}
		}

		return width;
	}

	uint32_t RenderTarget::GetHeight() const
	{
		uint32_t height = 0;
		for (auto& texture : textures_)
		{
			if (texture)
			{
				const TextureDesc& desc = texture->GetDesc();
				height = std::max(height, desc.height);
			}
		}

		return height;
	}

	Viewport RenderTarget::GetViewport(float top_left_x, float top_left_y, float min_depth, float max_depth) const
	{
		float width = GetWidth();
		float height = GetHeight();

		return { top_left_x,top_left_y,width,height,min_depth,max_depth };
	}

	uint32_t RenderTarget::GetNumColors() const
	{
		uint32_t num = 0;

		for (uint32_t i = 0; i < static_cast<uint32_t>(AttachmentPoint::kDepthStencil); ++i)
		{
			if (textures_[i])
			{
				++num;
			}
		}

		return num;
	}

	SampleDesc RenderTarget::GetSampleDesc() const
	{
		// todo
		return { 1,0 };
	}
}
