#include "game.h"

#include <thread>
#include <chrono>

using namespace light;
using namespace light::rhi;

struct Vertex
{
	float x;
	float y;
	float z;
};

struct Color
{
	float r;
	float g;
	float b;
	float a;
};

class TestGame final : public Game
{
public:
	explicit TestGame(const GameParams& params)
		: Game(params)
	{
	}

	~TestGame() override
	= default;

	void OnResize(uint32_t width, uint32_t height) override
	{
		Game::OnResize(width, height);
	}

	bool OnInit() override
	{
		auto command_queue = device_->GetCommandQueue(CommandListType::kDirect);
		auto command_list = command_queue->GetCommandList();

		std::vector<VertexAttributeDesc> vertex_attributes =
		{
			{"POSITION",0,Format::RGB32_FLOAT,0,0u,false}
		};

		std::vector<Vertex> vertexs
		{
			{ -0.5f, -0.5f, 0.5f },
			{ -0.5f, +0.5f, 0.5f },
			{ +0.5f, +0.5f, 0.5f },
		};

		std::vector<uint16_t> indices
		{
			0,1,2
		};

		BufferDesc vertex_desc;
		vertex_desc.type = BufferType::kVertex;
		vertex_desc.format = Format::RGB32_FLOAT;
		vertex_desc.size_in_bytes = sizeof(Vertex) * 3;
		vertex_desc.stride = sizeof(Vertex);
		vertex_buffer_ = device_->CreateBuffer(vertex_desc);
		
		command_list->WriteBuffer(vertex_buffer_, reinterpret_cast<uint8_t*>(vertexs.data()), vertexs.size() * sizeof(Vertex));

		BufferDesc index_desc;
		index_desc.format = Format::R16_UINT;
		index_desc.type = BufferType::kIndex;
		index_desc.size_in_bytes = sizeof(uint16_t) * 3;
		index_desc.stride = sizeof(uint16_t);
		index_buffer_ = device_->CreateBuffer(index_desc);

		command_list->WriteBuffer(index_buffer_, reinterpret_cast<uint8_t*>(indices.data()), indices.size() * sizeof(uint16_t));

		TextureDesc depth_tex_desc;
		depth_tex_desc.format = Format::D24S8;
		depth_tex_desc.width = swap_chain_->GetWidth();
		depth_tex_desc.height = swap_chain_->GetHeight();

		depth_stencil_texture_ = device_->CreateTexture(depth_tex_desc);

		BindingParameter parameter1;
		parameter1.InitAsConstants(4, 0);
		
		BindingParameter parameter2;
		parameter2.InitAsConstantBufferView(1);

		BindingParameter::DescriptorRange range;
		range.base_shader_register = 2;
		range.num_descriptors = 1;
		range.range_type = DescriptorRangeType::kConstantsBufferView;

		BindingParameter parameter3;
		parameter3.InitAsDescriptorTable(1, &range);

		BindingLayout* binding_layout = new BindingLayout(2);
		binding_layout->Add(0, parameter1);
		binding_layout->Add(1, parameter2);
		binding_layout->Add(2, parameter3);

		GraphicsPipelineDesc pso_desc;
		pso_desc.input_layout = device_->CreateInputLayout(std::move(vertex_attributes));
		pso_desc.binding_layout = BindingLayoutHandle::Create(binding_layout);
		pso_desc.vs = device_->CreateShader(ShaderType::kVertex, "shaders/color.hlsl", "VS", "vs_5_0");
		pso_desc.ps = device_->CreateShader(ShaderType::kPixel, "shaders/color.hlsl", "PS", "ps_5_0");
		pso_desc.primitive_type = PrimitiveTopology::kTriangleList;

		RenderTarget rt = swap_chain_->GetRenderTarget();
		rt.AttacthAttachment(AttachmentPoint::kDepthStencil, depth_stencil_texture_);

		pso_ = device_->CreateGraphicsPipeline(pso_desc,rt);

		command_list->ExecuteCommandList();

		command_queue->Flush();

		return true;
	}

	void OnUpdate(double dt) override
	{
		const auto command_list = device_->GetCommandList(CommandListType::kDirect);

		RenderTarget rt = swap_chain_->GetRenderTarget();
		rt.AttacthAttachment(AttachmentPoint::kDepthStencil, depth_stencil_texture_);

		command_list->SetRenderTarget(rt);
		command_list->SetViewport(rt.GetViewport());
		command_list->SetScissorRect({ 0,0,std::numeric_limits<int32_t>::max(),std::numeric_limits<int32_t>::max() });

		constexpr float clear_color[] = { 1.0, 0.0, 0.0, 1.0 };
		command_list->ClearTexture(rt.GetAttachment(AttachmentPoint::kColor0).texture,clear_color);

		command_list->ClearDepthStencilTexture(depth_stencil_texture_, ClearFlags::kClearFlagDepth | ClearFlags::kClearFlagStencil, 1, 0);

		command_list->SetGraphicsPipeline(pso_);

		Color color{ 1.0,0.0,1.0,1.0 };
		command_list->SetGraphics32BitConstants(0, color);
		command_list->SetGraphicsDynamicConstantBuffer(1, color);

		BufferDesc desc;
		desc.type = BufferType::kConstant;
		desc.size_in_bytes = sizeof(Color);
		desc.format = Format::RGB32_FLOAT;

		BufferHandle buf = device_->CreateBuffer(desc);
		command_list->WriteBuffer(buf, (uint8_t*)&color, sizeof(Color));
		command_list->SetConstantBufferView(2,0,buf);

		command_list->SetPrimitiveTopology(PrimitiveTopology::kTriangleList);
		command_list->SetVertexBuffer(0, vertex_buffer_);
		command_list->SetIndexBuffer(index_buffer_);

		command_list->DrawIndexed(3, 1, 0, 0, 0);

		command_list->ExecuteCommandList();
		
		swap_chain_->Present();

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	void OnRender(double dt) override
	{
		
	}
private:
	TextureHandle depth_stencil_texture_;
	GraphicsPipelineHandle pso_;
	BufferHandle vertex_buffer_;
	BufferHandle index_buffer_;
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