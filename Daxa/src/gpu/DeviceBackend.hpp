#pragma once

#include "../DaxaCore.hpp"

#include <vector>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <VkBootstrap.h>

#include "Graveyard.hpp"

namespace daxa {
    namespace gpu {
        struct DeviceBackend {
			DeviceBackend(vkb::Instance& instance);
			~DeviceBackend();

            vkb::Device device                                                              = {};
			void (*vkCmdBeginRenderingKHR)(VkCommandBuffer, const VkRenderingInfoKHR*) 		= nullptr;
			void (*vkCmdEndRenderingKHR)(VkCommandBuffer) 									= nullptr;
			void (*vkCmdPipelineBarrier2KHR)(VkCommandBuffer, VkDependencyInfoKHR const*) 	= nullptr;
			VmaAllocator allocator 															= {};
			u32 graphicsQFamilyIndex 														= -1;
			u32 transferQFamilyIndex														= -1;
			u32 computeQFamilyIndex															= -1;
			std::vector<u32> allQFamilyIndices 												= {};
            Graveyard graveyard                                                             = {};
			VkDescriptorPool bindAllSetPool 												= {};
			VkDescriptorSetLayout bindAllSetLayout 											= {};
			VkDescriptorSet bindAllSet 														= {};
			std::vector<u16> samplerIndexFreeList 											= {};
			u16 nextSamplerIndex															= {};
			std::vector<u16> combinedImageSamplerIndexFreeList 								= {};
			u16 nextCombinedImageSamplerIndex												= {};
			std::vector<u16> sampledImageIndexFreeList 										= {};
			u16 nextSampledImageIndex														= {};
			std::vector<u16> storageImageIndexFreeList 										= {};
			u16 nextStorageImageIndex														= {};
			std::vector<u16> uniformBufferIndexFreeList 									= {};
			u16 nextUnifromBufferIndex														= {};
			std::vector<u16> storageBufferIndexFreeList 									= {};
			u16 nextStorageBufferIndex														= {};
        };

		inline const VkDescriptorPoolSize BIND_ALL_SAMPLER_POOL_SIZE {
			.type = VK_DESCRIPTOR_TYPE_SAMPLER,
			.descriptorCount = (1<<16)-1,
		};
		inline const VkDescriptorPoolSize BIND_ALL_COMBINED_IMAGE_SAMPLER_POOL_SIZE {
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = (1<<12) - 1,
		};
		inline const VkDescriptorPoolSize BIND_ALL_SAMPLED_IMAGE_POOL_SIZE {
			.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.descriptorCount = (1<<16)-1,
		};
		inline const VkDescriptorPoolSize BIND_ALL_STORAGE_IMAGE_POOL_SIZE {
			.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			.descriptorCount = (1<<16)-1,
		};
		inline const VkDescriptorPoolSize BIND_ALL_STORAGE_BUFFER_POOL_SIZE {
			.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = (1<<11) - 1,
		};

		inline const VkDescriptorSetLayoutBinding BIND_ALL_SAMPLER_SET_LAYOUT_BINDING {
			.binding = 0,
			.descriptorType = BIND_ALL_SAMPLER_POOL_SIZE.type,
			.descriptorCount = BIND_ALL_SAMPLER_POOL_SIZE.descriptorCount,
			.stageFlags = VK_SHADER_STAGE_ALL,
		};
		inline const VkDescriptorSetLayoutBinding BIND_ALL_COMBINED_IMAGE_SAMPLER_SET_LAYOUT_BINDING {
			.binding = 1,
			.descriptorType = BIND_ALL_COMBINED_IMAGE_SAMPLER_POOL_SIZE.type,
			.descriptorCount = BIND_ALL_COMBINED_IMAGE_SAMPLER_POOL_SIZE.descriptorCount,
			.stageFlags = VK_SHADER_STAGE_ALL,
		};
		inline const VkDescriptorSetLayoutBinding BIND_ALL_SAMPLED_IMAGE_SET_LAYOUT_BINDING {
			.binding = 2,
			.descriptorType = BIND_ALL_SAMPLED_IMAGE_POOL_SIZE.type,
			.descriptorCount = BIND_ALL_SAMPLED_IMAGE_POOL_SIZE.descriptorCount,
			.stageFlags = VK_SHADER_STAGE_ALL,
		};
		inline const VkDescriptorSetLayoutBinding BIND_ALL_STORAGE_IMAGE_SET_LAYOUT_BINDING {
			.binding = 3,
			.descriptorType = BIND_ALL_STORAGE_IMAGE_POOL_SIZE.type,
			.descriptorCount = BIND_ALL_STORAGE_IMAGE_POOL_SIZE.descriptorCount,
			.stageFlags = VK_SHADER_STAGE_ALL,
		};
		inline const VkDescriptorSetLayoutBinding BIND_ALL_STORAGE_BUFFER_SET_LAYOUT_BINDING {
			.binding = 4,
			.descriptorType = BIND_ALL_STORAGE_BUFFER_POOL_SIZE.type,
			.descriptorCount = BIND_ALL_STORAGE_BUFFER_POOL_SIZE.descriptorCount,
			.stageFlags = VK_SHADER_STAGE_ALL,
		};
    }
}

#ifdef _DEBUG
#define DAXA_CHECK_VK_RESULT(x)                                                 \
	do                                                              			\
	{                                                              				\
		VkResult err = x;                                           			\
		if (err)                                                    			\
		{                                                           			\
			std::cerr << "[[DAXA VULKAN RESULT ERROR]] " << err << std::endl; 	\
			abort();                                                			\
		}                                                           			\
	} while (0)
#else
#define DAXA_CHECK_VK_RESULT(x) x
#endif