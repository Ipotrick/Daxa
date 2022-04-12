#include "Sampler.hpp"
#include "Instance.hpp"
#include "backend/DeviceBackend.hpp"

namespace daxa {
	Sampler::Sampler(std::shared_ptr<DeviceBackend> deviceBackend, SamplerCreateInfo const& ci) 
		: deviceBackend{ std::move(deviceBackend) }
		, flags{ 0 }
		, magFilter{ ci.magFilter }
		, minFilter{ ci.minFilter }
		, mipmapMode{ ci.mipmapMode }
		, addressModeU{ ci.addressModeU }
		, addressModeV{ ci.addressModeV }
		, addressModeW{ ci.addressModeW }
		, mipLodBias{ ci.mipLodBias }
		, anisotropyEnable{ ci.anisotropyEnable }
		, maxAnisotropy{ ci.maxAnisotropy }
		, compareEnable{ ci.compareEnable }
		, compareOp{ ci.compareOp }
		, minLod{ ci.minLod }
		, maxLod{ ci.maxLod }
		, borderColor{ ci.borderColor }
		, unnormalizedCoordinates{ ci.unnormalizedCoordinates }
	{
		VkSamplerCreateInfo samplerCI {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = magFilter,
			.minFilter = minFilter,
			.mipmapMode = mipmapMode,
			.addressModeU = addressModeU,
			.addressModeV = addressModeV,
			.addressModeW = addressModeW,
			.mipLodBias = mipLodBias,
			.anisotropyEnable = anisotropyEnable,
			.maxAnisotropy = maxAnisotropy,
			.compareEnable = compareEnable,
			.compareOp = compareOp,
			.minLod = minLod,
			.maxLod = maxLod,
			.borderColor = borderColor,
			.unnormalizedCoordinates = unnormalizedCoordinates,
		};

		DAXA_CHECK_VK_RESULT_M(vkCreateSampler(this->deviceBackend->device.device, &samplerCI, nullptr, &sampler), "failed to create sampler");

		if (instance->pfnSetDebugUtilsObjectNameEXT != nullptr && ci.debugName != nullptr) {
			this->debugName = ci.debugName;

			VkDebugUtilsObjectNameInfoEXT nameInfo {
				.sType =  VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = NULL,
				.objectType = VK_OBJECT_TYPE_SAMPLER,
				.objectHandle = (uint64_t)sampler,
				.pObjectName = ci.debugName,
			};
			daxa::instance->pfnSetDebugUtilsObjectNameEXT(this->deviceBackend->device.device, &nameInfo);
		}

		std::unique_lock bindAllLock(this->deviceBackend->bindAllMtx);
		u16 index;
		if (this->deviceBackend->samplerIndexFreeList.empty()) {
			index = this->deviceBackend->nextSamplerIndex++;
		} else {
			index = this->deviceBackend->samplerIndexFreeList.back();
			this->deviceBackend->samplerIndexFreeList.pop_back();
		}
		DAXA_ASSERT_M(index > 0 && index < BIND_ALL_SAMPLER_SET_LAYOUT_BINDING.descriptorCount, "failed to create sampler: exausted indices for bind all set");
		VkDescriptorImageInfo imageInfo{
			.sampler = sampler,
		};
		VkWriteDescriptorSet write {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = this->deviceBackend->bindAllSet,
			.dstBinding = BIND_ALL_SAMPLER_SET_LAYOUT_BINDING.binding,
			.dstArrayElement = index,
			.descriptorCount = 1,
			.descriptorType = BIND_ALL_SAMPLER_SET_LAYOUT_BINDING.descriptorType,
			.pImageInfo = &imageInfo,
		};
		vkUpdateDescriptorSets(this->deviceBackend->device.device, 1, &write, 0, nullptr);
		samplerIndex = index;
	}

	Sampler::~Sampler() {
		if (this->deviceBackend->device.device) {
			{
				std::unique_lock bindAllLock(this->deviceBackend->bindAllMtx);
				this->deviceBackend->samplerIndexFreeList.push_back(samplerIndex);
				
				VkDescriptorImageInfo imageInfo{
					.sampler = deviceBackend->dummySampler,
					.imageView = VK_NULL_HANDLE,
					.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				};
				VkWriteDescriptorSet write {
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.pNext = nullptr,
					.dstSet = this->deviceBackend->bindAllSet,
					.dstBinding = BIND_ALL_SAMPLER_SET_LAYOUT_BINDING.binding,
					.dstArrayElement = samplerIndex,
					.descriptorCount = 1,
					.descriptorType = BIND_ALL_SAMPLER_SET_LAYOUT_BINDING.descriptorType,
					.pImageInfo = &imageInfo,
				};
				vkUpdateDescriptorSets(this->deviceBackend->device.device, 1, &write, 0, nullptr);
			}
			vkDestroySampler(this->deviceBackend->device.device, sampler, nullptr);
			this->deviceBackend = {};
		}
	}
	
	void SamplerStaticFunctionOverride::cleanup(std::shared_ptr<Sampler>& value) {
		if (value && value.use_count() == 1) {
			std::unique_lock lock(value->deviceBackend->graveyard.mtx);
			for (auto& zombieList : value->deviceBackend->graveyard.activeZombieLists) {
				zombieList->zombies.push_back(value);
			}
		}
	}
}