#include "Image.hpp"
#include "common.hpp"
#include <cassert>

namespace daxa {
	namespace gpu {

		DAXA_DEFINE_TRIVIAL_MOVE(Image)

		Image::~Image() {
			if (device && view) {
				vkDestroyImageView(device, view, nullptr);
				printf("destroy view\n");
			}
			if (allocator && image && allocation) {
				vmaDestroyImage(allocator, image, allocation);
				printf("destroy image\n");
			}
			std::memset(this, 0, sizeof(Image));
		}

		Image Image::create2dImage(VkDevice device, VmaAllocator allocator, u32 queueFamilyIndex, Image2dCreateInfo ci) {
			Image ret;

			VkImageCreateInfo ici{
				.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.pNext = nullptr,
				.imageType = VkImageType::VK_IMAGE_TYPE_2D,
				.format = ci.format,
				.extent = VkExtent3D{ ci.width, ci.height,1 },
				.mipLevels = 1,
				.arrayLayers = 1,
				.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
				.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL,
				.usage = ci.imageUsage,
				.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
				.queueFamilyIndexCount = 1,
				.pQueueFamilyIndices = &queueFamilyIndex,
				.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
			};

			VmaAllocationCreateInfo aci{ 
				.usage = ci.memoryUsage,
				.requiredFlags = (VkMemoryPropertyFlags)ci.memoryPropertyFlags,
			};
			

			auto err = vmaCreateImage(allocator, (VkImageCreateInfo*)&ici, &aci, (VkImage*)&ret.image, &ret.allocation, nullptr);
			DAXA_ASSERT_M(err == VK_SUCCESS, "could not create image");

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

			return std::move(ret);
		}

		//ImageHandle::~ImageHandle() {
		//	if (image) {
		//	}
		//	//image.reset();
		//}
		//
		//ImageHandle::ImageHandle(ImageHandle&& other) noexcept {
		//	this->image = std::move(other.image);
		//}
		//
		//ImageHandle& ImageHandle::operator=(ImageHandle&& other) noexcept {
		//	this->image = std::move(other.image);
		//	return *this;
		//}
		//
		//ImageHandle::ImageHandle(ImageHandle const& other) {
		//	this->image = other.image;
		//}
		//
		//ImageHandle& ImageHandle::operator=(ImageHandle const& other) {
		//	this->image = other.image;
		//	return *this;
		//}

		ImageHandle::ImageHandle(std::shared_ptr<Image> other)
			:image{ std::move(other) }
		{
			//counters[image.get()] = image.use_count();
		}
	}
}
