#include "ImageViewBackend.hpp"

namespace daxa {
    void ImageViewBackend::create(VkDevice device, ImageViewInfo const& info, VkImageView preCreatedView) {
        this->info = info;
        if (this->info.subresourceRange.aspectMask == VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM) {
			this->info.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = image->getMipLevels(),
				.baseArrayLayer = 0,
				.layerCount = image->getArrayLayers(),
			};
		}

		if ( preCreatedView != VK_NULL_HANDLE ) {
			this->imageView = preCreatedView;
		}
		else {
			VkImageViewCreateInfo ivci{
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext = nullptr,
				.flags = this->info.flags,
				.image = this->info.image->getVkImage(),
				.viewType = this->info.viewType,
				.format = this->info.format,
				.components = this->info.components,
				.subresourceRange = this->info.subresourceRange,
			};

			DAXA_CHECK_VK_RESULT_M(vkCreateImageView(device, &ivci, nullptr, &this->imageView), "failed to create image view");
		}

		//if (instance->pfnSetDebugUtilsObjectNameEXT != nullptr && this->info.debugName != nullptr) {
		//	VkDebugUtilsObjectNameInfoEXT imageNameInfo {
		//		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
		//		.pNext = NULL,
		//		.objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
		//		.objectHandle = (uint64_t)this->imageView,
		//		.pObjectName = this->info.debugName,
		//	};
		//	instance->pfnSetDebugUtilsObjectNameEXT(device, &imageNameInfo);
		//}
    }

    void ImageViewBackend::destroy(VkDevice device) {
		vkDestroyImageView(device, this->imageView, nullptr);
        this->image = {};
    }
}