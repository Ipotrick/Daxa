#pragma once

#include <memory>

#include <vulkan/vulkan.h>

#include "../dependencies/vk_mem_alloc.hpp"

#include "../../DaxaCore.hpp"

namespace daxa {
	namespace gpu {
		struct Image2dCreateInfo {
			u32 width;
			u32 height;
			VkFormat format;
			VkImageUsageFlags imageUsage = VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			VkImageAspectFlags imageAspekt = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
			VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
			VkMemoryPropertyFlags memoryPropertyFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
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
			VkImageAspectFlags getVkAspect() const { return aspect; }
			VkImage getVkImage() const { return image; }
			VkImageView getVkView() const { return view; }
		//private:
			friend class Device;
			friend class ImageHandle;
			friend class RenderWindow;

			static Image create2dImage(VkDevice device, VmaAllocator allocator, Image2dCreateInfo ci);

			VkDevice device;
			VmaAllocator allocator;
			VmaAllocation allocation;
			VkImageTiling tiling;
			VkImageUsageFlags usageFlags;
			VkImageLayout layout;
			VkFormat viewFormat;
			VkImageType type;
			VkExtent3D extent;
			VkImageAspectFlags aspect;
			VkImage image;
			VkImageView view;
		};

		class ImageHandle {
		public:
			ImageHandle() = default;
			Image const& operator*() const { return *image; }
			Image& operator*() { return *image; }
			Image const* operator->() const { return image.get(); }
			Image* operator->() { return image.get(); }
		private:
			friend class Device;
			friend class RenderWindow;

			ImageHandle(std::shared_ptr<Image> image);

			std::shared_ptr<Image> image;
		};
	}
}