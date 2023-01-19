#pragma once

#include <string>

#include "resource.h"
#include "types.h"

namespace light::rhi
{
	struct TextureDesc
	{
		uint32_t width = 0;
		uint32_t height = 0;
		Format format = Format::UNKNOWN;

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