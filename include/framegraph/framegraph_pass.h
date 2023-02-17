#pragma once

#include <functional>

#include "rhi/device.h"

namespace light::fg
{
	class FrameGraphPassResources;

	struct FrameGraphPassConcept
	{
		FrameGraphPassConcept() = default;
		virtual ~FrameGraphPassConcept() = default;

		FrameGraphPassConcept(const FrameGraphPassConcept&) = delete;
		FrameGraphPassConcept& operator=(const FrameGraphPassConcept&) = delete;

		FrameGraphPassConcept(FrameGraphPassConcept&&) noexcept = default;
		FrameGraphPassConcept& operator=(FrameGraphPassConcept&&) noexcept = default;

		virtual void operator()(FrameGraphPassResources& resources, rhi::Device* device, rhi::CommandList* command_list) = 0;
	};

	template<typename Data,typename Execute>
	struct FrameGraphPass final : public FrameGraphPassConcept
	{
		FrameGraphPass(Execute&& execute)
			: execute_func(std::forward<Execute>(execute))
		{

		}

		void operator()(FrameGraphPassResources& resources, rhi::Device* device, rhi::CommandList* command_list) override
		{
			std::invoke(execute_func, data, resources, device, command_list);
		}

		Data data;
		Execute execute_func;
	};
}