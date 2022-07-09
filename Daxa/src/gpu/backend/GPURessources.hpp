#pragma once

#include "../../DaxaCore.hpp"
#include "Backend.hpp"

#include <memory>
#include <span>
#include <vector>
#include <array>
#include <mutex>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "../GPUHandles.hpp"

#include "BufferBackend.hpp"
#include "ImageViewBackend.hpp"
#include "SamplerBackend.hpp"

namespace daxa {

    constexpr inline size_t MAX_GPU_HANDLES = 500;

    template<typename RessourceBackendT, typename HandleT, size_t MAX_N>
    struct GPURessourcePool {
        static constexpr inline size_t PAGE_BITS = 10;
        static constexpr inline size_t PAGE_SIZE = 1 << PAGE_BITS;
        static constexpr inline size_t PAGE_MASK = PAGE_SIZE-1;

        using PageT = std::array<RessourceBackendT, PAGE_SIZE>;

        std::vector<u32> freeIndexStack = {};
        u32 nextIndex = 1;
        std::vector<u8> versions = {};
        std::mutex mtx = {};
        std::array<std::unique_ptr<PageT>, (MAX_N + (PAGE_SIZE-1)) / PAGE_SIZE> pages = {};

        std::pair<HandleT, RessourceBackendT&> getNew() {
            u32 index = 0;
            if (freeIndexStack.empty()) {
                index = nextIndex++;
                DAXA_ASSERT_M(index < MAX_N, "can not create more gpu ressources");
            } else {
                index = freeIndexStack.back();
                freeIndexStack.pop_back();
            }

            if (index >= versions.size()) {
                versions.resize(index+1, 0);
            }

            u8 version = versions[index];

            size_t page = index >> PAGE_BITS;
            size_t offset = index & PAGE_MASK;

            if (!pages[page]) {
                std::unique_lock lock{ mtx };
                if (!pages[page]) {
                    pages[page] = std::make_unique<PageT>();
                }
            }

            return { HandleT{ index, version }, (*pages[page])[offset] };
        }

        void putback(HandleT handle) {
            if (handle.index >= versions.size()) {
                versions.resize(handle.index + 1, 0);
            }
            versions[handle.index] += 1;
            freeIndexStack.push_back(handle.index);
        }

        RessourceBackendT& get(HandleT handle) {
            size_t page = handle.index >> PAGE_BITS;
            size_t offset = handle.index & PAGE_MASK;
            return (*(pages[page]))[offset];
        }
    };

    inline static const VkDescriptorPoolSize BIND_ALL_SAMPLER_POOL_SIZE {
		.type = VK_DESCRIPTOR_TYPE_SAMPLER,
		.descriptorCount = MAX_GPU_HANDLES,
	};
	inline static const VkDescriptorPoolSize BIND_ALL_COMBINED_IMAGE_SAMPLER_POOL_SIZE {
		.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = MAX_GPU_HANDLES,
	};
	inline static const VkDescriptorPoolSize BIND_ALL_SAMPLED_IMAGE_POOL_SIZE {
		.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		.descriptorCount = MAX_GPU_HANDLES,
	}; 
	inline static const VkDescriptorPoolSize BIND_ALL_STORAGE_IMAGE_POOL_SIZE {
		.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		.descriptorCount = MAX_GPU_HANDLES,
	};
	inline static const VkDescriptorPoolSize BIND_ALL_STORAGE_BUFFER_POOL_SIZE {
		.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = MAX_GPU_HANDLES,
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

    struct GPURessourceTable{
        VkDescriptorPool bindAllSetPool = {};
        VkDescriptorSet bindAllSet = {};
        VkDescriptorSetLayout bindAllSetLayout = {};

        GPURessourcePool<BufferBackend, BufferHandle, MAX_GPU_HANDLES> buffers = {};
        GPURessourcePool<ImageViewBackend, ImageViewHandle, MAX_GPU_HANDLES> imageViews = {};
        GPURessourcePool<SamplerBackend, SamplerHandle, MAX_GPU_HANDLES> samplers = {};

        BufferBackend& getBackend(BufferHandle handle) {
            return buffers.get(handle);
        }

        ImageViewBackend& getBackend(ImageViewHandle handle) {
            return imageViews.get(handle);
        }

        SamplerBackend& getBackend(SamplerHandle handle) {
            return samplers.get(handle);
        }
    };

    void createGPURessourceTable(VkDevice device, GPURessourceTable& table);

    void destroyGPURessourceTable(VkDevice device, GPURessourceTable& table);

    inline BufferHandle createBufferHandleAndInsertIntoTable(VkDevice device, VmaAllocator allocator, GPURessourceTable& table, u32 queueFamilyIndex, BufferInfo const& info) {
        auto [handle, bufferBackend] = table.buffers.getNew();
        bufferBackend.create(device, allocator, queueFamilyIndex, info);

        VkDescriptorBufferInfo bufferInfo{
            .buffer = bufferBackend.buffer,
            .offset = 0,
            .range = VK_WHOLE_SIZE,
        };
        DAXA_ASSERT_M(handle.index > 0 && handle.index < BIND_ALL_STORAGE_BUFFER_SET_LAYOUT_BINDING.descriptorCount, "failed to create buffer: exausted indices for bind all set");
        VkWriteDescriptorSet write {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = table.bindAllSet,
            .dstBinding = BIND_ALL_STORAGE_BUFFER_SET_LAYOUT_BINDING.binding,
            .dstArrayElement = handle.index,
            .descriptorCount = 1,
            .descriptorType = BIND_ALL_STORAGE_BUFFER_SET_LAYOUT_BINDING.descriptorType,
            .pBufferInfo = &bufferInfo,
        };
        vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);

        return handle;
    }

