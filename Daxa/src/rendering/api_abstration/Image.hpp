#pragma once

#include <memory>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "../dependencies/vk_mem_alloc.hpp"
#include "../dependencies/vulkanhelper.hpp"

#include "../../DaxaCore.hpp"

namespace daxa {
	namespace gpu {
		struct Image2dCreateInfo {
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
			Image(Image&&) noexcept;
			Image& operator=(Image&&) noexcept;

			~Image();

			vk::ImageCreateInfo getVkCreateInfo() const { return createInfo; }
			vk::ImageViewCreateInfo getVkViewCreateInfo() const { return viewCreateInfo; }
			vk::Image getVkImage() const { return image; }
			vk::ImageView getVkView() const { return *view; }
		private:
			static Image create2dImage(vk::Device device, VmaAllocator allocator, Image2dCreateInfo ci);
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
			ImageHandle(std::shared_ptr<Image> image);
			friend class Device;
			std::shared_ptr<Image> image;
		};
	}
}