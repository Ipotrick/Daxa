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
	
	ImageView::ImageView(std::shared_ptr<DeviceBackend> deviceBackend, ImageViewCreateInfo const& ci, VkImageView givenView) 
		: deviceBackend{ std::move(deviceBackend) }
		, flags{ ci.flags }
		, image{ ci.image }
		, viewType{ ci.viewType }
		, format{ ci.format }
		, components{ ci.components }
		, subresourceRange{ ci.subresourceRange } 
		, defaultSampler{ ci.defaultSampler }
	{
		if (this->subresourceRange.aspectMask == VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM) {
			this->subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = image->getMipLevels(),
				.baseArrayLayer = 0,
				.layerCount = image->getArrayLayers(),
			};
		}

		if (givenView == VK_NULL_HANDLE) {
			VkImageViewCreateInfo ivci{
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext = nullptr,
				.flags = this->flags,
				.image = this->image->getVkImage(),
				.viewType = this->viewType,
				.format = this->format,
				.components = this->components,
				.subresourceRange = this->subresourceRange,
			};

			DAXA_CHECK_VK_RESULT_M(vkCreateImageView(this->deviceBackend->device.device, &ivci, nullptr, &view), "failed to create image view");
		} else {
			view = givenView;
		}

		if (image->getVkImageUsageFlags() & VK_IMAGE_USAGE_SAMPLED_BIT || 
			image->getVkImageUsageFlags() & VK_IMAGE_USAGE_STORAGE_BIT
		) {
			std::unique_lock bindAllLock(this->deviceBackend->bindAllMtx);

			if (!this->deviceBackend->imageViewIndexFreeList.empty()) {
				this->descriptorIndex = this->deviceBackend->imageViewIndexFreeList.back();
				this->deviceBackend->imageViewIndexFreeList.pop_back();
			}
			else {
				this->descriptorIndex = this->deviceBackend->nextImageViewIndex++;
			}

			if (image->getVkImageUsageFlags() & VK_IMAGE_USAGE_SAMPLED_BIT) {
				if (defaultSampler.valid()) {
					VkDescriptorImageInfo imageInfo{
						.sampler = defaultSampler->getVkSampler(),
						.imageView = view,
						.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					};
					VkWriteDescriptorSet write {
						.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
						.pNext = nullptr,
						.dstSet = this->deviceBackend->bindAllSet,
						.dstBinding = BIND_ALL_COMBINED_IMAGE_SAMPLER_SET_LAYOUT_BINDING.binding,
						.dstArrayElement = this->descriptorIndex,
						.descriptorCount = 1,
						.descriptorType = BIND_ALL_COMBINED_IMAGE_SAMPLER_SET_LAYOUT_BINDING.descriptorType,
						.pImageInfo = &imageInfo,
					};
					vkUpdateDescriptorSets(this->deviceBackend->device.device, 1, &write, 0, nullptr);
				}
				VkDescriptorImageInfo imageInfo{
					.sampler = nullptr,
					.imageView = view,
					.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				};
				VkWriteDescriptorSet write {
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.pNext = nullptr,
					.dstSet = this->deviceBackend->bindAllSet,
					.dstBinding = BIND_ALL_SAMPLED_IMAGE_SET_LAYOUT_BINDING.binding,
					.dstArrayElement = this->descriptorIndex,
					.descriptorCount = 1,
					.descriptorType = BIND_ALL_SAMPLED_IMAGE_SET_LAYOUT_BINDING.descriptorType,
					.pImageInfo = &imageInfo,
				};
				vkUpdateDescriptorSets(this->deviceBackend->device.device, 1, &write, 0, nullptr);
			}
			if (image->getVkImageUsageFlags() & VK_IMAGE_USAGE_STORAGE_BIT) {
				VkDescriptorImageInfo imageInfo{
					.sampler = nullptr,
					.imageView = view,
					.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
				};
				VkWriteDescriptorSet write {
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.pNext = nullptr,
					.dstSet = this->deviceBackend->bindAllSet,
					.dstBinding = BIND_ALL_STORAGE_IMAGE_SET_LAYOUT_BINDING.binding,
					.dstArrayElement = this->descriptorIndex,
					.descriptorCount = 1,
					.descriptorType = BIND_ALL_STORAGE_IMAGE_SET_LAYOUT_BINDING.descriptorType,
					.pImageInfo = &imageInfo,
				};
				vkUpdateDescriptorSets(this->deviceBackend->device.device, 1, &write, 0, nullptr);
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
			if (descriptorIndex != 0){
				std::unique_lock bindAllLock(deviceBackend->bindAllMtx);

				this->deviceBackend->imageViewIndexFreeList.push_back(descriptorIndex);

				if (image->getVkImageUsageFlags() & VK_IMAGE_USAGE_SAMPLED_BIT) {
					VkDescriptorImageInfo imageInfo{
						.sampler = VK_NULL_HANDLE,
						.imageView = VK_NULL_HANDLE,
						.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					};
					VkWriteDescriptorSet write {
						.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
						.pNext = nullptr,
						.dstSet = this->deviceBackend->bindAllSet,
						.dstBinding = BIND_ALL_SAMPLED_IMAGE_SET_LAYOUT_BINDING.binding,
						.dstArrayElement = descriptorIndex,
						.descriptorCount = 1,
						.descriptorType = BIND_ALL_SAMPLED_IMAGE_SET_LAYOUT_BINDING.descriptorType,
						.pImageInfo = &imageInfo,
					};
					vkUpdateDescriptorSets(this->deviceBackend->device.device, 1, &write, 0, nullptr);

					if (defaultSampler.valid()) {
						VkDescriptorImageInfo imageInfo{
							.sampler = deviceBackend->dummySampler,
							.imageView = VK_NULL_HANDLE,
							.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
						};
						VkWriteDescriptorSet write {
							.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
							.pNext = nullptr,
							.dstSet = this->deviceBackend->bindAllSet,
							.dstBinding = BIND_ALL_COMBINED_IMAGE_SAMPLER_SET_LAYOUT_BINDING.binding,
							.dstArrayElement = descriptorIndex,
							.descriptorCount = 1,
							.descriptorType = BIND_ALL_COMBINED_IMAGE_SAMPLER_SET_LAYOUT_BINDING.descriptorType,
							.pImageInfo = &imageInfo,
						};
						vkUpdateDescriptorSets(this->deviceBackend->device.device, 1, &write, 0, nullptr);
					}
				}
				if (image->getVkImageUsageFlags() & VK_IMAGE_USAGE_STORAGE_BIT) {
					VkDescriptorImageInfo imageInfo{
						.sampler = VK_NULL_HANDLE,
						.imageView = VK_NULL_HANDLE,
						.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					};
					VkWriteDescriptorSet write {
						.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
						.pNext = nullptr,
						.dstSet = this->deviceBackend->bindAllSet,
						.dstBinding = BIND_ALL_STORAGE_IMAGE_SET_LAYOUT_BINDING.binding,
						.dstArrayElement = descriptorIndex,
						.descriptorCount = 1,
						.descriptorType = BIND_ALL_STORAGE_IMAGE_SET_LAYOUT_BINDING.descriptorType,
						.pImageInfo = &imageInfo,
					};
					vkUpdateDescriptorSets(this->deviceBackend->device.device, 1, &write, 0, nullptr);
				}
			}
			vkDestroyImageView(deviceBackend->device.device, view, nullptr);
			view = VK_NULL_HANDLE;
		}
	}

	void ImageViewStaticFunctionOverride::cleanup(std::shared_ptr<ImageView>& value) {
		if (value && value->deviceBackend && value.use_count() == 1) {
			std::unique_lock lock(value->deviceBackend->graveyard.mtx);
			for (auto& zombieList : value->deviceBackend->graveyard.activeZombieLists) {
				zombieList->zombies.push_back(value);
			}
		}
	}
}
