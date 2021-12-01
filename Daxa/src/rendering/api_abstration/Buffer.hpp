#pragma once

#include "../../DaxaCore.hpp"

#include <memory>
#include <span>

#include <vulkan/vulkan.h>

#include "../dependencies/vk_mem_alloc.hpp"

namespace daxa {
	namespace gpu {
		struct BufferCreateInfo {
			uz size;
			VkBufferUsageFlags usage;
			VmaMemoryUsage memoryUsage;
		};

		class BufferHandle;

		class Buffer {
		public:
			Buffer();
			Buffer(Buffer&&) noexcept;
			Buffer& operator=(Buffer&&) noexcept;

			~Buffer();

			VkBuffer getVkBuffer() const { return buffer; }
			size_t getSize() const { return size; }
			VkBufferUsageFlags getVkBufferUsage() const { return usage; }
			VmaMemoryUsage getVmaMemoryUsage() const { return memoryUsage; }
		private:
			friend class Device;
			friend class BufferHandle;
			friend class CommandList;
			friend void uploadToStagingBuffer(std::span<u8>, BufferHandle, size_t);

			Buffer(VkDevice device, u32 queueFamilyIndex, VmaAllocator allocator, BufferCreateInfo ci);

			VkBuffer buffer;
			size_t size;
			VkBufferUsageFlags usage;
			VmaMemoryUsage memoryUsage;
			VmaAllocation allocation;
			VmaAllocator allocator;
		};

		class BufferHandle {
		public:
			Buffer& operator*() { return *buffer; }
			Buffer* operator->() { return buffer.get(); }
		private:
			friend class Device;
			BufferHandle(std::shared_ptr<Buffer> buffer);
			std::shared_ptr<Buffer> buffer;
		};

		void uploadToStagingBuffer(std::span<u8> hostMemorySrc, BufferHandle bufferDst, size_t offset);
	}
}