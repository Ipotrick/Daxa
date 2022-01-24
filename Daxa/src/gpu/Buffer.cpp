#include "Buffer.hpp"
#include "Instance.hpp"

namespace daxa {
	namespace gpu {
		Buffer::Buffer(std::shared_ptr<DeviceBackend> deviceBackend, BufferCreateInfo& ci) 
			: deviceBackend{ std::move(deviceBackend) }
			, size{ ci.size }
			, usage{ ci.usage }
			, memoryUsage{ ci.memoryUsage }
		{
			VkBufferCreateInfo bci{
				.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.pNext = nullptr,
				.size = ci.size,
				.usage = ci.usage,
				.sharingMode = VK_SHARING_MODE_CONCURRENT,
				.queueFamilyIndexCount = (u32)this->deviceBackend->allQFamilyIndices.size(),
				.pQueueFamilyIndices = this->deviceBackend->allQFamilyIndices.data(),
			};

			VmaAllocationCreateInfo aci{
				.usage = ci.memoryUsage,
				.requiredFlags = ci.memoryProperties,
			};

			vmaCreateBuffer(this->deviceBackend->allocator, (VkBufferCreateInfo*)&bci, &aci, &buffer, &allocation, nullptr);

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
		}

		bool Buffer::isUsedByGPU() const {
			return usesOnGPU == 0;
		}

		Buffer::~Buffer() {
			if (this->deviceBackend) {
				vmaDestroyBuffer(this->deviceBackend->allocator, buffer, allocation);
			}
		}

		void Buffer::upload(void const* src, size_t size, size_t dstOffset) {
			DAXA_ASSERT_M(getVmaMemoryUsage() & (VMA_MEMORY_USAGE_CPU_TO_GPU | VMA_MEMORY_USAGE_GPU_TO_CPU), "can only upload to buffers with the memory usage flag: VMA_MEMORY_USAGE_CPU_TO_GPU");
			DAXA_ASSERT_M(getSize() + dstOffset >= size, "uploaded memory overruns the buffer size");
			DAXA_ASSERT_M(usesOnGPU == 0, "can not upload to buffer that is currently in use on the gpu directly from host. to indirectly upload to this buffer, use the command list upload.");

			u8* bufferMemPtr{ nullptr };
			vmaMapMemory(this->deviceBackend->allocator, allocation, (void**)&bufferMemPtr);
			std::memcpy(bufferMemPtr + dstOffset, src, size);
			vmaUnmapMemory(this->deviceBackend->allocator, allocation);
		}

		void* Buffer::mapMemory() {
			DAXA_ASSERT_M(getVmaMemoryUsage() & (VMA_MEMORY_USAGE_CPU_TO_GPU | VMA_MEMORY_USAGE_GPU_TO_CPU), "can only upload to buffers with the memory usage flag: VMA_MEMORY_USAGE_CPU_TO_GPU");
			DAXA_ASSERT_M(usesOnGPU == 0, "can not upload to buffer that is currently in use on the gpu directly from host. to indirectly upload to this buffer, use the command list upload.");
			memoryMapCount += 1;
			void* ret;
			vmaMapMemory(this->deviceBackend->allocator, allocation, &ret);
			return ret;
		}

		void Buffer::unmapMemory() {
			DAXA_ASSERT_M(memoryMapCount > 0, "can not unmap memory of buffer that has no mapped memory.");
			memoryMapCount -= 1;
			vmaUnmapMemory(this->deviceBackend->allocator, allocation);
		}
	}
}
