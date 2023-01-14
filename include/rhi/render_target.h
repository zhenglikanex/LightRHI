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

	class RenderTarget
	{
	public:
		RenderTarget() = default;
		~RenderTarget() = default;

		RenderTarget(const RenderTarget&) = default;
		RenderTarget& operator=(const RenderTarget&) = default;

		RenderTarget(RenderTarget&&) = default;
		RenderTarget& operator=(RenderTarget&&) = default;

		using TextureHandleArray = std::array<TextureHandle, static_cast<uint32_t>(AttachmentPoint::kNumAttachmentPoints)>;

		void AttacthTexture(AttachmentPoint attachment_point, TextureHandle texture);

		TextureHandle GetTexture(AttachmentPoint attachment_point) const;

		const TextureHandleArray& GetTextures() const { return textures_; }
		
		uint32_t GetWidth() const;

		uint32_t GetHeight() const;

		Viewport GetViewport(float top_left_x = 0,float top_left_y = 0,float min_depth = 0,float max_depth = 1) const;

		uint32_t GetNumColors() const;

		SampleDesc GetSampleDesc() const;

	private:
		TextureHandleArray textures_;
	};
}