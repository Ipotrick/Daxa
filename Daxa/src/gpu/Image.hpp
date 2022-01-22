#pragma once

#include "../DaxaCore.hpp"

#include <memory>
#include <unordered_map>
#include <optional>

#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>

#include "Handle.hpp"
#include "Sampler.hpp"
#include "Graveyard.hpp"

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

		class Image : public GraveyardRessource {
		public:
			Image() 							= default;
			Image(Image const&) 				= delete;
			Image& operator=(Image const&) 		= delete;
			Image(Image&&) noexcept 			= delete;
			Image& operator=(Image&&) noexcept 	= delete;
			virtual ~Image();

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

			std::string const& getDebugName() const { return debugName; }
		private:
			friend class Device;
			friend class ImageHandle;
			friend class Swapchain;
			friend struct ImageStaticFunctionOverride;

			static void construct2dImage(VkDevice device, Graveyard* graveyard, VmaAllocator allocator, u32 queueFamilyIndex, Image2dCreateInfo const& ci, Image& dst);

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
			std::string debugName = {};
			Graveyard* graveyard = {};
		};

        struct ImageStaticFunctionOverride {
            static void cleanup(std::shared_ptr<Image>& value) {
				if (value && value.use_count() == 1) {
					std::unique_lock lock(value->graveyard->mtx);
					for (auto& zombieList : value->graveyard->activeZombieLists) {
						zombieList->zombies.push_back(value);
					}
				}
			}
        };

		class ImageHandle : public SharedHandle<Image, ImageStaticFunctionOverride>{};
	}
}