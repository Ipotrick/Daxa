#include "BufferBackend.hpp"
#include "../Instance.hpp"

namespace daxa {
    BufferBackend::BufferBackend(std::shared_ptr<DeviceBackend> deviceBackend, BufferCreateInfo& ci) 
		: deviceBackend{ std::move(deviceBackend) }
		, size{ ci.size }
		, memoryType{ ci.memoryType }
	{
        VmaMemoryUsage inferedVmaUsage;
        switch (ci.memoryType) {
            case MemoryType::CPU_ONLY: inferedVmaUsage = VMA_MEMORY_USAGE_CPU_ONLY; break;
            case MemoryType::GPU_ONLY: inferedVmaUsage = VMA_MEMORY_USAGE_GPU_ONLY; break;
            case MemoryType::CPU_TO_GPU: inferedVmaUsage = VMA_MEMORY_USAGE_CPU_TO_GPU; break;
            case MemoryType::GPU_TO_CPU: inferedVmaUsage = VMA_MEMORY_USAGE_GPU_TO_CPU; break;
        };

        VkBufferUsageFlags usageFlags = 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT |
            VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT |
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT |
            VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT |
            VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT |
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
            VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;

		VkBufferCreateInfo bci{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = nullptr,
			.size = ci.size,
			.usage =  usageFlags,
			.sharingMode = VK_SHARING_MODE_CONCURRENT,
			.queueFamilyIndexCount = (u32)this->deviceBackend->allQFamilyIndices.size(),
			.pQueueFamilyIndices = this->deviceBackend->allQFamilyIndices.data(),
		};

		VmaAllocationCreateInfo aci{
			.usage = inferedVmaUsage,
			//.usage = ci.memoryUsage,
			//.requiredFlags = ci.memoryProperties,
		};

		DAXA_CHECK_VK_RESULT_M(vmaCreateBuffer(this->deviceBackend->allocator, (VkBufferCreateInfo*)&bci, &aci, &buffer, &allocation, nullptr), "failed to create buffer");

		if (instance->pfnSetDebugUtilsObjectNameEXT != nullptr && ci.debugName != nullptr) {
			this->debugName = ci.debugName;
			VkDebugUtilsObjectNameInfoEXT nameInfo {
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = NULL,
				.objectType = VK_OBJECT_TYPE_BUFFER,
				.objectHandle = (uint64_t)this->buffer,
				.pObjectName = ci.debugName,
			};
			instance->pfnSetDebugUtilsObjectNameEXT(this->deviceBackend->device.device, &nameInfo);
		}

		//if (ci.usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
			std::unique_lock bindAllLock(this->deviceBackend->bindAllMtx);
			u16 index;
			if (this->deviceBackend->storageBufferIndexFreeList.empty()) {
				index = this->deviceBackend->nextStorageBufferIndex++;
			} else {
				index = this->deviceBackend->storageBufferIndexFreeList.back();
				this->deviceBackend->storageBufferIndexFreeList.pop_back();
			}
			VkDescriptorBufferInfo bufferInfo{
				.buffer = buffer,
				.offset = 0,
				.range = VK_WHOLE_SIZE,
			};
			DAXA_ASSERT_M(index > 0 && index < BIND_ALL_STORAGE_BUFFER_SET_LAYOUT_BINDING.descriptorCount, "failed to create buffer: exausted indices for bind all set");
			VkWriteDescriptorSet write {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = this->deviceBackend->bindAllSet,
				.dstBinding = BIND_ALL_STORAGE_BUFFER_SET_LAYOUT_BINDING.binding,
				.dstArrayElement = index,
				.descriptorCount = 1,
				.descriptorType = BIND_ALL_STORAGE_BUFFER_SET_LAYOUT_BINDING.descriptorType,
				.pBufferInfo = &bufferInfo,
			};
			vkUpdateDescriptorSets(this->deviceBackend->device.device, 1, &write, 0, nullptr);
			descriptorIndex = index;
		//}
	}

	BufferBackend::~BufferBackend() {
		if (this->deviceBackend) {
			if (descriptorIndex != 0) {
				std::unique_lock bindAllLock(deviceBackend->bindAllMtx);
				this->deviceBackend->storageBufferIndexFreeList.push_back(descriptorIndex);
				
				VkDescriptorBufferInfo bufferInfo{
					.buffer = VK_NULL_HANDLE,
					.offset = 0,
					.range = VK_WHOLE_SIZE,
				};
				VkWriteDescriptorSet write {
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.pNext = nullptr,
					.dstSet = this->deviceBackend->bindAllSet,
					.dstBinding = BIND_ALL_STORAGE_BUFFER_SET_LAYOUT_BINDING.binding,
					.dstArrayElement = descriptorIndex,
					.descriptorCount = 1,
					.descriptorType = BIND_ALL_STORAGE_BUFFER_SET_LAYOUT_BINDING.descriptorType,
					.pBufferInfo = &bufferInfo,
				};
				vkUpdateDescriptorSets(this->deviceBackend->device.device, 1, &write, 0, nullptr);
			}
			vmaDestroyBuffer(this->deviceBackend->allocator, buffer, allocation);
		}
	}

	void BufferBackend::upload(void const* src, size_t size, size_t dstOffset) {
		//DAXA_ASSERT_M(getVmaMemoryUsage() & (VMA_MEMORY_USAGE_CPU_TO_GPU | VMA_MEMORY_USAGE_GPU_TO_CPU), "can only upload to buffers with the memory usage flag: VMA_MEMORY_USAGE_CPU_TO_GPU");
		DAXA_ASSERT_M(getSize() + dstOffset >= size, "uploaded memory overruns the buffer size");
		//DAXA_ASSERT_M(usesOnGPU == 0, "can not upload to buffer that is currently in use on the gpu directly from host. to indirectly upload to this buffer, use the command list upload.");

		u8* bufferMemPtr{ nullptr };
		DAXA_CHECK_VK_RESULT_M(vmaMapMemory(this->deviceBackend->allocator, allocation, (void**)&bufferMemPtr), "failed to map buffer memory");
		std::memcpy(bufferMemPtr + dstOffset, src, size);
		vmaUnmapMemory(this->deviceBackend->allocator, allocation);
	}

	void* BufferBackend::mapMemoryRaw() {
		//DAXA_ASSERT_M(getVmaMemoryUsage() & (VMA_MEMORY_USAGE_CPU_TO_GPU | VMA_MEMORY_USAGE_GPU_TO_CPU), "can only upload to buffers with the memory usage flag: VMA_MEMORY_USAGE_CPU_TO_GPU");
		//DAXA_ASSERT_M(usesOnGPU == 0, "can not upload to buffer that is currently in use on the gpu directly from host. to indirectly upload to this buffer, use the command list upload.");
        
        
		memoryMapCount += 1;
		void* ret;
		DAXA_CHECK_VK_RESULT_M(vmaMapMemory(this->deviceBackend->allocator, allocation, &ret), "failed to map buffer memory");
		return ret;
	}

	void BufferBackend::unmapMemoryRaw() {
		DAXA_ASSERT_M(memoryMapCount > 0, "can not unmap memory of buffer that has no mapped memory.");
		memoryMapCount -= 1;
		vmaUnmapMemory(this->deviceBackend->allocator, allocation);
	}
}