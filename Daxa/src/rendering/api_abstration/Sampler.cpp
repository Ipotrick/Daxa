#include "Sampler.hpp"

#include "common.hpp"

namespace daxa {
	namespace gpu {

		DAXA_DEFINE_TRIVIAL_MOVE(Sampler)

		Sampler::Sampler(VkDevice device, SamplerCreateInfo const& createInfo) 
			: device{ device }
			, flags{ 0 }
			, magFilter{ createInfo.magFilter }
			, minFilter{ createInfo.minFilter }
			, mipmapMode{ createInfo.mipmapMode }
			, addressModeU{ createInfo.addressModeU }
			, addressModeV{ createInfo.addressModeV }
			, addressModeW{ createInfo.addressModeW }
			, mipLodBias{ createInfo.mipLodBias }
			, anisotropyEnable{ createInfo.anisotropyEnable }
			, maxAnisotropy{ createInfo.maxAnisotropy }
			, compareEnable{ createInfo.compareEnable }
			, compareOp{ createInfo.compareOp }
			, minLod{ createInfo.minLod }
			, maxLod{ createInfo.maxLod }
			, borderColor{ createInfo.borderColor }
			, unnormalizedCoordinates{ createInfo.unnormalizedCoordinates }
		{
			VkSamplerCreateInfo samplerCI{
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
		}

		Sampler::~Sampler() {
			if (device) {
				vkDestroySampler(device, sampler, nullptr);
			}
			std::memset(this, 0, sizeof(Sampler));
		}
	}
}