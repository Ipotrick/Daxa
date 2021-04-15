#pragma once

#include <unordered_map>

#include "Vulkan.hpp"

namespace std {
	template<>
	struct hash<vk::SamplerCreateInfo> {
		size_t operator()(const vk::SamplerCreateInfo& e) const
		{
			return static_cast<size_t>(
				static_cast<u32>(e.flags) ^
				static_cast<u32>(e.magFilter) ^
				static_cast<u32>(e.minFilter) ^
				static_cast<u32>(e.mipmapMode) ^
				static_cast<u32>(e.addressModeU) ^
				static_cast<u32>(e.addressModeV) ^
				static_cast<u32>(e.addressModeW) ^
				static_cast<u32>(e.mipLodBias) ^
				static_cast<u32>(e.anisotropyEnable) ^
				static_cast<u32>(e.maxAnisotropy) ^
				static_cast<u32>(e.compareEnable) ^
				static_cast<u32>(e.compareOp) ^
				static_cast<u32>(e.minLod) ^
				static_cast<u32>(e.maxLod) ^
				static_cast<u32>(e.borderColor) ^
				static_cast<u32>(e.unnormalizedCoordinates));
		}
	};
}

namespace daxa {
	class SamplerManager {
	public:
		SamplerManager(vk::Device device) : device{ device } {}

		vk::Sampler get(const vk::SamplerCreateInfo& createInfo)
		{
			if (!samplers.contains(createInfo)) {
				samplers[createInfo] = device.createSamplerUnique(createInfo);
			}
			return samplers[createInfo].get();
		}
	private:
		vk::Device device;
		std::unordered_map<vk::SamplerCreateInfo, vk::UniqueSampler> samplers;
	};
}
