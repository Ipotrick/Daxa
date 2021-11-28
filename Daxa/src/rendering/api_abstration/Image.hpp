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

			VkImageTiling getTiling() const { return tiling; };
			VkImageUsageFlags getUsageFlags() const { return usageFlags; };
			VkImageLayout getLayout() const { return layout; }
			VkFormat getViewFormat() const { return viewFormat; }
			VkImageType getImageType() const { return type; }
			VkExtent3D getExtent() const { return extent; }
			vk::Image getVkImage() const { return image; }
			vk::ImageView getVkView() const { return view; }
		private:
			friend class Device;
			friend class ImageHandle;
			friend class RenderWindow;

			static Image create2dImage(vk::Device device, VmaAllocator allocator, Image2dCreateInfo ci);

			VkDevice device;
			VmaAllocator allocator;
			VmaAllocation allocation;
			VkImageTiling tiling;
			VkImageUsageFlags usageFlags;
			VkImageLayout layout;
			VkFormat viewFormat;
			VkImageType type;
			VkExtent3D extent;
			vk::Image image;
			vk::ImageView view;
		};

		class ImageHandle {
		public:
			ImageHandle() = default;
			Image& operator*() { return *image; }
			Image* operator->() { return image.get(); }
		private:
			friend class Device;
			friend class RenderWindow;

			ImageHandle(std::shared_ptr<Image> image);

			std::shared_ptr<Image> image;
		};
	}
}