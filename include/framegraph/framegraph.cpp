#include "framegraph.h"

#include <stack>

namespace light::fg
{
	FrameGraph::Builder::Builder(FrameGraph& framegraph, PassNode& pass_node)
		: framegraph_(framegraph)
		, pass_node_(pass_node)
	{
	}

	FrameGraphHandle FrameGraph::Builder::Read(FrameGraphHandle handle)
	{
		return pass_node_.Read(handle);
	}

	FrameGraphHandle FrameGraph::Builder::Write(FrameGraphHandle handle)
	{
		// 对写入的导入资源passnode标记有效
		if (framegraph_.GetFrameGraphResource(handle).IsImported())
		{
			pass_node_.SideEffect();
		}

		// 是否是自己创建的resource
		if (pass_node_.HasCreate(handle))
		{
			pass_node_.Write(handle);
		}
		else 
		{
			// 确保pass_node有读取resource
			pass_node_.Read(handle);

			// 创建新的resource_node,并写入
			handle = framegraph_.CreateNewVersionNode(handle);

			pass_node_.Write(handle);
		}

		return handle;
	}
	void FrameGraph::Builder::SetSideEffect()
	{
		pass_node_.SideEffect();
	}

	void FrameGraph::Clear()
	{
		resources_.clear();
		pass_nodes_.clear();
		resource_nodes_.clear();
	}

	void FrameGraph::Compile()
	{
		for (auto& pass_node : pass_nodes_)
		{
			pass_node.ref_count = pass_node.writes.size();

			for(auto handle : pass_node.reads)
			{
				auto& resource_node = resource_nodes_[handle];
				++resource_node.ref_count;
			}

			for (auto handle : pass_node.writes)
			{
				auto& resource_node = resource_nodes_[handle];
				resource_node.producer = &pass_node;
			}
		}

		// culling
		std::stack<ResourceNode* > unreferenced_resources;
		for (auto& node : resource_nodes_)
		{
			if (node.ref_count == 0)
			{
				unreferenced_resources.push(&node);
			}
		}

		while (!unreferenced_resources.empty())
		{
			auto& resource_node = *unreferenced_resources.top();
			unreferenced_resources.pop();

			auto producer = resource_node.producer;

			if (!producer || producer->HasSideEffect())
			{
				continue;
			}
			
			for (auto& handle : producer->reads)
			{
				if (--resource_nodes_[handle].ref_count == 0)
				{
					unreferenced_resources.push(&resource_nodes_[handle]);
				}
			}
		}

		//lifetime
		for (auto& pass_node : pass_nodes_)
		{
			for (auto handle : pass_node.creates)
			{
				GetFrameGraphResource(handle).producer = &pass_node;
			}

			for (auto handle : pass_node.reads)
			{
				GetFrameGraphResource(handle).last = &pass_node;
			}

			for (auto handle : pass_node.writes)
			{
				GetFrameGraphResource(handle).last = &pass_node;
			}
		}
	}

	void FrameGraph::Execute(rhi::Device* device)
	{
		for (auto& pass_node : pass_nodes_)
		{
			if (pass_node.CanExecute())
			{
				continue;
			}

			for (auto handle : pass_node.creates)
			{
				GetFrameGraphResource(handle).Create();
			}

			FrameGraphPassResources resource(*this, pass_node);
			std::invoke(*pass_node.execute, resource, device, nullptr);
			
			for (auto& resource : resources_)
			{
				if (resource.last == &pass_node && !resource.IsImported())
				{
					resource.Destroy();
				}
			}
		}
	}

	PassNode& FrameGraph::CreatePassNode(std::string_view name, std::unique_ptr<FrameGraphPassConcept> pass)
	{
		uint32_t id = pass_nodes_.size();
		auto& result = pass_nodes_.emplace_back(name, id, std::move(pass));
		return result;
	}

	ResourceNode& FrameGraph::CreateResourceNode(std::string_view name, uint32_t rid,uint32_t version)
	{
		uint32_t id = resource_nodes_.size();
		return resource_nodes_.emplace_back(name, id, rid, version);
	}

	FrameGraphHandle FrameGraph::CreateNewVersionNode(FrameGraphHandle handle)
	{
		auto& resource_node = resource_nodes_[handle];
		auto& resource = GetFrameGraphResource(resource_node.rid);
		return CreateResourceNode(resource_node.name, resource_node.rid, ++resource.version).id;
	}

	FrameGraphResource& FrameGraph::GetFrameGraphResource(FrameGraphHandle handle)
	{
		const auto& node = resource_nodes_[handle]; 
		return resources_[node.rid];
	}

	FrameGraphPassResources::FrameGraphPassResources(FrameGraph& framegraph, PassNode& pass_node)
		: framegraph_(framegraph)
		, pass_node_(pass_node)
	{

	}
}