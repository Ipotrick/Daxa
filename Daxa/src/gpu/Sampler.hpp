#pragma once

#include "../DaxaCore.hpp"

#include <memory>
#include <cstring>

#include <vulkan/vulkan.h>

#include "Handle.hpp"
#include "Graveyard.hpp"

namespace daxa {
	struct SamplerCreateInfo {
		VkFilter                magFilter 				= VK_FILTER_LINEAR;
		VkFilter                minFilter 				= VK_FILTER_LINEAR;
		VkSamplerMipmapMode     mipmapMode 				= VK_SAMPLER_MIPMAP_MODE_LINEAR;
		VkSamplerAddressMode    addressModeU 			= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		VkSamplerAddressMode    addressModeV 			= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		VkSamplerAddressMode    addressModeW 			= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		float                   mipLodBias 				= 0.0f;
		VkBool32                anisotropyEnable 		= VK_FALSE;
		float                   maxAnisotropy 			= 0.0f;
		VkBool32                compareEnable 			= VK_FALSE;
		VkCompareOp             compareOp 				= VK_COMPARE_OP_MAX_ENUM;
		float                   minLod 					= 0.0f;
		float                   maxLod 					= 0.0f;
		VkBorderColor           borderColor 			= VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		VkBool32                unnormalizedCoordinates = VK_FALSE;
		char const* 			debugName 				= {};

		bool operator == (SamplerCreateInfo const& other) const {
			return std::memcmp(this, &other, sizeof(SamplerCreateInfo)) == 0;
		}
	};

	class Sampler : public GraveyardRessource {
	public:
		Sampler(std::shared_ptr<DeviceBackend> deviceBackend, SamplerCreateInfo const& createInfo);
		Sampler(Sampler&&) noexcept										= delete;
		Sampler& operator=(Sampler&&) noexcept							= delete;
		Sampler(Sampler const&) 										= delete;
		Sampler& operator=(Sampler const&) 								= delete;
		virtual ~Sampler();

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
		u16 getDescriptorIndex() const { return samplerIndex; }
		std::string const& getDebugName() const { return debugName; }
	private:
		friend struct SamplerStaticFunctionOverride;

		std::shared_ptr<DeviceBackend> 	deviceBackend 			= {};
		VkSampler						sampler 				= {};
		VkSamplerCreateFlags    		flags					= {};
		VkFilter                		magFilter				= {};
		VkFilter                		minFilter				= {};
		VkSamplerMipmapMode     		mipmapMode				= {};
		VkSamplerAddressMode    		addressModeU			= {};
		VkSamplerAddressMode    		addressModeV			= {};
		VkSamplerAddressMode    		addressModeW			= {};
		float                   		mipLodBias				= {};
		VkBool32                		anisotropyEnable		= {};
		float                   		maxAnisotropy			= {};
		VkBool32                		compareEnable			= {};
		VkCompareOp             		compareOp				= {};
		float                   		minLod					= {};
		float                   		maxLod					= {};
		VkBorderColor           		borderColor				= {};
		VkBool32                		unnormalizedCoordinates	= {};
		u16 							samplerIndex 			= {};
		std::string						debugName 				= {};
	};

	struct SamplerStaticFunctionOverride {
		static void cleanup(std::shared_ptr<Sampler>& value);
	};

	class SamplerHandle : public SharedHandle<Sampler, SamplerStaticFunctionOverride>{};
}