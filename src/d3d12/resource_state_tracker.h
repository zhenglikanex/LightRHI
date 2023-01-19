#pragma once

#include <vector>
#include <unordered_map>
#include <mutex>

#include "rhi/resource.h"

#include "d3d12.h"

namespace light::rhi
{
	class D12CommandList;

	//用于跨越多个命令列表和多线程跟踪资源的状态
	//确保正确的资源状态转换，即使资源在不同线程
	//上以不同的状态使用。
	//这里指定单个命令列表不会跨越多个线程共享
	//(命令列表是单线程操作的)
	//当向ResourceStateTracker提交资源转换时，
	//首先检查该资源之前是否已经在当前命令表上使用过。
	//如果资源尚未在命令列表中使用，它将会将转换屏障
	//添加到挂起资源队列(未直接直接添加到命令列表)
	//并将资源的后续状态添已知队列。下次遇到相同资源时
	//它使用资源的已知状态作为转换之前的状态，
	//并将屏障添加到命令列表中
	//讲命令列表提交到命令队列时，将挂起的屏障与资源的
	//全局状态进行比较。如果全局状态和挂起状态不同，
	//则将挂起的屏障添加到另一个命令列表(专门用来提交中间的状态转换)
	//该列表插入到命令队列中，位于正在执行的命令列表之前。

	class ResourceStateTracker
	{
	public:
		static std::mutex s_global_mutex;

		void ResourceBarrier(const D3D12_RESOURCE_BARRIER& barrier);

		void FlushResourceBarriers(D12CommandList* command_list);

		uint32_t FlushPendingResourceBarriers(D12CommandList* command_list);

		void CommitFinalResourceStates();

		void Reset();

		void AddGlobalResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES state);
	private:
		struct ResourceState
		{
			void SetSubresourceState(UINT subresource,D3D12_RESOURCE_STATES state)
			{
				if(subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
				{
					this->state = state;
					subresource_state.clear();
				}
				else
				{
					subresource_state[subresource] = state;
				}
			}

			D3D12_RESOURCE_STATES GetSubresourceState(UINT subresource) const
			{
				auto it = subresource_state.find(subresource);
				if(it != subresource_state.end())
				{
					return it->second;
				}

				return state;
			}

			D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
			std::unordered_map<UINT, D3D12_RESOURCE_STATES> subresource_state;
		};

		using ResourceStateMap = std::unordered_map<ID3D12Resource*, ResourceState>;

		//尚未在命令中的使用过的资源,将在命令列表执行前在命令队列上执行
		//(通过添加到另个转换用的命令列表)
		std::vector<D3D12_RESOURCE_BARRIER> pending_resource_barriers_;

		// 资源屏障，需要添加到命令列表执行
		std::vector<D3D12_RESOURCE_BARRIER> resource_barriers_;

		ResourceStateMap final_resource_state_;

		static ResourceStateMap s_global_resource_state_;

	};
}
