#pragma once

#include <d3d12.h>
#include <memory>
#include <deque>
#include <vector>

#include "rhi/resource.h"

namespace light::rhi
{
	class D12Device;

	class UploadBuffer
	{
	public:
		// 分配的cpu和gpu地址
		struct Allocation
		{
			void*						cpu = nullptr;
			D3D12_GPU_VIRTUAL_ADDRESS	gpu = 0;
			ID3D12Resource*				upload_resource = nullptr;
			size_t						offset = 0;
		};

		explicit UploadBuffer(D12Device* device, size_t page_size = 2 * 1024 * 1024);

		UploadBuffer(const UploadBuffer&) = delete;
		UploadBuffer& operator=(const UploadBuffer&) = delete;

		UploadBuffer(UploadBuffer&&) = default;
		UploadBuffer& operator=(UploadBuffer&&) = default;

		size_t GetPageSize() const { return page_size_; }

		Allocation Allocate(size_t bytes, size_t alignment);

		void Rest();

	private:
		class Page
		{
		public:
			Page(D12Device* device, size_t page_size);
			~Page();

			// 检查是否还有可分配的空间
			bool HasSpace(size_t bytes, size_t alignment) const;

			Allocation Allocate(size_t bytes, size_t alignment);

			void Reset();
		private:
			D12Device* device_;

			Handle<ID3D12Resource> resource_;

			void* cpu_;
			D3D12_GPU_VIRTUAL_ADDRESS gpu_;

			size_t page_size_;

			size_t offset_;
		};

		std::shared_ptr<Page> RequestPage();

		Allocation AllocateLarge(size_t bytes, size_t alignment);

		D12Device* device_;

		size_t page_size_;

		// 当前分配出去的page
		std::deque<std::shared_ptr<Page>> page_pool_;

		// 未分配可以复用的page
		std::deque<std::shared_ptr<Page>> available_pages_;

		std::shared_ptr<Page> current_page_;

		std::vector<Handle<ID3D12Resource>> large_upload_resources_;
	};
}