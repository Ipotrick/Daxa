#pragma once

#include "../DaxaCore.hpp"

#include <memory>
#include <unordered_map>
#include <optional>

#include <vulkan/vulkan.h>

#include "Image.hpp"

namespace daxa {
	enum class MemoryType {
		GPU_ONLY,
		CPU_ONLY,
		CPU_TO_GPU,
		GPU_TO_CPU
	};

	struct GPUHandle {
		u32 index : 24;
		u32 version : 8;

		bool isValid() const { return index != 0; }
	};

	struct BufferInfo {
		uz 				size		= {};
		MemoryType		memoryType 	= MemoryType::GPU_ONLY;
		char const* 	debugName 	= {};
	};

	struct BufferHandle : public GPUHandle{
		static BufferHandle fromRaw(u32 integer) { 
			BufferHandle id;
			std::memcpy(&id, &integer, sizeof(u32));
			return id;
		}
	};

	struct ImageViewInfo {
		VkImageViewCreateFlags     	flags				= {};
		ImageHandle					image		 		= {};
		VkImageViewType            	viewType			= VK_IMAGE_VIEW_TYPE_2D;
		VkFormat                   	format				= {};
		VkComponentMapping         	components			= {};
		VkImageSubresourceRange    	subresourceRange	= {
			.aspectMask = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM,
			.baseMipLevel = 0,
			.levelCount = 0,
			.baseArrayLayer = 0,
			.layerCount = 0,
		};
		char const* 				debugName			= {};
	};

	struct ImageViewHandle : public GPUHandle{
		static ImageViewHandle fromRaw(u32 integer) { 
			ImageViewHandle id;
			std::memcpy(&id, &integer, sizeof(u32));
			return id;
		}
	};

	struct SamplerInfo {
		VkFilter               	magFilter 				= VK_FILTER_LINEAR;
		VkFilter               	minFilter 				= VK_FILTER_LINEAR;
		VkSamplerMipmapMode    	mipmapMode 				= VK_SAMPLER_MIPMAP_MODE_LINEAR;
		VkSamplerAddressMode   	addressModeU 			= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		VkSamplerAddressMode   	addressModeV 			= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		VkSamplerAddressMode   	addressModeW 			= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		f32                   	mipLodBias 				= 0.0f;
		VkBool32               	anisotropyEnable 		= VK_FALSE;
		f32                   	maxAnisotropy 			= 0.0f;
		VkBool32               	compareEnable 			= VK_FALSE;
		VkCompareOp            	compareOp 				= VK_COMPARE_OP_MAX_ENUM;
		f32                   	minLod 					= 0.0f;
		f32                   	maxLod 					= 0.0f;
		VkBorderColor          	borderColor 			= VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		VkBool32               	unnormalizedCoordinates = VK_FALSE;
		char const* 			debugName 				= {};

		bool operator == (SamplerInfo const& other) const {
			return std::memcmp(this, &other, sizeof(SamplerInfo)) == 0;
		}
	};

	struct SamplerHandle : public GPUHandle{ 
		static SamplerHandle fromRaw(u32 integer) { 
			SamplerHandle id;
			std::memcpy(&id, &integer, sizeof(u32));
			return id;
		}
	};

	enum class GPUHandleType {
		Buffer,
		ImageView,
		Sampler
	};
}