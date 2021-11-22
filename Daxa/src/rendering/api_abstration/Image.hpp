#pragma once

#include <memory>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "../dependencies/vk_mem_alloc.hpp"
#include "../dependencies/vulkanhelper.hpp"

#include "../../DaxaCore.hpp"

namespace gpu {
	struct ImageCreateInfo {
		u32 width;
		u32 height;
		vk::Format format;
		vk::ImageUsageFlags imageUsage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
		vk::ImageAspectFlags imageAspekt = vk::ImageAspectFlagBits::eColor;
		VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
		vk::MemoryPropertyFlags memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
	};

	class Image {
	public: 
		Image() = default;
		Image(Image&&) = delete;
		Image& operator=(Image&&) = delete;

		~Image();
	private:
		friend class Device;
		friend class ImageHandle;
		VmaAllocator allocator;
		VmaAllocation allocation;
		vk::ImageCreateInfo createInfo;
		vk::ImageViewCreateInfo viewCreateInfo;
		vk::Image image;
		vk::UniqueImageView view;
	};

	class ImageHandle {
	public:
		Image& operator*() { return *image; }
		Image* operator->() { return image.get(); }
	private:
		friend class Device;
		std::shared_ptr<Image> image;
	};
}
