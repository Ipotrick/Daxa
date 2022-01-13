#include "Image.hpp"
#include <cassert>
#include "Instance.hpp"

namespace daxa {
	namespace gpu {

		Image::~Image() {
			if (device && view) {
				vkDestroyImageView(device, view, nullptr);
				device = VK_NULL_HANDLE;
				view = VK_NULL_HANDLE;
			}
			if (allocator && image && allocation) {
				vmaDestroyImage(allocator, image, allocation);
				allocator = VK_NULL_HANDLE;
				image = VK_NULL_HANDLE;
				allocation = VK_NULL_HANDLE;
			}
		}

		void Image::construct2dImage(VkDevice device, VmaAllocator allocator, u32 queueFamilyIndex, Image2dCreateInfo const& ci, Image& ret) {

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
				.queueFamilyIndexCount = 1,
				.pQueueFamilyIndices = &queueFamilyIndex,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			};

			VmaAllocationCreateInfo aci{ 
				.usage = ci.memoryUsage,
				.requiredFlags = (VkMemoryPropertyFlags)ci.memoryPropertyFlags,
			};
			

			auto err = vmaCreateImage(allocator, (VkImageCreateInfo*)&ici, &aci, (VkImage*)&ret.image, &ret.allocation, nullptr);
			DAXA_ASSERT_M(err == VK_SUCCESS, "could not create image");

			if (ci.debugName) {
				const VkDebugUtilsObjectNameInfoEXT imageNameInfo =
				{
					VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT, // sType
					NULL,                                               // pNext
					VK_OBJECT_TYPE_IMAGE,                               // objectType
					(uint64_t)ret.image,                                // objectHandle
					ci.debugName,                            			// pObjectName
				};
				instance->pfnSetDebugUtilsObjectNameEXT(device, &imageNameInfo);
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

			vkCreateImageView(device, &ivci, nullptr, &ret.view);

			if (ci.debugName) {
				const VkDebugUtilsObjectNameInfoEXT imageNameInfo =
				{
					VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT, // sType
					NULL,                                               // pNext
					VK_OBJECT_TYPE_IMAGE_VIEW,                               // objectType
					(uint64_t)ret.view,                                // objectHandle
					ci.debugName,                            			// pObjectName
				};
				instance->pfnSetDebugUtilsObjectNameEXT(device, &imageNameInfo);
			}

			ret.allocator = allocator;
			ret.tiling = (VkImageTiling)ici.tiling;
			ret.usageFlags = (VkImageUsageFlags)ici.usage;
			ret.viewFormat = (VkFormat)ivci.format;
			ret.type = VK_IMAGE_TYPE_2D;
			ret.extent = ici.extent;
			ret.device = device;
			ret.aspect = (VkImageAspectFlags)ci.imageAspekt;
			ret.arrayLayers = 1;
			ret.mipmapLevels = 1;

			if (ci.sampler.has_value()) {
				ret.sampler = *ci.sampler;
			}
		}

		ImageHandle::ImageHandle(std::shared_ptr<Image> other)
			:image{ std::move(other) }
		{ }
	}
}
