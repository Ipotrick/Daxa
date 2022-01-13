#pragma once

#include "../DaxaCore.hpp"

#include <memory>
#include <unordered_map>
#include <optional>

#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>

#include "Sampler.hpp"

namespace daxa {
	namespace gpu {
		struct Image2dCreateInfo {
			u32 width = 1;
			u32 height = 1;
			VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
			VkImageUsageFlags imageUsage = VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			VkImageAspectFlags imageAspekt = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
			VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
			VkMemoryPropertyFlags memoryPropertyFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			std::optional<SamplerHandle> sampler = {};		// optional sampler
			char const* debugName = {};
		};

		class Image {
		public:
			Image() 							= default;
			Image(Image const&) 				= delete;
			Image& operator=(Image const&) 		= delete;
			Image(Image&&) noexcept 			= delete;
			Image& operator=(Image&&) noexcept 	= delete;
			~Image();

			VkImage getVkImage() const { return image; }
			VkImageView getVkView() const { return view; }
			VkImageTiling getVkTiling() const { return tiling; };
			VkImageUsageFlags getVkUsageFlags() const { return usageFlags; };
			VkFormat getVkViewFormat() const { return viewFormat; }
			VkImageType getVkImageType() const { return type; }
			VkExtent3D getVkExtent() const { return extent; }
			VkImageAspectFlags getVkAspect() const { return aspect; }
			u32 getVkMipmapLevels() const { return mipmapLevels; }
			u32 getVkArrayLayers() const { return arrayLayers; }
			bool hasSampler() const { return sampler.valid(); }
			SamplerHandle getSampler() const { return sampler; }
		private:
			friend class Device;
			friend class ImageHandle;
			friend class Swapchain;

			static void construct2dImage(VkDevice device, VmaAllocator allocator, u32 queueFamilyIndex, Image2dCreateInfo const& ci, Image& dst);

			VkDevice device = VK_NULL_HANDLE;
			VmaAllocator allocator = VK_NULL_HANDLE;
			VmaAllocation allocation = VK_NULL_HANDLE;
			VkImage image;
			VkImageView view;
			VkImageTiling tiling;
			VkImageUsageFlags usageFlags;
			VkFormat viewFormat;
			VkImageType type;
			VkExtent3D extent;
			VkImageAspectFlags aspect;
			u32 mipmapLevels;
			u32 arrayLayers;
			SamplerHandle sampler = {};	// this is an optional field
		};

		class ImageHandle {
		public:
			ImageHandle() = default;
			ImageHandle(std::shared_ptr<Image> image);

			Image const& operator*() const { return *image; }
			Image& operator*() { return *image; }
			Image const* operator->() const { return image.get(); }
			Image* operator->() { return image.get(); }

			operator bool() const { return image.operator bool(); }
			bool operator!() const { return !image; }
			bool valid() const { return *this; }

			size_t getRefCount() const { return image.use_count(); }
			Image* get() { return image.get(); }
			Image const* get() const { return image.get(); }
		private:
			friend class Device;
			friend class Swapchain;

			std::shared_ptr<Image> image = {};
		};
	}
}