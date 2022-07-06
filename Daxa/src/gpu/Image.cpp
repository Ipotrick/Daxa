#include "Image.hpp"
#include <cassert>
#include "Instance.hpp"
#include "backend/DeviceBackend.hpp"

namespace daxa {
    Image::Image(std::shared_ptr<DeviceBackend> deviceBackend, ImageCreateInfo const& ci)
		: deviceBackend{ std::move(deviceBackend) }
		, flags{ ci.flags }
		, imageType{ ci.imageType }
		, format{ ci.format }
		, extent{ ci.extent }
		, mipLevels{ ci.mipLevels }
		, arrayLayers{ ci.arrayLayers }
		, samples{ ci.samples }
		, tiling{ ci.tiling }
		, usage{ ci.usage }
		, memoryUsage{ ci.memoryUsage }
	{
		VkImageCreateInfo ici{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = ci.flags,
			.imageType = ci.imageType,
			.format = ci.format,
			.extent = ci.extent,
			.mipLevels = ci.mipLevels,
			.arrayLayers = ci.arrayLayers,
			.samples = ci.samples,
			.tiling = ci.tiling,
			.usage = ci.usage,
			.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		};

		VmaAllocationCreateInfo aci{ 
			.usage = ci.memoryUsage,
		};
		DAXA_CHECK_VK_RESULT_M(vmaCreateImage(this->deviceBackend->allocator, (VkImageCreateInfo*)&ici, &aci, (VkImage*)&image, &allocation, nullptr), "failed to create image");

		if (instance->pfnSetDebugUtilsObjectNameEXT != nullptr && ci.debugName != nullptr) {
			debugName = ci.debugName;
			VkDebugUtilsObjectNameInfoEXT imageNameInfo{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = nullptr,
				.objectType = VK_OBJECT_TYPE_IMAGE,
				.objectHandle = (uint64_t)image,
				.pObjectName = ci.debugName,
			};
			instance->pfnSetDebugUtilsObjectNameEXT(this->deviceBackend->device.device, &imageNameInfo);
		}
	}

	Image::~Image() {
		if (deviceBackend && image && allocation) {
			vmaDestroyImage(deviceBackend->allocator, image, allocation);
			image = VK_NULL_HANDLE;
			allocation = VK_NULL_HANDLE;
		}
	}
}