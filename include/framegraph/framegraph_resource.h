#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace light::fg
{
	struct PassNode;

	struct FrameGraphResource
	{
		struct Concept
		{
			virtual ~Concept() = default;

			virtual void Create() = 0;
			virtual void Destroy() = 0;

			virtual std::string ToString() = 0;
		};

		template<typename T>
		struct Model : public Concept
		{
			Model(typename T::Desc&& desc, T&& resource)
				: desc(std::forward(desc))
				, resource(std::forward(resource))
			{

			}

			void Create() override
			{
				resource.Create(desc);
			}

			void Destroy() override
			{
				resource.Destroy(desc);
			}

			typename const T::Desc desc;
			T resource;
		};

		template<typename T>
		FrameGraphResource(uint32_t id, typename T::Desc&& desc, T&& resource, uint32_t version, bool imported = false)
			: id(id)
			, concept_model(std::make_unique<Model<T>>(std::forward(desc), std::forward(resource)))
			, version(version)
			, imported(imported)
			, producer(nullptr)
			, last(nullptr)
		{

		}

		void Create();
		void Destroy();

		bool IsImported() const { return imported; }
	
		template<typename T>
		T& Get()
		{
			return GetModel<T>()->resource;
		}

		template<typename T>
		typename T::Desc& GetDesc()
		{
			return GetModel<T>()->desc;
		}
	
		template<typename T>
		auto* GetModel()
		{
			return static_cast<Model<T>*>(concept_model.get());
		}

		const uint32_t id;
		std::unique_ptr<Concept> concept_model;
		uint32_t version;		// 每一次声明是递增
		const bool imported;	// 是否是导入的资源

		// 用于标记resource的生命周期
		PassNode* producer;
		PassNode* last;
	};
}