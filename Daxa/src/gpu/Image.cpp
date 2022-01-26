#include "Image.hpp"
#include <cassert>
#include "Instance.hpp"

namespace daxa {
	namespace gpu {

		Image::~Image() {
			if (deviceBackend->device.device && view) {
				vkDestroyImageView(deviceBackend->device.device, view, nullptr);
				view = VK_NULL_HANDLE;
			}
			if (deviceBackend->allocator && image && allocation) {
				vmaDestroyImage(deviceBackend->allocator, image, allocation);
				image = VK_NULL_HANDLE;
				allocation = VK_NULL_HANDLE;
			}
			deviceBackend = {};
		}

		void Image::construct2dImage(std::shared_ptr<DeviceBackend> deviceBackend, Image2dCreateInfo const& ci, Image& ret) {
			VkImageCreateInfo ici{
				.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.pNext = nullptr,
				.imageType = VkImageType::VK_IMAGE_TYPE_2D,
				.format = ci.format,
				.extent = VkExtent3D{ ci.width, ci.height, 1 },
				.mipLevels = 1,
				.arrayLayers = 1,
				.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
				.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL,
				.usage = ci.imageUsage,
				.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
				.queueFamilyIndexCount = 0,
				.pQueueFamilyIndices = nullptr,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			};

			VmaAllocationCreateInfo aci{ 
				.usage = ci.memoryUsage,
				.requiredFlags = (VkMemoryPropertyFlags)ci.memoryPropertyFlags,
			};
			

			DAXA_CHECK_VK_RESULT_M(vmaCreateImage(deviceBackend->allocator, (VkImageCreateInfo*)&ici, &aci, (VkImage*)&ret.image, &ret.allocation, nullptr), "failed to create image");

			if (instance->pfnSetDebugUtilsObjectNameEXT != nullptr && ci.debugName != nullptr) {
				ret.debugName = ci.debugName;

				VkDebugUtilsObjectNameInfoEXT imageNameInfo{
					.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
					.pNext = NULL,
					.objectType = VK_OBJECT_TYPE_IMAGE,
					.objectHandle = (uint64_t)ret.image,
					.pObjectName = ci.debugName,
				};
				instance->pfnSetDebugUtilsObjectNameEXT(deviceBackend->device.device, &imageNameInfo);
			}

			VkImageViewCreateInfo ivci{
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext = nullptr,
				.image = ret.image,
				.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D,
				.format = ci.format,
				.subresourceRange = VkImageSubresourceRange{
					.aspectMask = ci.imageAspekt,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			};

			DAXA_CHECK_VK_RESULT_M(vkCreateImageView(deviceBackend->device.device, &ivci, nullptr, &ret.view), "failed to create image view");

			if (instance->pfnSetDebugUtilsObjectNameEXT != nullptr && ci.debugName != nullptr) {
				VkDebugUtilsObjectNameInfoEXT imageNameInfo {
					.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
					.pNext = NULL,
					.objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
					.objectHandle = (uint64_t)ret.view,
					.pObjectName = ci.debugName,
				};
				instance->pfnSetDebugUtilsObjectNameEXT(deviceBackend->device.device, &imageNameInfo);
			}

			ret.deviceBackend = std::move(deviceBackend);
			ret.tiling = (VkImageTiling)ici.tiling;
			ret.usageFlags = (VkImageUsageFlags)ici.usage;
			ret.viewFormat = (VkFormat)ivci.format;
			ret.type = VK_IMAGE_TYPE_2D;
			ret.extent = ici.extent;
			ret.aspect = (VkImageAspectFlags)ci.imageAspekt;
			ret.arrayLayers = 1;
			ret.mipmapLevels = 1;

			if (ci.sampler.has_value()) {
				ret.sampler = *ci.sampler;
			}
		}

		
		ImageView::ImageView(std::shared_ptr<DeviceBackend> deviceBackend, ImageViewCreateInfo const& ci) 
			: deviceBackend{ deviceBackend }
			, flags{ ci.flags }
			, image{ ci.image }
			, viewType{ ci.viewType }
			, format{ ci.format }
			, components{ ci.components }
			, subresourceRange{ ci.subresourceRange }
		{
			VkImageViewCreateInfo ivci{
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext = nullptr,
    			.flags = ci.flags,
				.image = ci.image->getVkImage(),
    			.viewType = ci.viewType,
    			.format = ci.format,
    			.components = ci.components,
    			.subresourceRange = ci.subresourceRange,
			};

			DAXA_CHECK_VK_RESULT_M(vkCreateImageView(deviceBackend->device.device, &ivci, nullptr, &view), "failed to create image view");

			if (instance->pfnSetDebugUtilsObjectNameEXT != nullptr && ci.debugName != nullptr) {
				VkDebugUtilsObjectNameInfoEXT imageNameInfo {
					.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
					.pNext = NULL,
					.objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
					.objectHandle = (uint64_t)view,
					.pObjectName = ci.debugName,
				};
				instance->pfnSetDebugUtilsObjectNameEXT(deviceBackend->device.device, &imageNameInfo);
				debugName = ci.debugName;
			}
		}

		ImageView::~ImageView() {
			if (deviceBackend->device.device && view) {
				vkDestroyImageView(deviceBackend->device.device, view, nullptr);
				view = VK_NULL_HANDLE;
			}
		}
	}
}
