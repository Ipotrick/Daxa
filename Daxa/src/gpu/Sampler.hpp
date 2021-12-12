#pragma once

#include "../DaxaCore.hpp"

#include <memory>

#include <vulkan/vulkan.h>

namespace daxa {
	namespace gpu {

		struct SamplerCreateInfo {
			VkFilter                magFilter = VK_FILTER_LINEAR;
			VkFilter                minFilter = VK_FILTER_LINEAR;
			VkSamplerMipmapMode     mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			VkSamplerAddressMode    addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			VkSamplerAddressMode    addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			VkSamplerAddressMode    addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			float                   mipLodBias = 0.0f;
			VkBool32                anisotropyEnable = VK_FALSE;
			float                   maxAnisotropy = 0.0f;
			VkBool32                compareEnable = VK_FALSE;
			VkCompareOp             compareOp = VK_COMPARE_OP_MAX_ENUM;
			float                   minLod = 0.0f;
			float                   maxLod = 0.0f;
			VkBorderColor           borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			VkBool32                unnormalizedCoordinates = VK_FALSE;
		};

		class Sampler {
		public:
			Sampler(VkDevice device, SamplerCreateInfo const& createInfo);
			Sampler(Sampler&&) noexcept										= delete;
			Sampler& operator=(Sampler&&) noexcept							= delete;
			Sampler(Sampler const&) 										= delete;
			Sampler& operator=(Sampler const&) 								= delete;
			~Sampler();

			VkSampler getVkSampler() const { return sampler; }
			VkFilter getVkMagFilter() const { return magFilter; }
			VkFilter getVkMinFilter() const { return minFilter; }
			VkSamplerMipmapMode getVkMipmapMode() const { return mipmapMode; }
			VkSamplerAddressMode getVkSamplerAdressModeU() const { return addressModeU; }
			VkSamplerAddressMode getVkSamplerAdressModeV() const { return addressModeV; }
			VkSamplerAddressMode getVkSamplerAdressModeW() const { return addressModeW; }
			float getVkMipLodBias() const { return mipLodBias; }
			VkBool32 getVkAnisotropyEnable() const { return anisotropyEnable; }
			float getVkMaxAnisotropy() const { return maxAnisotropy; }
			VkBool32 getVkCompareEnable() const { return compareEnable; }
			VkCompareOp getVkCompareOp() const { return compareOp; }
			float getVkMinLod () const { return minLod; }
			float getVkMasLod () const { return maxLod; }
			VkBorderColor getVkBorderColor () const { return borderColor; }
			VkBool32 getVkNormalizedCoodinatesEnable () const { return unnormalizedCoordinates; }
		private:
			VkDevice				device = VK_NULL_HANDLE;
			VkSampler				sampler = VK_NULL_HANDLE;
			VkSamplerCreateFlags    flags;
			VkFilter                magFilter;
			VkFilter                minFilter;
			VkSamplerMipmapMode     mipmapMode;
			VkSamplerAddressMode    addressModeU;
			VkSamplerAddressMode    addressModeV;
			VkSamplerAddressMode    addressModeW;
			float                   mipLodBias;
			VkBool32                anisotropyEnable;
			float                   maxAnisotropy;
			VkBool32                compareEnable;
			VkCompareOp             compareOp;
			float                   minLod;
			float                   maxLod;
			VkBorderColor           borderColor;
			VkBool32                unnormalizedCoordinates;
		};

		class SamplerHandle {
		public:
			SamplerHandle() = default;
			SamplerHandle(std::shared_ptr<Sampler> sampler);

			size_t getRefCount() const { return sampler.use_count(); }

			operator bool() const { return sampler.operator bool(); }
			bool operator!() const { return !sampler; }
			bool valid() const { return *this; }

			Sampler const& operator*() const { return *sampler; }
			Sampler& operator*() { return *sampler; }
			Sampler const* operator->() const { return sampler.get(); }
			Sampler* operator->() { return sampler.get(); }
		private:

			std::shared_ptr<Sampler> sampler = {};
		};
	}
}