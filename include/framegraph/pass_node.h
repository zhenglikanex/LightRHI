#pragma once

#include <vector>
#include <memory>

#include "graph_node.h"
#include "framegraph_pass.h"

namespace light::fg
{
	using FrameGraphHandle = uint32_t;

	struct PassNode final : public GraphNode
	{
		PassNode(std::string_view name, uint32_t id, std::unique_ptr<FrameGraphPassConcept>&& execute);

		FrameGraphHandle Create(FrameGraphHandle handle);
		FrameGraphHandle Read(FrameGraphHandle handle);
		FrameGraphHandle Write(FrameGraphHandle handle);

		void SideEffect() { side_effect = true; }

		bool HasSideEffect() const { return side_effect; }
		bool CanExecute() const { return ref_count > 0 || HasSideEffect(); }

		bool HasCreate(FrameGraphHandle handle) const;

		std::vector<FrameGraphHandle> creates;
		std::vector<FrameGraphHandle> reads;
		std::vector<FrameGraphHandle> writes;

		std::unique_ptr<FrameGraphPassConcept> execute;

		bool side_effect;
	};
}