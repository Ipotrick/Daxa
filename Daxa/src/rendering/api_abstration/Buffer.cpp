#include "Buffer.hpp"
#include "common.hpp"

namespace daxa {
	namespace gpu {

		Buffer::Buffer(VkDevice device, u32 queueFamilyIndex, VmaAllocator allocator, BufferCreateInfo ci) {

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

		BufferHandle::BufferHandle(Buffer&& buffer)
			: buffer{ std::make_shared<Buffer>(std::move(buffer)) }
		{ }

		void uploadToStagingBuffer(std::span<u8> hostMemorySrc, BufferHandle bufferDst, size_t offset) {
			DAXA_ASSERT_M(bufferDst->getVmaMemoryUsage() & VMA_MEMORY_USAGE_CPU_TO_GPU, "staging buffer must be of memory type: VMA_MEMORY_USAGE_CPU_TO_GPU!");
			DAXA_ASSERT_M(bufferDst->getSize() + offset >= hostMemorySrc.size(), "staging buffer must be at last or greater than (size of memory that is to be uploaded + offset)!");

			void* bufferMemPtr{ nullptr };
			vmaMapMemory(bufferDst->allocator, bufferDst->allocation, &bufferMemPtr);
			std::memcpy(bufferMemPtr, hostMemorySrc.data(), hostMemorySrc.size());
			vmaUnmapMemory(bufferDst->allocator, bufferDst->allocation);
		}
	}
}
