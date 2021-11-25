#include "Image.hpp"
#include "common.hpp"
#include <cassert>

namespace daxa {
	namespace gpu {

		DAXA_DEFINE_TRIVIAL_MOVE(Image)

		Image::~Image() {
			if (allocator) {
				vmaDestroyImage(allocator, image, allocation);
				allocator = nullptr;
				allocation = nullptr;
				image = nullptr;
				view = {};
			}
		}

		Image Image::create2dImage(vk::Device device, VmaAllocator allocator, Image2dCreateInfo ci) {
			Image ret;

			vk::ImageCreateInfo ici{};
			ici.imageType = vk::ImageType::e2D;
			ici.format = ci.format;
			ici.extent = vk::Extent3D{ ci.width, ci.height,1 };
			ici.mipLevels = 1;
			ici.arrayLayers = 1;
			ici.usage = ci.imageUsage;
			ici.tiling = vk::ImageTiling::eOptimal;
			ici.initialLayout = vk::ImageLayout::eUndefined;
			ici.sharingMode = vk::SharingMode::eExclusive;
			ici.samples = vk::SampleCountFlagBits::e1;

			VmaAllocationCreateInfo aci{};
			aci.usage = ci.memoryUsage;
			aci.requiredFlags = (VkMemoryPropertyFlags)ci.memoryPropertyFlags;

			auto err = vmaCreateImage(allocator, (VkImageCreateInfo*)&ici, &aci, (VkImage*)&ret.image, &ret.allocation, nullptr);
			assert(err == VK_SUCCESS);

			vk::ImageViewCreateInfo ivci{};
			ivci.image = ret.image;
			ivci.format = ci.format;
			ivci.viewType = vk::ImageViewType::e2D;
			ivci.subresourceRange.baseMipLevel = 0;
			ivci.subresourceRange.levelCount = 1;
			ivci.subresourceRange.baseArrayLayer = 0;
			ivci.subresourceRange.layerCount = 1;
			ivci.subresourceRange.aspectMask = ci.imageAspekt;

			ret.view = device.createImageViewUnique(ivci);

			ret.allocator = allocator;
			ret.createInfo = ici;
			ret.viewCreateInfo = ivci;

			return std::move(ret);
		}

		ImageHandle::ImageHandle(std::shared_ptr<Image> image) {
			this->image = image;
		}
	}
}
