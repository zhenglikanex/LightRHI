#include "upload_buffer.h"

#include "d12_device.h"

#include "d3dx12.h"

namespace light::rhi
{
	UploadBuffer::UploadBuffer(D12Device* device, size_t page_size)
		: device_(device)
		, page_size_(page_size)
	{

	}

	UploadBuffer::Allocation UploadBuffer::Allocate(size_t bytes, size_t alignment)
	{
		if(bytes > page_size_)
		{
			return AllocateLarge(bytes, alignment);
		}

		if(!current_page_ || current_page_->HasSpace(bytes,alignment))
		{
			current_page_ = RequestPage();
		}

		return current_page_->Allocate(bytes, alignment);
	}

	void UploadBuffer::Rest()
	{
		available_pages_ = std::move(page_pool_);

		current_page_ = nullptr;

		for(auto& page : available_pages_)
		{
			page->Reset();
		}

		for(auto& resource : large_upload_resources_)
		{
			resource->Unmap(0, nullptr);
		}

		large_upload_resources_.clear();
	}

	std::shared_ptr<UploadBuffer::Page> UploadBuffer::RequestPage()
	{
		if(!available_pages_.empty())
		{
			auto page = available_pages_.front();
			available_pages_.pop_front();
			return page;
		}

		auto page = std::make_shared<Page>(device_, page_size_);
		if(page)
		{
			page_pool_.push_back(page);
		}

		return page;
	}

	UploadBuffer::Allocation UploadBuffer::AllocateLarge(size_t bytes, size_t alignment)
	{
		size_t align_bytes = Align(bytes, alignment);

		Handle<ID3D12Resource> upload_resource;

		CD3DX12_HEAP_PROPERTIES heap_properties(D3D12_HEAP_TYPE_UPLOAD);

		CD3DX12_RESOURCE_DESC buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(align_bytes);

		device_->GetNative()->CreateCommittedResource(
			&heap_properties,
			D3D12_HEAP_FLAG_NONE,
			&buffer_desc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload_resource));

		large_upload_resources_.push_back(upload_resource);

		Allocation result;
		
		upload_resource->Map(0, nullptr, &result.cpu);
		result.gpu = upload_resource->GetGPUVirtualAddress();
		result.upload_resource = upload_resource;

		return result;
	}

	UploadBuffer::Page::Page(D12Device* device, size_t page_size)
		: device_(device)
		, page_size_(page_size)
		, offset_(0)
	{
		CD3DX12_HEAP_PROPERTIES heap_properties(D3D12_HEAP_TYPE_UPLOAD);

		CD3DX12_RESOURCE_DESC buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(page_size);

		device_->GetNative()->CreateCommittedResource(
			&heap_properties,
			D3D12_HEAP_FLAG_NONE,
			&buffer_desc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource_));

		resource_->SetName(L"Upload Buffer (Page)");

		gpu_ = resource_->GetGPUVirtualAddress();
		resource_->Map(0, nullptr, &cpu_);
	}

	UploadBuffer::Page::~Page()
	{
		resource_->Unmap(0, nullptr);
		cpu_ = nullptr;
		gpu_ = 0;
	}

	bool UploadBuffer::Page::HasSpace(size_t bytes, size_t alignment) const
	{
		size_t align_bytes = Align(bytes, alignment);
		size_t align_offset = Align(offset_, alignment);

		return align_offset + align_bytes < page_size_;
	}

	UploadBuffer::Allocation UploadBuffer::Page::Allocate(size_t bytes, size_t alignment)
	{
		if(!HasSpace(bytes,alignment))
		{
			throw std::bad_alloc();
		}

		offset_ = Align(offset_, alignment);

		Allocation result{ static_cast<char*>(cpu_) + offset_,gpu_ + offset_,resource_,offset_ };

		offset_ += Align(bytes,alignment);

		return result;
	}

	void UploadBuffer::Page::Reset()
	{
		offset_ = 0;
	}

	
}
