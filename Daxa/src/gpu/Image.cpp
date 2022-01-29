#include "Image.hpp"
#include <cassert>
#include "Instance.hpp"

namespace daxa {
	namespace gpu {
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
		
		ImageView::ImageView(std::shared_ptr<DeviceBackend> deviceBackend, ImageViewCreateInfo const& ci) 
			: deviceBackend{ std::move(deviceBackend) }
			, flags{ ci.flags }
			, image{ ci.image }
			, viewType{ ci.viewType }
			, format{ ci.format }
			, components{ ci.components }
			, subresourceRange{ ci.subresourceRange }
			, defaultSampler{ ci.defaultSampler }
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

			DAXA_CHECK_VK_RESULT_M(vkCreateImageView(this->deviceBackend->device.device, &ivci, nullptr, &view), "failed to create image view");

			if (image->getVkImageUsageFlags() & VK_IMAGE_USAGE_SAMPLED_BIT || 
				image->getVkImageUsageFlags() & VK_IMAGE_USAGE_STORAGE_BIT
			) {
				std::unique_lock bindAllLock(this->deviceBackend->bindAllMtx);
				if (image->getVkImageUsageFlags() & VK_IMAGE_USAGE_SAMPLED_BIT) {
					if (defaultSampler.valid()) {
						u16 index;
						if (this->deviceBackend->combinedImageSamplerIndexFreeList.empty()) {
							index = this->deviceBackend->nextCombinedImageSamplerIndex++;
						} else {
							index = this->deviceBackend->combinedImageSamplerIndexFreeList.back();
							this->deviceBackend->combinedImageSamplerIndexFreeList.pop_back();
						}
						DAXA_ASSERT_M(index > 0 && index < BIND_ALL_COMBINED_IMAGE_SAMPLER_SET_LAYOUT_BINDING.descriptorCount, "failed to create image view: exausted indices for bind all set");
						VkDescriptorImageInfo imageInfo{
							.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
							.imageView = view,
							.sampler = defaultSampler->getVkSampler(),
						};
						VkWriteDescriptorSet write {
							.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
							.pNext = nullptr,
							.dstSet = this->deviceBackend->bindAllSet,
							.dstBinding = BIND_ALL_COMBINED_IMAGE_SAMPLER_SET_LAYOUT_BINDING.binding,
							.dstArrayElement = index,
							.descriptorCount = 1,
							.descriptorType = BIND_ALL_COMBINED_IMAGE_SAMPLER_SET_LAYOUT_BINDING.descriptorType,
							.pImageInfo = &imageInfo,
						};
						vkUpdateDescriptorSets(this->deviceBackend->device.device, 1, &write, 0, nullptr);
						imageSamplerIndex = index;
					}
					u16 index;
					if (this->deviceBackend->sampledImageIndexFreeList.empty()) {
						index = this->deviceBackend->nextSampledImageIndex++;
					} else {
						index = this->deviceBackend->sampledImageIndexFreeList.back();
						this->deviceBackend->sampledImageIndexFreeList.pop_back();
					}
					DAXA_ASSERT_M(index > 0 && index < BIND_ALL_SAMPLED_IMAGE_SET_LAYOUT_BINDING.descriptorCount, "failed to create image view: exausted indices for bind all set");
					VkDescriptorImageInfo imageInfo{
						.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						.imageView = view,
						.sampler = nullptr,
					};
					VkWriteDescriptorSet write {
						.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
						.pNext = nullptr,
						.dstSet = this->deviceBackend->bindAllSet,
						.dstBinding = BIND_ALL_SAMPLED_IMAGE_SET_LAYOUT_BINDING.binding,
						.dstArrayElement = index,
						.descriptorCount = 1,
						.descriptorType = BIND_ALL_SAMPLED_IMAGE_SET_LAYOUT_BINDING.descriptorType,
						.pImageInfo = &imageInfo,
					};
					vkUpdateDescriptorSets(this->deviceBackend->device.device, 1, &write, 0, nullptr);
					sampledImageIndex = index;
				}
				if (image->getVkImageUsageFlags() & VK_IMAGE_USAGE_STORAGE_BIT) {
					u16 index;
					if (this->deviceBackend->storageImageIndexFreeList.empty()) {
						index = this->deviceBackend->nextStorageImageIndex++;
					} else {
						index = this->deviceBackend->storageImageIndexFreeList.back();
						this->deviceBackend->storageImageIndexFreeList.pop_back();
					}
					DAXA_ASSERT_M(index > 0 && index < BIND_ALL_STORAGE_IMAGE_SET_LAYOUT_BINDING.descriptorCount, "failed to create image view: exausted indices for bind all set");
					VkDescriptorImageInfo imageInfo{
						.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
						.imageView = view,
						.sampler = nullptr,
					};
					VkWriteDescriptorSet write {
						.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
						.pNext = nullptr,
						.dstSet = this->deviceBackend->bindAllSet,
						.dstBinding = BIND_ALL_STORAGE_IMAGE_SET_LAYOUT_BINDING.binding,
						.dstArrayElement = index,
						.descriptorCount = 1,
						.descriptorType = BIND_ALL_STORAGE_IMAGE_SET_LAYOUT_BINDING.descriptorType,
						.pImageInfo = &imageInfo,
					};
					vkUpdateDescriptorSets(this->deviceBackend->device.device, 1, &write, 0, nullptr);
					storageImageIndex = index;
				}
			}

			if (instance->pfnSetDebugUtilsObjectNameEXT != nullptr && ci.debugName != nullptr) {
				VkDebugUtilsObjectNameInfoEXT imageNameInfo {
					.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
					.pNext = NULL,
					.objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
					.objectHandle = (uint64_t)view,
					.pObjectName = ci.debugName,
				};
				instance->pfnSetDebugUtilsObjectNameEXT(this->deviceBackend->device.device, &imageNameInfo);
				debugName = ci.debugName;
			}
		}

		ImageView::~ImageView() {
			if (deviceBackend && view) {
				if (imageSamplerIndex != std::numeric_limits<u16>::max() || 
					sampledImageIndex != std::numeric_limits<u16>::max() ||
					storageImageIndex != std::numeric_limits<u16>::max()
				){
					std::unique_lock bindAllLock(deviceBackend->bindAllMtx);
					if (imageSamplerIndex != std::numeric_limits<u16>::max()) {
						this->deviceBackend->combinedImageSamplerIndexFreeList.push_back(imageSamplerIndex);
					}
					if (sampledImageIndex != std::numeric_limits<u16>::max()) {
						this->deviceBackend->sampledImageIndexFreeList.push_back(sampledImageIndex);
					}
					if (storageImageIndex != std::numeric_limits<u16>::max()) {
						this->deviceBackend->storageImageIndexFreeList.push_back(storageImageIndex);
					}
				}
				vkDestroyImageView(deviceBackend->device.device, view, nullptr);
				view = VK_NULL_HANDLE;
			}
		}
	}
}
