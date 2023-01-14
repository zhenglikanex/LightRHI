#include "root_signature.h"

#include "d12_device.h"

namespace light::rhi
{
	RootSignature::RootSignature(D12Device* device,size_t hash,BindingLayoutHandle binding_layout, bool allow_input_layout)
		: device_(device)
		, hash_(hash)
	{
		const auto& parameters = binding_layout->GetParameters();

		std::vector<D3D12_ROOT_PARAMETER1> root_parameters(parameters.size());

		std::vector<std::vector<D3D12_DESCRIPTOR_RANGE1>> ranges;
		ranges.reserve(parameters.size());

		for (uint32_t i = 0; i < parameters.size(); ++i)
		{
			const auto& in = parameters[i];
			auto& out = root_parameters[i];

			if (in.type == BindingParameterType::kDescriptorTable)
			{
				auto& descriptor_ranges = ranges.emplace_back(in.descriptor_table.num_descriptor_ranges);

				for (uint32_t range_index = 0; range_index < in.descriptor_table.num_descriptor_ranges; ++range_index)
				{
					auto& range = descriptor_ranges[i];
					range.RangeType = ConvertDescriptorRangeType(in.descriptor_table.descriptor_ranges[i].range_type);
					range.NumDescriptors = in.descriptor_table.descriptor_ranges[i].num_descriptors;
					range.BaseShaderRegister = in.descriptor_table.descriptor_ranges[i].base_shader_register;
					range.RegisterSpace = in.descriptor_table.descriptor_ranges[i].register_space;
					range.OffsetInDescriptorsFromTableStart = in.descriptor_table.descriptor_ranges[i].offset_in_descriptors_from_table_start;
				}

				out.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				out.ShaderVisibility = ConvertShaderVisibility(in.shader_visibility);
				out.DescriptorTable.NumDescriptorRanges = in.descriptor_table.num_descriptor_ranges;
				out.DescriptorTable.pDescriptorRanges = descriptor_ranges.data();
			}
			else if (in.type == BindingParameterType::kConstants)
			{
				out.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
				out.ShaderVisibility = ConvertShaderVisibility(in.shader_visibility);
				out.Constants.Num32BitValues = in.constants.num32_bit_values;
				out.Constants.ShaderRegister = in.constants.shader_register;
				out.Constants.RegisterSpace = in.constants.register_space;
			}
			else if (in.type == BindingParameterType::kConstantBufferView)
			{
				out.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
				out.ShaderVisibility = ConvertShaderVisibility(in.shader_visibility);
				out.Descriptor.ShaderRegister = in.descriptor.shader_register;
				out.Descriptor.RegisterSpace = in.descriptor.register_space;
			}
			else if (in.type == BindingParameterType::kShaderResourceView)
			{
				out.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
				out.ShaderVisibility = ConvertShaderVisibility(in.shader_visibility);
				out.Descriptor.ShaderRegister = in.descriptor.shader_register;
				out.Descriptor.RegisterSpace = in.descriptor.register_space;
			}
			else if (in.type == BindingParameterType::kUnorderAccessView)
			{
				out.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
				out.ShaderVisibility = ConvertShaderVisibility(in.shader_visibility);
				out.Descriptor.ShaderRegister = in.descriptor.shader_register;
				out.Descriptor.RegisterSpace = in.descriptor.register_space;
			}
		}

		Handle<ID3DBlob> serialized_rs = nullptr;
		Handle<ID3DBlob> error_blob = nullptr;

		D3D12_ROOT_SIGNATURE_FLAGS flag = D3D12_ROOT_SIGNATURE_FLAG_NONE;
		if(allow_input_layout)
		{
			flag |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		}

		//todo:
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rs_desc{};
		rs_desc.Init_1_1(root_parameters.size(), root_parameters.data(), 0, nullptr, flag);

		ThrowIfFailed(D3D12SerializeVersionedRootSignature(&rs_desc, &serialized_rs, &error_blob));

		ThrowIfFailed(device_->GetNative()->CreateRootSignature(
			0,
			serialized_rs->GetBufferPointer(), 
			serialized_rs->GetBufferSize(), 
			IID_PPV_ARGS(&root_signature_)));
	}

	RootSignature::~RootSignature()
	{
		// ÊÍ·Å»º´æ
		device_->ReleaseRootSignature(this);
	}
}
