#include "pass_node.h"

namespace light::fg
{
	PassNode::PassNode(std::string_view name, uint32_t id, std::unique_ptr<FrameGraphPassConcept>&& execute)
		: GraphNode(name, id)
		, execute(std::move(execute))
	{
	}

	FrameGraphHandle PassNode::Create(FrameGraphHandle handle)
	{
		auto it = std::find(creates.begin(), creates.end(), handle);
		if (it != creates.end())
		{
			return *it;
		}

		return creates.emplace_back(handle);
	}
	FrameGraphHandle PassNode::Read(FrameGraphHandle handle)
	{
		auto it = std::find(reads.begin(), reads.end(), handle);
		if (it != reads.end())
		{
			return *it;
		}

		return reads.emplace_back(handle);
	}
	FrameGraphHandle PassNode::Write(FrameGraphHandle handle)
	{
		auto it = std::find(writes.begin(), writes.end(), handle);
		if (it != writes.end())
		{
			return *it;
		}

		return writes.emplace_back(handle);
	}
	bool PassNode::HasCreate(FrameGraphHandle handle) const
	{
		auto it = std::find(creates.begin(), creates.end(), handle);
		return it != creates.end();
	}
}