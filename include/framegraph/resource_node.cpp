#include "resource_node.h"

namespace light::fg
{
	ResourceNode::ResourceNode(std::string_view name, uint32_t id, uint32_t rid, uint32_t version)
		: GraphNode(name, id)
		, rid(rid)
		, version(0)
		, producer(nullptr)
	{
	}
}
