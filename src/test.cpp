//#include "d3d12/d12_device.h"
//
//#include <numeric>
//
//#undef max
//#undef min
//
//using namespace light::rhi;
//
//enum Params : uint8_t
//{
//	kOne = 0,
//	kNumParams
//};
//
//class App
//{
//public:
//	App()
//	{
//		
//	}
//
//	bool Initialize()
//	{
//		device_ = MakeHandle<D12Device>();
//
//		swap_chain_ = CheckedCast<D12Device*>(device_)->CreateSwapChian(nullptr);
//
//		CommandList* command_list = device_->GetCommandList(CommandListType::kDirect);
//
//		BufferDesc vertex_desc;
//		vertex_desc.type = BufferType::kVertex;
//		vertex_desc.byte = 10;
//		vertex_desc.initial_state = ResourceStates::kCopyDest;
//		vertex_desc.stride = 10;
//		vertex_desc.debug_name = "vertex_buffer";
//
//		BufferHandle vertex_buffer = device_->CreateBuffer(vertex_desc);
//		command_list->WriteBuffer(vertex_buffer, nullptr, 0, 0);
//
//		BufferDesc index_desc;
//		index_desc.type = BufferType::kIndex;
//		index_desc.byte = 20;
//		index_desc.initial_state = ResourceStates::kCopyDest;
//		index_desc.stride = 10;
//		index_desc.debug_name = "index_buffer";
//
//		BufferHandle index_buffer = device_->CreateBuffer(index_desc);
//		command_list->WriteBuffer(index_buffer, nullptr, 0, 0);
//
//		command_list->ExecuteCommandList();
//
//		std::vector<VertexAttributeDesc> attributes;
//		VertexAttributeDesc desc;
//
//		InputLayoutHandle input_layout = device_->CreateInputLayout(std::move(attributes));
//
//		BindingLayoutHandle binding_layout = MakeHandle<BindingLayout>(static_cast<uint8_t>(Params::kNumParams));
//
//		BindingParameter params1;
//		params1.InitAsConstants(0, 0);
//
//		binding_layout->Add(Params::kOne, params1);
//
//		GraphicsPipelineDesc pso_desc;
//		pso_desc.primitive_type = PrimitiveTopology::kTriangleList;
//		pso_desc.input_layout = input_layout;
//		pso_desc.binding_layout = binding_layout;
//
//		pso_ = device_->CreateGraphicsPipeline(pso_desc, swap_chain_->GetRenderTarget());
//
//		return true;
//	}
//
//	void Run()
//	{
//		
//	}
//
//	void Render()
//	{
//		CommandList* command_list = device_->GetCommandList(CommandListType::kDirect);
//
//		RenderTarget rt = swap_chain_->GetRenderTarget();
//		rt.AttacthTexture(AttachmentPoint::kDepthStencil, depth_stencil_texture_);
//
//		float clear_color[] = { 0.4f, 0.6f, 0.9f, 1.0f };
//		command_list->ClearTexture(rt.GetTexture(AttachmentPoint::kColor0), clear_color);
//
//		command_list->ClearDepthStencilTexture(
//			rt.GetTexture(AttachmentPoint::kDepthStencil),
//			ClearFlags::kClearFlagDepth | ClearFlags::kClearFlagStencil,
//			1.0f,
//			0);
//
//		command_list->SetRednerTarget(rt);
//
//		command_list->SetViewport(rt.GetViewport());
//		command_list->SetScissorRect({ 0,0,std::numeric_limits<uint32_t>::max(),std::numeric_limits<uint32_t>::max() });
//
//		command_list->ExecuteCommandList();
//
//		swap_chain_->Present();
//		
//	}
//private:
//	DeviceHandle device_;
//	SwapChainHandle swap_chain_;
//	TextureHandle depth_stencil_texture_;
//	
//	GraphicsPipelineHandle pso_;
//
//};
//
//int main()
//{
//	App app;
//	if(app.Initialize())
//	{
//		app.Run();
//	}
//}