#pragma once

#include <queue>
#include <mutex>

namespace light::rhi
{
	template<class T>
	class ThreadSafeQueue
	{
	public:
		ThreadSafeQueue()
		{
			
		}
		ThreadSafeQueue(const ThreadSafeQueue& rhs)
		{
			std::unique_lock<std::mutex> lock(rhs.mutex_);
			queue_ = rhs.queue_;
		}

		void Push(T value)
		{
			std::unique_lock<std::mutex> lock(mutex_);
			queue_.push(std::move(value));
		}

		bool TryPop(T& value)
		{
			std::unique_lock<std::mutex> lock(mutex_);
			if(queue_.empty())
			{
				return false;
			}

			value = queue_.front();
			queue_.pop();

			return true;
		}

		bool Empty() const
		{
			std::unique_lock<std::mutex> lock(mutex_);
			return queue_.empty();
		}

		size_t Size() const
		{
			std::unique_lock<std::mutex> lock(mutex_);
			return queue_.size();
		}
	private:
		mutable std::mutex mutex_;
		std::queue<T> queue_;
	};
}