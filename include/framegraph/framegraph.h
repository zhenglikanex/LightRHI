#pragma once

#include <memory>
#include <any>
#include <typeindex>
#include <unordered_map>

#include "pass_node.h"
#include "resource_node.h"
#include "framegraph_pass.h"

#include "rhi/device.h"
#include "rhi/command_list.h"

namespace light::fg
{
	class FrameGraph
	{
	public:
		friend class FrameGraphPassResources;

		class Blackbord
		{
		public:
			Blackbord() = default;
			~Blackbord() = default;

			Blackbord(const Blackbord&) = delete;
			Blackbord(Blackbord&&) = delete;

			Blackbord& operator=(const Blackbord&) = delete;
			Blackbord& operator=(Blackbord&&) = delete;

			template<typename Data>
			void Add(Data&& data)
			{
				storage_.emplace(typeid(Data), std::forward<Data>(data));
			}

			template<typename Data>
			Data& Get()
			{
				auto it = storage_.find(typeid(Data));
				return std::any_cast<const Data&>(it->second);
			}

			template<typename Data>
			Data* TryGet()
			{
				auto it = storage_.find(typeid(Data));
				if (it != storage_.end())
				{
					return &std::any_cast<Data>(it->second);
				}

				return nullptr;
			}

			template<typename Data>
			bool Contains() const
			{
				return storage_.find(typeid(Data)) != storage_.end();
			}
		private:
			std::unordered_map<std::type_index, std::any> storage_;
		};

		class Builder
		{
		public:
			Builder(FrameGraph& framegraph, PassNode& pass_node);

			template<typename T>
			FrameGraphHandle Create(std::string_view name, typename T::Desc&& desc)
			{
				return CreateFrameGraphResource(name, std::forward<typename T::Desc>(desc));
			}

			FrameGraphHandle Read(FrameGraphHandle handle);
			FrameGraphHandle Write(FrameGraphHandle handle);

			void SetSideEffect();
		private:
			FrameGraph& framegraph_;
			PassNode& pass_node_;
		};
		

		void Clear();

		template<typename T>
		FrameGraphHandle Import(std::string_view name, typename T::Desc&& desc)
		{
			return CreateFrameGraphResource(name, std::forward<typename T::Desc>(desc),true);
		}

		template<typename Data,typename Setup,typename Execute>
		const Data& AddPass(std::string_view name,Setup&& setup, Execute&& execute)
		{
			static_assert(std::is_invocable_v<Setup, Builder, Data>, "invalid setup callback");
			static_assert(std::is_invocable_v<Execute, const Data&,
				FrameGraphPassResources&, rhi::Device*, rhi::CommandList*>, "invalid execute callback");

			auto* pass = new FrameGraphPass<Data, Execute>(std::forward<Execute>(execute));
			std::unique_ptr<FrameGraphPassConcept> pass_concept(pass);

			auto& pass_node = CreatePassNode(name, std::move(pass_concept));

			Builder builder(*this, pass_node);
			std::invoke(setup, builder, pass->data);
			
			return pass->data;
		}

		void Compile();
		
		void Execute(rhi::Device* device);
	private:
		template<typename T>
		FrameGraphHandle CreateFrameGraphResource(std::string_view name, typename T::Desc&& desc,bool import = false)
		{
			auto rid = resources_.size();
			resources_.emplace_back(rid, std::forward<typename T::Desc>(desc), 0,import);

			auto& node = CreateResourceNode(name, rid,0);

			return node.id;
		}

		PassNode& CreatePassNode(std::string_view name, std::unique_ptr<FrameGraphPassConcept> pass);
		
		ResourceNode& CreateResourceNode(std::string_view name,uint32_t rid,uint32_t version);

		FrameGraphHandle CreateNewVersionNode(FrameGraphHandle handle);

		FrameGraphResource& GetFrameGraphResource(FrameGraphHandle handle);

		std::vector<FrameGraphResource> resources_;
		std::vector<PassNode> pass_nodes_;
		std::vector<ResourceNode> resource_nodes_;
	};	

	class FrameGraphPassResources
	{
	public:
		FrameGraphPassResources(FrameGraph& framegraph, PassNode& pass_node);
		~FrameGraphPassResources() = default;

		FrameGraphPassResources(const FrameGraphPassResources&) = delete;
		FrameGraphPassResources(FrameGraphPassResources&&) = delete;

		FrameGraphPassResources& operator=(const FrameGraphPassResources&) = delete;
		FrameGraphPassResources& operator=(FrameGraphPassResources&&) = delete;

		template<typename T>
		T& Get(FrameGraphHandle handle)
		{
			return framegraph_.GetFrameGraphResource(handle).Get();
		}

		template<typename T>
		typename T::Desc& GetDesc(FrameGraphHandle handle)
		{
			return framegraph_.GetFrameGraphResource(handle).GetDesc();
		}
	private:
		FrameGraph& framegraph_;
		PassNode& pass_node_;
	};
}

