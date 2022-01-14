#include "Sampler.hpp"
#include "Instance.hpp"

namespace daxa {
	namespace gpu {

		Sampler::Sampler(VkDevice device, SamplerCreateInfo const& ci) 
			: device{ device }
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

			vkCreateSampler(device, &samplerCI, nullptr, &sampler);

			if (instance->pfnSetDebugUtilsObjectNameEXT != nullptr && ci.debugName != nullptr) {
				this->debugName = ci.debugName;

				auto nameInfo = VkDebugUtilsObjectNameInfoEXT{
					.sType =  VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
					.pNext = NULL,
					.objectType = VK_OBJECT_TYPE_SAMPLER,
					.objectHandle = (uint64_t)sampler,
					.pObjectName = ci.debugName,
				};
				daxa::gpu::instance->pfnSetDebugUtilsObjectNameEXT(device, &nameInfo);
			}
		}

		Sampler::~Sampler() {
			if (device) {
				vkDestroySampler(device, sampler, nullptr);
				device = VK_NULL_HANDLE;
			}
		}
	}
}