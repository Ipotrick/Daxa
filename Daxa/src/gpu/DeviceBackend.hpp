#pragma once

#include "../DaxaCore.hpp"

#include <vector>
#include <mutex>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <VkBootstrap.h>

#include "Graveyard.hpp"

namespace daxa {
	struct DeviceBackend {
		DeviceBackend(vkb::Instance& instance, PFN_vkSetDebugUtilsObjectNameEXT pfnSetDebugUtilsObjectNameEXT);
		~DeviceBackend();

		vkb::Device device                                                              = {};
		void (*vkCmdBeginRenderingKHR)(VkCommandBuffer, const VkRenderingInfoKHR*) 		= nullptr;
		void (*vkCmdEndRenderingKHR)(VkCommandBuffer) 									= nullptr;
		void (*vkCmdPipelineBarrier2KHR)(VkCommandBuffer, VkDependencyInfoKHR const*) 	= nullptr;
		VmaAllocator allocator 															= {};
		u32 graphicsQFamilyIndex 														= 0xffffffff;
		u32 transferQFamilyIndex														= 0xffffffff;
		u32 computeQFamilyIndex															= 0xffffffff;
		std::vector<u32> allQFamilyIndices 												= {};
		Graveyard graveyard                                                             = {};
		std::mutex bindAllMtx 															= {};
		VkDescriptorPool bindAllSetPool 												= {};
		VkDescriptorSetLayout bindAllSetLayout 											= {};
		VkDescriptorSet bindAllSet 														= {};
		std::vector<u16> imageViewIndexFreeList											= {};
		u16 nextImageViewIndex 															= 1;
		std::vector<u16> samplerIndexFreeList 											= {};
		u16 nextSamplerIndex															= 1;
		std::vector<u16> storageBufferIndexFreeList 									= {};
		u16 nextStorageBufferIndex														= 1;
		VkSampler dummySampler 															= {};
		VkPhysicalDeviceProperties2 properties 											= {};
	private:
		void createDummies();
		void destroyDummies();
	};

	inline static const VkDescriptorPoolSize BIND_ALL_SAMPLER_POOL_SIZE {
		.type = VK_DESCRIPTOR_TYPE_SAMPLER,
		.descriptorCount = (1<<16),
	};
	inline static const VkDescriptorPoolSize BIND_ALL_COMBINED_IMAGE_SAMPLER_POOL_SIZE {
		.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = (1<<16),
	};
	inline static const VkDescriptorPoolSize BIND_ALL_SAMPLED_IMAGE_POOL_SIZE {
		.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		.descriptorCount = (1<<16),
	}; 
	inline static const VkDescriptorPoolSize BIND_ALL_STORAGE_IMAGE_POOL_SIZE {
		.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		.descriptorCount = (1<<16),
	};
	inline static const VkDescriptorPoolSize BIND_ALL_STORAGE_BUFFER_POOL_SIZE {
		.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = (1<<16),
	};

	inline static const VkDescriptorSetLayoutBinding BIND_ALL_SAMPLER_SET_LAYOUT_BINDING {
		.binding = 0,
		.descriptorType = BIND_ALL_SAMPLER_POOL_SIZE.type,
		.descriptorCount = BIND_ALL_SAMPLER_POOL_SIZE.descriptorCount,
		.stageFlags = VK_SHADER_STAGE_ALL,
	};
	inline static const VkDescriptorSetLayoutBinding BIND_ALL_COMBINED_IMAGE_SAMPLER_SET_LAYOUT_BINDING {
		.binding = 1,
		.descriptorType = BIND_ALL_COMBINED_IMAGE_SAMPLER_POOL_SIZE.type,
		.descriptorCount = BIND_ALL_COMBINED_IMAGE_SAMPLER_POOL_SIZE.descriptorCount,
		.stageFlags = VK_SHADER_STAGE_ALL,
	};
	inline static const VkDescriptorSetLayoutBinding BIND_ALL_SAMPLED_IMAGE_SET_LAYOUT_BINDING {
		.binding = 2,
		.descriptorType = BIND_ALL_SAMPLED_IMAGE_POOL_SIZE.type,
		.descriptorCount = BIND_ALL_SAMPLED_IMAGE_POOL_SIZE.descriptorCount,
		.stageFlags = VK_SHADER_STAGE_ALL,
	};
	inline static const VkDescriptorSetLayoutBinding BIND_ALL_STORAGE_IMAGE_SET_LAYOUT_BINDING {
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

	const char* getVkResultString(VkResult result);
}

#ifdef _DEBUG
#define DAXA_CHECK_VK_RESULT_M(x, message)                                      					\
	do {                                                              								\
		VkResult err = x;                                           								\
		if (err)                                                    								\
		{                                                           								\
			char const* errStr = getVkResultString(err);											\
			std::cerr << "[[DAXA_CHECK_VK_RESULT_M ERROR]] " << message << "; error: " << errStr << std::endl; \
			abort();                                                								\
		}                                                           								\
	} while (0)																						
#define DAXA_CHECK_VK_RESULT(x)                                      								\
	do {                                                              								\
		VkResult err = x;                                           								\
		if (err)                                                    								\
		{                                                           								\
			char const* errStr = getVkResultString(err);											\
			std::cerr << "[[DAXA_CHECK_VK_RESULT ERROR]] " << errStr << std::endl; 					\
			abort();                                                								\
		}                                                           								\
	} while (0)
#else
#define DAXA_CHECK_VK_RESULT_M(x, m) (void)x
#define DAXA_CHECK_VK_RESULT(x) (void)x
#endif