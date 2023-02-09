#pragma once

#include <iostream>
#include <chrono>
#include <string>
#include <memory>
#include <thread>

#include "rhi/thread_safe_queue.hpp"

class AutoTimer
{
public:
	AutoTimer(const std::string& name)
		:name_(name)
		, start_(std::chrono::high_resolution_clock::now())
	{
		if(!thread_)
		{
			thread_ = std::make_unique<std::thread>([]()
				{
					std::string msg;
					while(true)
					{
						while (AutoTimer::queue_.TryPop(msg))
						{
							std::cout << msg << std::endl;
						}

						std::this_thread::sleep_for(std::chrono::seconds(1));
					}
				});
		}
	}

	~AutoTimer()
	{
		Dump("");
	}

	void Dump(const std::string& desc)
	{
		auto dt = std::chrono::high_resolution_clock::now() - start_;
		std::string str = desc + name_ + ":" + std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(dt).count()) + "\n";
		queue_.Push(std::move(str));
	}

private:
	std::string name_;
	std::chrono::high_resolution_clock::time_point start_;
public:
	static light::rhi::ThreadSafeQueue<std::string> queue_;
	static std::unique_ptr<std::thread> thread_;
};

inline light::rhi::ThreadSafeQueue<std::string> AutoTimer::queue_;
inline std::unique_ptr<std::thread> AutoTimer::thread_;