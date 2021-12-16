#include "Buffer.hpp"

namespace daxa {
	namespace gpu {

		Buffer::Buffer(VkDevice device, u32 queueFamilyIndex, VmaAllocator allocator, BufferCreateInfo& ci) {

			VkBufferCreateInfo bci{
				.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.pNext = nullptr,
				.size = ci.size,
				.usage = ci.usage,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.queueFamilyIndexCount = 1,
				.pQueueFamilyIndices = &queueFamilyIndex,
			};

			VmaAllocationCreateInfo aci{
				.usage = ci.memoryUsage,
				.requiredFlags = ci.memoryProperties,
			};

			vmaCreateBuffer(allocator, (VkBufferCreateInfo*)&bci, &aci, &buffer, &allocation, nullptr);

			this->allocator = allocator;
			this->size = ci.size;
			this->usage = ci.usage;
			this->memoryUsage = ci.memoryUsage;
		}

		bool Buffer::isUsedByGPU() const {
			return usesOnGPU == 0;
		}

		Buffer::~Buffer() {
			if (allocator) {
				vmaDestroyBuffer(allocator, buffer, allocation);
				allocator = VK_NULL_HANDLE;
			}
		}

		void Buffer::upload(void const* src, size_t size, size_t dstOffset) {
			DAXA_ASSERT_M(getVmaMemoryUsage() & VMA_MEMORY_USAGE_CPU_TO_GPU, "can only upload to buffers with the memory usage flag: VMA_MEMORY_USAGE_CPU_TO_GPU");
			DAXA_ASSERT_M(getSize() + dstOffset >= size, "uploaded memory overruns the buffer size");
			DAXA_ASSERT_M(usesOnGPU == 0, "can not upload to buffer that is currently in use on the gpu directly from host. to indirectly upload to this buffer, use the command list upload.");

			u8* bufferMemPtr{ nullptr };
			vmaMapMemory(allocator, allocation, (void**)&bufferMemPtr);
			std::memcpy(bufferMemPtr + dstOffset, src, size);
			vmaUnmapMemory(allocator, allocation);
		}

		void* Buffer::mapMemory() {
			DAXA_ASSERT_M(getVmaMemoryUsage() & VMA_MEMORY_USAGE_CPU_TO_GPU, "can only upload to buffers with the memory usage flag: VMA_MEMORY_USAGE_CPU_TO_GPU");
			DAXA_ASSERT_M(usesOnGPU == 0, "can not upload to buffer that is currently in use on the gpu directly from host. to indirectly upload to this buffer, use the command list upload.");
			memoryMapCount += 1;
			void* ret;
			vmaMapMemory(allocator, allocation, &ret);
			return ret;
		}

		void Buffer::unmapMemory() {
			DAXA_ASSERT_M(memoryMapCount > 0, "can not unmap memory of buffer that has no mapped memory.");
			memoryMapCount -= 1;
			vmaUnmapMemory(allocator, allocation);
		}
	}
}
