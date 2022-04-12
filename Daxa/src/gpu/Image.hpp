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
	struct ImageCreateInfo {
		VkImageCreateFlags      flags 		= {};
		VkImageType             imageType 	= VK_IMAGE_TYPE_2D;
		VkFormat                format 		= VK_FORMAT_R8G8B8A8_SRGB;
		VkExtent3D              extent 		= {};
		uint32_t                mipLevels 	= 1;
		uint32_t                arrayLayers = 1;
		VkSampleCountFlagBits   samples 	= VK_SAMPLE_COUNT_1_BIT;
		VkImageTiling           tiling 		= VK_IMAGE_TILING_OPTIMAL;
		VkImageUsageFlags       usage 		= {};
		VmaMemoryUsage 			memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
		char const*  			debugName 	= {};
	};

	class Image {
	public:
		Image(std::shared_ptr<DeviceBackend> deviceBackend, ImageCreateInfo const& ci);
		Image() 							= default;
		Image(Image const&) 				= delete;
		Image& operator=(Image const&) 		= delete;
		Image(Image&&) noexcept 			= delete;
		Image& operator=(Image&&) noexcept 	= delete;
		~Image();

		VkImageCreateFlags getVkImageCreateFlags() const { return flags; }
		VkImageType getVkImageType() const { return imageType; }
		VkFormat getVkFormat() const { return format; }
		VkExtent3D getVkExtent3D() const { return extent; }
		u32 getMipLevels() const { return mipLevels; }
		u32 getArrayLayers() const { return arrayLayers; }
		VkSampleCountFlagBits getVkSampleCountFlagBits() const { return samples; }
		VkImageTiling getVkImageTiling() const { return tiling; }
		VkImageUsageFlags getVkImageUsageFlags() const { return usage; }
		VmaMemoryUsage getVmaMemoryUsage() const { return memoryUsage; }
		VkImage getVkImage() const { return image; }
		VmaAllocation getVmaAllocation() const { return allocation; }
		std::string const& getDebugName() const { return debugName; }
	private:
		friend class Device;
		friend class Swapchain;
		friend struct ImageStaticFunctionOverride;

		std::shared_ptr<DeviceBackend> 	deviceBackend 	= {};
		VkImageCreateFlags      		flags 			= {};
		VkImageType             		imageType 		= {};
		VkFormat                		format 			= {};
		VkExtent3D              		extent 			= {};
		uint32_t                		mipLevels 		= {};
		uint32_t                		arrayLayers 	= {};
		VkSampleCountFlagBits   		samples 		= {};
		VkImageTiling           		tiling 			= {};
		VkImageUsageFlags       		usage 			= {};
		VmaMemoryUsage 					memoryUsage 	= {};
		VkImage 						image 			= {};
		VmaAllocation 					allocation 		= {};
		std::string  					debugName 		= {};
	};

	class ImageHandle : public SharedHandle<Image>{};

	struct ImageViewCreateInfo {
		VkImageViewCreateFlags     	flags				= {};
		ImageHandle					image		 		= {};
		VkImageViewType            	viewType			= VK_IMAGE_VIEW_TYPE_2D;
		VkFormat                   	format				= {};
		VkComponentMapping         	components			= {};
		VkImageSubresourceRange    	subresourceRange	= {
			.aspectMask = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM,
			.baseMipLevel = 0,
			.levelCount = 0,
			.baseArrayLayer = 0,
			.layerCount = 0,
		};
		SamplerHandle				defaultSampler		= {};
		char const* 				debugName			= {};
	};

	class ImageView : public GraveyardRessource {
	public:
		ImageView(std::shared_ptr<DeviceBackend> deviceBackend, ImageViewCreateInfo const& ci, VkImageView view = VK_NULL_HANDLE);
		ImageView() 								= default;
		ImageView(ImageView const&) 				= delete;
		ImageView& operator=(ImageView const&) 		= delete;
		ImageView(ImageView&&) noexcept 			= delete;
		ImageView& operator=(ImageView&&) noexcept 	= delete;
		virtual ~ImageView();

		VkImageViewCreateFlags getVkImageViewCreateFlags() const { return flags; }
		ImageHandle const& getImageHandle() const { return image; }
		VkImageViewType getVkImageViewType() const { return viewType; }
		VkFormat getVkFormat() const { return format; }
		VkComponentMapping getVkComponentMapping() const { return components; }
		VkImageSubresourceRange getVkImageSubresourceRange() const { return subresourceRange; }
		VkImageView getVkImageView() const { return view; }
		SamplerHandle const& getSampler() const { return defaultSampler; }
		u16 getDescriptorIndex() const {
			return descriptorIndex;
		}
		std::string const& getDebugName() const { return debugName; }
	private:
		friend class Device;
		friend class Swapchain;
		friend struct ImageViewStaticFunctionOverride;

		std::shared_ptr<DeviceBackend> 	deviceBackend 		= {};
		VkImageViewCreateFlags     		flags				= {};
		ImageHandle						image		 		= {};
		VkImageViewType            		viewType			= {};
		VkFormat                   		format				= {};
		VkComponentMapping         		components			= {};
		VkImageSubresourceRange    		subresourceRange	= {};
		VkImageView						view				= {};
		SamplerHandle 					defaultSampler 		= {};
		u16 							descriptorIndex 	= {};
		std::string 					debugName 			= {};
	};

	struct ImageViewStaticFunctionOverride {
		static void cleanup(std::shared_ptr<ImageView>& value);
	};

	class ImageViewHandle : public SharedHandle<ImageView, ImageViewStaticFunctionOverride>{};
}