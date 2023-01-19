#pragma once

#include <vector>
#include <memory>

#include "rhi/device.h"
#include "rhi/swap_chain.h"

#include "GLFW/glfw3.h"

namespace light
{
	struct GameParams
	{
		uint32_t width = 1280;
		uint32_t height = 720;
		rhi::GraphicsApi api = rhi::GraphicsApi::kD3D12;
	};

	class Game
	{
	public:
		explicit Game(const GameParams& desc);

		virtual ~Game() = 0 {};

		Game(const Game&) = delete;
		Game(Game&&) = delete;

		Game& operator=(const Game&) = delete;
		Game& operator=(Game&&) = delete;

		bool Initialize();

		void Shutdown();;

		void Run();

		virtual void OnResize(uint32_t width, uint32_t height);

		virtual bool OnInit() = 0;

		virtual void OnUpdate(double dt) = 0;

		virtual void OnRender(double dt) = 0;
	private:
		bool InitWindow();

		bool InitDeviceAndSwapChain();

	protected:
		GameParams params_;
		GLFWwindow* window_;
		rhi::DeviceHandle device_;
		rhi::SwapChainHandle swap_chain_;
		double last_frame_time_;
	};
}
