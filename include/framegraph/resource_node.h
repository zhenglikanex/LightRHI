#pragma once

#include "graph_node.h"
#include "framegraph_resource.h"

namespace light::fg
{
	struct PassNode;
	struct ResourceNode final : public GraphNode
	{
		ResourceNode(std::string_view name, uint32_t id, uint32_t rid,uint32_t version);
	
		const uint32_t rid;
		const uint32_t version;

		PassNode* producer;
	};
}