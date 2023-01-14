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

		ID3D12RootSignature* GetNative() { return root_signature_; }

	private:
		D12Device* device_;
		Handle<ID3D12RootSignature> root_signature_;
		size_t hash_;
	};

	using RootSignatureHandle = Handle<RootSignature>;
}