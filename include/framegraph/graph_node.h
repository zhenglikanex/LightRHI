#pragma once

#include <string_view>
#include <string>

namespace light::fg
{
	struct GraphNode
	{
		GraphNode(std::string_view name, uint32_t id);
		virtual ~GraphNode() = default;

		GraphNode(const GraphNode&) = delete;
		GraphNode(GraphNode&&) = default;

		GraphNode& operator=(const GraphNode&) = delete;
		GraphNode& operator=(GraphNode&&) = default;
	
		const std::string name;
		const uint32_t id;
		int32_t ref_count;
	};
}