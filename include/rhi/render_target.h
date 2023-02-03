#pragma once

#include <array>

#include "types.h"
#include "texture.h"

namespace light::rhi
{
	enum class AttachmentPoint : uint8_t
	{
		kColor0,
		kColor1,
		kColor2,
		kColor3,
		kColor4,
		kColor5,
		kColor6,
		kColor7,
		kDepthStencil,
		kNumAttachmentPoints
	};

	struct Attachment
	{
		TextureHandle texture = nullptr;

		Format format = Format::UNKNOWN;
		uint32_t mip_level = -1;
		uint32_t array_slice = -1;
		uint32_t num_array_slice = -1;

		bool IsAllSubresource() const
		{
			return mip_level == -1 && array_slice == -1 && num_array_slice == -1;
		}
	};

	class RenderTarget
	{
	public:
		RenderTarget() = default;
		~RenderTarget() = default;

		RenderTarget(const RenderTarget&) = default;
		RenderTarget& operator=(const RenderTarget&) = default;

		RenderTarget(RenderTarget&&) = default;
		RenderTarget& operator=(RenderTarget&&) = default;

		using AttachmentArray = std::array<Attachment, static_cast<uint32_t>(AttachmentPoint::kNumAttachmentPoints)>;

		void AttacthAttachment(AttachmentPoint attachment_point, TextureHandle texture);

		void AttacthAttachment(AttachmentPoint attachment_point, TextureHandle texture, uint32_t mip_level,uint32_t array_slice = 0);

		Attachment GetAttachment(AttachmentPoint attachment_point) const;

		const AttachmentArray& GetAttachments() const { return attachments_; }
		
		uint32_t GetWidth() const;

		uint32_t GetHeight() const;

		Viewport GetViewport(float top_left_x = 0,float top_left_y = 0,float min_depth = 0,float max_depth = 1) const;

		uint32_t GetNumColors() const;

		SampleDesc GetSampleDesc() const;

	private:
		AttachmentArray attachments_;
	};
}