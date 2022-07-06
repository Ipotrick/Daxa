#include "SamplerBackend.hpp"

namespace daxa {
    void SamplerBackend::create(VkDevice device, SamplerInfo const& info) {
        this->info = info;
        
        VkSamplerCreateInfo samplerCI {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = this->info.magFilter,
			.minFilter = this->info.minFilter,
			.mipmapMode =this->info. mipmapMode,
			.addressModeU = this->info.addressModeU,
			.addressModeV = this->info.addressModeV,
			.addressModeW = this->info.addressModeW,
			.mipLodBias = this->info.mipLodBias,
			.anisotropyEnable = this->info.anisotropyEnable,
			.maxAnisotropy = this->info.maxAnisotropy,
			.compareEnable = this->info.compareEnable,
			.compareOp = this->info.compareOp,
			.minLod = this->info.minLod,
			.maxLod = this->info.maxLod,
			.borderColor = this->info.borderColor,
			.unnormalizedCoordinates = this->info.unnormalizedCoordinates,
		};

		DAXA_CHECK_VK_RESULT_M(vkCreateSampler(device, &samplerCI, nullptr, &this->sampler), "failed to create sampler");

		//if (instance->pfnSetDebugUtilsObjectNameEXT != nullptr && this->info.debugName != nullptr) {
		//	VkDebugUtilsObjectNameInfoEXT nameInfo {
		//		.sType =  VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
		//		.pNext = NULL,
		//		.objectType = VK_OBJECT_TYPE_SAMPLER,
		//		.objectHandle = (uint64_t)this->sampler,
		//		.pObjectName = this->info.debugName,
		//	};
		//	daxa::instance->pfnSetDebugUtilsObjectNameEXT(device, &nameInfo);
		//}
    }

    void SamplerBackend::destroy(VkDevice device) {
		vkDestroySampler(device, this->sampler, nullptr);
    }
}