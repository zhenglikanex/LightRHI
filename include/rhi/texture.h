#pragma once

#include <string>

#include "resource.h"
#include "types.h"

namespace light::rhi
{
	struct TextureDesc
	{
		uint32_t width = 1;
		uint32_t height = 1;
		uint32_t depth = 1;
		uint32_t array_size = 1;
		uint32_t mip_levels = 1;
		Format format = Format::UNKNOWN;
		TextureDimension dimension = TextureDimension::kTexture2D;

		std::string debug_name;
	};

	class Texture : public Resource
	{
	public:
		explicit Texture(const TextureDesc& desc)
			: desc_(desc)
			, permanent_state_(false)
		{

		}

		const TextureDesc& GetDesc() const { return desc_; }

		void SetPermanentState(bool value) { permanent_state_ = value; }

		bool IsPermanentState() const { return permanent_state_; }

	protected:
		TextureDesc desc_;
		bool permanent_state_;
	};

	using TextureHandle = Handle<Texture>;
}