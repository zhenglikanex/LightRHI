#pragma once

#include <string>
#include "rhi/resource.h"

namespace light::rhi
{
	struct TextureDesc
	{
		uint32_t width;
		uint32_t height;
		Format format;

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