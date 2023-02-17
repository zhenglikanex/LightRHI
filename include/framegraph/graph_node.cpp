#include "graph_node.h"

namespace light::fg
{
	GraphNode::GraphNode(std::string_view name, uint32_t id)
		: name(name)
		, id(id)
		, ref_count(0)
	{
	}
}