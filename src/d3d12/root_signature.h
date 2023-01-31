#pragma once

#include <d3d12.h>

#include "rhi/resource.h"
#include "rhi/binding_layout.h"

namespace light::rhi
{
	class D12Device;

	class RootSignature final : public Resource
	{
	public:
		RootSignature(D12Device* device, size_t hash,BindingLayoutHandle binding_layout,bool allow_input_layout);
		~RootSignature() override;

		size_t GetHash() const { return hash_; }

		uint32_t GetNumParameters() const { return num_parameters_; }

		ID3D12RootSignature* GetNative() { return root_signature_; }

		uint32_t GetNumDescriptors(uint32_t index) const
		{
			return num_descriptors_per_table_[index];
		}

		uint32_t GetDescriptorTableBitMask() const { return descriptor_table_bit_mask_; }

		uint32_t GetSamplerTableBitMask() const { return sampler_table_bit_mask_; }

	private:
		D12Device* device_;

		Handle<ID3D12RootSignature> root_signature_;

		size_t hash_;

		// 记录每个描述符表的描述符个数
		// 32取每个root signature的最大描述符表个数
		uint32_t num_descriptors_per_table_[32];

		uint32_t num_parameters_;

		uint32_t descriptor_table_bit_mask_;

		uint32_t sampler_table_bit_mask_;
	};

	using RootSignatureHandle = Handle<RootSignature>;
}