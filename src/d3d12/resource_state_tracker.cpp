#include "resource_state_tracker.h"

#include <cassert>

#include "d12_command_list.h"

namespace light::rhi
{
	std::mutex ResourceStateTracker::s_global_mutex;
	ResourceStateTracker::ResourceStateMap ResourceStateTracker::s_global_resource_state_;

	void ResourceStateTracker::ResourceBarrier(const D3D12_RESOURCE_BARRIER& barrier)
	{
		if(barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
		{
			const D3D12_RESOURCE_TRANSITION_BARRIER& transition_barrier = barrier.Transition;

			//检查是否存在final队列中，如果存在说明在当前命令列表中使用过
			const auto iter = final_resource_state_.find(transition_barrier.pResource);
			if(iter != final_resource_state_.end())
			{
				auto& resource_state = iter->second;

				if(transition_barrier.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES &&
					!resource_state.subresource_state.empty())
				{
					for(auto subresource_state : resource_state.subresource_state)
					{
						// 判断资源最后的状态是否就是最后一次使用的状态
						if(transition_barrier.StateAfter != subresource_state.second)
						{
							// 将barrier的StateBefore修改为记录的最后的状态(barrier中的StateBefore传入的时候是未知的)
							D3D12_RESOURCE_BARRIER new_barrier = barrier;
							new_barrier.Transition.Subresource = subresource_state.first;
							new_barrier.Transition.StateBefore = subresource_state.second;

							resource_barriers_.push_back(new_barrier);
						}
					}
				}
				else
				{
					
					auto final_state = resource_state.GetSubresourceState(transition_barrier.Subresource);

					// 判断资源最后的状态是否就是最后一次使用的状态
					if(transition_barrier.StateAfter != final_state)
					{
						// 将barrier的StateBefore修改为记录的最后的状态(barrier中的StateBefore传入的时候是未知的)
						D3D12_RESOURCE_BARRIER new_barrier = barrier;
						new_barrier.Transition.StateBefore = final_state;

						resource_barriers_.push_back(new_barrier);
					}
				}
			}
			else
			{
				// 未使用过的资源,添加到pending队列中,
				// 需要在命令列表执行前添加到queue
				pending_resource_barriers_.push_back(barrier);
			}

			final_resource_state_[transition_barrier.pResource].SetSubresourceState(transition_barrier.Subresource, 
				transition_barrier.StateAfter);
		}
		else
		{
			resource_barriers_.push_back(barrier);
		}
	}

	void ResourceStateTracker::FlushResourceBarriers(D12CommandList* command_list)
	{
		if(resource_barriers_.empty())
		{
			return;
		}

		command_list->GetD3D12GraphicsCommandList()->ResourceBarrier(resource_barriers_.size(), resource_barriers_.data());
		resource_barriers_.clear();
	}

	uint32_t ResourceStateTracker::FlushPendingResourceBarriers(D12CommandList* command_list)
	{
		std::vector<D3D12_RESOURCE_BARRIER> resource_barriers;
		resource_barriers.reserve(pending_resource_barriers_.size() * 2);

		for(auto  pending_barrier : pending_resource_barriers_)
		{
			auto it = s_global_resource_state_.find(pending_barrier.Transition.pResource);
			if (it != s_global_resource_state_.end())
			{
				if(pending_barrier.Transition.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES && 
					!it->second.subresource_state.empty())
				{
					for(auto subresource_state : it->second.subresource_state)
					{
						if(pending_barrier.Transition.StateAfter != subresource_state.second)
						{
							D3D12_RESOURCE_BARRIER barrier = pending_barrier;
							barrier.Transition.StateBefore = subresource_state.second;
							resource_barriers.push_back(barrier);
						}
					}
				}
				else
				{
					D3D12_RESOURCE_BARRIER barrier = pending_barrier;
					barrier.Transition.StateBefore = it->second.GetSubresourceState(pending_barrier.Transition.Subresource);
					resource_barriers.push_back(barrier);
				}
			}
		}

		if(!resource_barriers.empty())
		{
			command_list->GetD3D12GraphicsCommandList()->ResourceBarrier(resource_barriers.size(), resource_barriers.data());
		}

		pending_resource_barriers_.clear();

		return resource_barriers.size();
	}

	void ResourceStateTracker::CommitFinalResourceStates()
	{
		for(auto& final_state : final_resource_state_)
		{
			s_global_resource_state_[final_state.first] = final_state.second;
		}

		final_resource_state_.clear();
	}

	void ResourceStateTracker::Reset()
	{
		final_resource_state_.clear();
		pending_resource_barriers_.clear();
		resource_barriers_.clear();
	}

	void ResourceStateTracker::AddGlobalResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES state)
	{
		std::unique_lock<std::mutex> lock(s_global_mutex);
		s_global_resource_state_[resource].SetSubresourceState(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, state);
	}
}
