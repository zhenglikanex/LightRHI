#include "root_signature.h"

#include "d12_device.h"

namespace light::rhi
{
	RootSignature::RootSignature(D12Device* device, size_t hash, BindingLayoutHandle binding_layout, bool allow_input_layout)
		: device_(device)
		, hash_(hash)
		, num_descriptors_per_table_{}
		, num_parameters_(0)
		, descriptor_table_bit_mask_(0)
		, sampler_table_bit_mask_(0)
	{

		if(binding_layout != nullptr)
		{
			num_parameters_ = binding_layout->Size();

			std::vector<D3D12_ROOT_PARAMETER> root_parameters(binding_layout->Size());

			std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> ranges;
			ranges.reserve(binding_layout->Size());

			for (uint32_t i = 0; i < binding_layout->Size(); ++i)
			{
				const auto& in = (*binding_layout)[i];
				auto& out = root_parameters[i];

				if (in.type == BindingParameterType::kDescriptorTable)
				{
					auto& descriptor_ranges = ranges.emplace_back(in.descriptor_table.num_descriptor_ranges);

					uint32_t offset = 0;
					for (uint32_t range_index = 0; range_index < in.descriptor_table.num_descriptor_ranges; ++range_index)
					{
						auto& out_range = descriptor_ranges[range_index];
						const auto& in_range = in.descriptor_table.descriptor_ranges[range_index];
						out_range.RangeType = ConvertDescriptorRangeType(in_range.range_type);
						out_range.NumDescriptors = in_range.num_descriptors;
						out_range.BaseShaderRegister = in_range.base_shader_register;
						out_range.RegisterSpace = in_range.register_space;
						out_range.OffsetInDescriptorsFromTableStart = offset;
						
						num_descriptors_per_table_[i] += out_range.NumDescriptors;
						offset += out_range.NumDescriptors;
					}

					out.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					out.ShaderVisibility = ConvertShaderVisibility(in.shader_visibility);
					out.DescriptorTable.NumDescriptorRanges = in.descriptor_table.num_descriptor_ranges;
					out.DescriptorTable.pDescriptorRanges = descriptor_ranges.data();
				

					if (descriptor_ranges.back().RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
					{
						sampler_table_bit_mask_ |= 1 << i;
					}
					else 
					{
						descriptor_table_bit_mask_ |= 1 << i;
					}
					
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
			if (allow_input_layout)
			{
				flag |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
			}

			//todo:RootSignature°æ±¾?
			//CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rs_desc{};
			CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(root_parameters.size(), root_parameters.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
			//rs_desc.Init_1_1(root_parameters.size(), root_parameters.data(), 0, nullptr, flag);
			D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized_rs.GetAddressOf(), error_blob.GetAddressOf());
			//ThrowIfFailed(D3D12SerializeVersionedRootSignature(&rs_desc, &serialized_rs, &error_blob));

			ThrowIfFailed(device_->GetNative()->CreateRootSignature(
				0,
				serialized_rs->GetBufferPointer(),
				serialized_rs->GetBufferSize(),
				IID_PPV_ARGS(&root_signature_)));
		}
		else
		{
			Handle<ID3DBlob> serialized_rs = nullptr;
			Handle<ID3DBlob> error_blob = nullptr;

			D3D12_ROOT_SIGNATURE_FLAGS flag = D3D12_ROOT_SIGNATURE_FLAG_NONE;
			if (allow_input_layout)
			{
				flag |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
			}

			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rs_desc{};
			rs_desc.Init_1_1(0, nullptr, 0, nullptr, flag);

			ThrowIfFailed(D3D12SerializeVersionedRootSignature(&rs_desc, &serialized_rs, &error_blob));

			ThrowIfFailed(device_->GetNative()->CreateRootSignature(
				0,
				serialized_rs->GetBufferPointer(),
				serialized_rs->GetBufferSize(),
				IID_PPV_ARGS(&root_signature_)));
		}
	}

	RootSignature::~RootSignature()
	{
		// ÊÍ·Å»º´æ
		device_->ReleaseRootSignature(this);
	}
}
