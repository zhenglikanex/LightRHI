#include "rhi/render_target.h"

namespace light::rhi
{
	void RenderTarget::AttacthAttachment(AttachmentPoint attachment_point, TextureHandle texture)
	{
		Attachment attachment;
		attachment.texture = texture;

		attachments_[static_cast<uint32_t>(attachment_point)] = attachment;
	}

	void RenderTarget::AttacthAttachment(AttachmentPoint attachment_point, TextureHandle texture, uint32_t mip_level,
		uint32_t array_slice)
	{
		Attachment attachment;
		attachment.texture = texture;
		attachment.mip_level = mip_level;
		attachment.array_slice = array_slice;

		attachments_[static_cast<uint32_t>(attachment_point)] = attachment;
	}

	Attachment RenderTarget::GetAttachment(AttachmentPoint attachment_point) const
	{
		return attachments_[static_cast<uint32_t>(attachment_point)];
	}

	uint32_t RenderTarget::GetWidth() const
	{
		uint32_t width = 0;
		for (auto& attachment : attachments_)
		{
			if (attachment.texture)
			{
				const TextureDesc& desc = attachment.texture->GetDesc();
				width = std::max(width, desc.width);
			}
		}

		return width;
	}

	uint32_t RenderTarget::GetHeight() const
	{
		uint32_t height = 0;
		for (auto& attachment : attachments_)
		{
			if (attachment.texture)
			{
				const TextureDesc& desc = attachment.texture->GetDesc();
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
			if (attachments_[i].texture)
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