    inline ImageViewHandle createImageViewHandleAndInsertIntoTable(VkDevice device, GPURessourceTable& table, ImageViewInfo const& info, VkImageView preCreatedView = VK_NULL_HANDLE) {
        auto [handle, imageViewBackend] = table.imageViews.getNew();
        imageViewBackend.create(device, info, preCreatedView);
        
        if (info.image->getVkImageUsageFlags() & VK_IMAGE_USAGE_SAMPLED_BIT) {
            VkDescriptorImageInfo imageInfo{
                .sampler = nullptr,
                .imageView = imageViewBackend.imageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
            VkWriteDescriptorSet write {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = table.bindAllSet,
                .dstBinding = BIND_ALL_SAMPLED_IMAGE_SET_LAYOUT_BINDING.binding,
                .dstArrayElement = handle.index,
                .descriptorCount = 1,
                .descriptorType = BIND_ALL_SAMPLED_IMAGE_SET_LAYOUT_BINDING.descriptorType,
                .pImageInfo = &imageInfo,
            };
            vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
        }
        if (info.image->getVkImageUsageFlags() & VK_IMAGE_USAGE_STORAGE_BIT) {
            VkDescriptorImageInfo imageInfo{
                .sampler = nullptr,
                .imageView = imageViewBackend.imageView,
                .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
            };
            VkWriteDescriptorSet write {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = table.bindAllSet,
                .dstBinding = BIND_ALL_STORAGE_IMAGE_SET_LAYOUT_BINDING.binding,
                .dstArrayElement = handle.index,
                .descriptorCount = 1,
                .descriptorType = BIND_ALL_STORAGE_IMAGE_SET_LAYOUT_BINDING.descriptorType,
                .pImageInfo = &imageInfo,
            };
            vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
        }

        return handle;
    }

    inline SamplerHandle createSamplerHandleAndInsertIntoTable(VkDevice device, GPURessourceTable& table, SamplerInfo const& info) {
        auto [handle, samplerBackend] = table.samplers.getNew();
        samplerBackend.create(device, info);

		DAXA_ASSERT_M(handle.index > 0 && handle.index < BIND_ALL_SAMPLER_SET_LAYOUT_BINDING.descriptorCount, "failed to create sampler: exausted indices for bind all set");
		VkDescriptorImageInfo imageInfo{
			.sampler = samplerBackend.sampler,
		};
		VkWriteDescriptorSet write {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = table.bindAllSet,
			.dstBinding = BIND_ALL_SAMPLER_SET_LAYOUT_BINDING.binding,
			.dstArrayElement = handle.index,
			.descriptorCount = 1,
			.descriptorType = BIND_ALL_SAMPLER_SET_LAYOUT_BINDING.descriptorType,
			.pImageInfo = &imageInfo,
		};
		vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);

        return handle;
    }

    inline void destroyBufferAndRemoveFromTable(VkDevice device, VmaAllocator allocator, GPURessourceTable& table, BufferHandle handle) {
        // TODO(pahrens): replace descriptor index slot with a dummy
        table.buffers.get(handle).destroy(allocator);
        table.buffers.putback(handle);
    }

    inline void destroyImageViewAndRemoveFromTable(VkDevice device, GPURessourceTable& table, ImageViewHandle handle) {
        // TODO(pahrens): replace descriptor index slot with a dummy
        table.imageViews.get(handle).destroy(device);
        table.imageViews.putback(handle);
    }

    inline void destroySamplerAndRemoveFromTable(VkDevice device, GPURessourceTable& table, SamplerHandle handle) {
        // TODO(pahrens): replace descriptor index slot with a dummy
        table.samplers.get(handle).destroy(device);
        table.samplers.putback(handle);
    }
}