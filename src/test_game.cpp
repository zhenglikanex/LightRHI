#include "game.h"

using namespace light;
using namespace light::rhi;

class TestGame final : public Game
{
public:
	explicit TestGame(const GameParams& params)
		: Game(params)
	{
	}

	~TestGame() override
	{
		
	}

	void OnResize(uint32_t width, uint32_t height) override
	{
		Game::OnResize(width, height);
	}

	bool OnInit() override
	{
		std::vector<VertexAttributeDesc> vertex_attributes =
		{
			{"POSITION",0,Format::RGB32_FLOAT,0,~0u,false}
		};

		RenderTarget rt = swap_chain_->GetRenderTarget();
		//rt.AttacthTexture(AttachmentPoint::kDepthStencil, depth_stencil_texture_);

		GraphicsPipelineDesc pso_desc;
		pso_desc.input_layout = device_->CreateInputLayout(std::move(vertex_attributes));

		pso_ = device_->CreateGraphicsPipeline(pso_desc,rt);

		return true;
	}

	void OnUpdate(double dt) override
	{
		auto command_list = device_->GetCommandList(CommandListType::kDirect);

		RenderTarget rt = swap_chain_->GetRenderTarget();

		command_list->SetRenderTarget(rt);
		command_list->SetViewport(rt.GetViewport());
		command_list->SetScissorRect({ 0,0,std::numeric_limits<uint32_t>::max(),std::numeric_limits<uint32_t>::max() });

		float clear_color[] = { 1.0, 0.0, 0.0, 1.0 };
		command_list->ClearTexture(rt.GetTexture(AttachmentPoint::kColor0),clear_color);

		command_list->SetGraphicsPipeline(pso_);

		command_list->ExecuteCommandList();

		swap_chain_->Present();
	}

	void OnRender(double dt) override
	{
		
	}
private:
	TextureHandle depth_stencil_texture_;
	GraphicsPipelineHandle pso_;
};

int main()
{
	GameParams params;

	TestGame game(params);

	if(game.Initialize())
	{
		game.Run();
	}

	game.Shutdown();
	
	return 0;
}