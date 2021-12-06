#include "Buffer.hpp"
#include "common.hpp"

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
			};

			vmaCreateBuffer(allocator, (VkBufferCreateInfo*)&bci, &aci, &buffer, &allocation, nullptr);

			this->allocator = allocator;
			this->size = ci.size;
			this->usage = ci.usage;
			this->memoryUsage = ci.memoryUsage;
		}

		bool Buffer::isUsedByGPU() const {
			return bInUseOnGPU;
		}

		Buffer::Buffer() {
			std::memset(this, 0, sizeof(buffer));
		}

		DAXA_DEFINE_TRIVIAL_MOVE(Buffer)

		Buffer::~Buffer() {
			if (allocator) {
				vmaDestroyBuffer(allocator, buffer, allocation);
				std::memset(this, 0, sizeof(Buffer));
			}
		}

		void Buffer::uploadFromHost(void* src, size_t size, size_t dstOffset) {
			DAXA_ASSERT_M(getVmaMemoryUsage() & VMA_MEMORY_USAGE_CPU_TO_GPU, "can only upload to buffers with the memory usage flag: VMA_MEMORY_USAGE_CPU_TO_GPU");
			DAXA_ASSERT_M(getSize() + dstOffset >= size, "uploaded memory overruns the buffer size");

			void* bufferMemPtr{ nullptr };
			vmaMapMemory(allocator, allocation, &bufferMemPtr);
			std::memcpy(bufferMemPtr, src, size);
			vmaUnmapMemory(allocator, allocation);
		}

		BufferHandle::BufferHandle(Buffer&& buffer)
			: buffer{ std::make_shared<Buffer>(std::move(buffer)) }
		{ }
	}
}
