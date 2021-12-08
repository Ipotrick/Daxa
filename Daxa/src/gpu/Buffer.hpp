#pragma once

#include "../DaxaCore.hpp"

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
			Buffer(Buffer const&) = delete;
			Buffer& operator=(Buffer const&) = delete;
			Buffer(Buffer&&) noexcept;
			Buffer& operator=(Buffer&&) noexcept;
			~Buffer();

			/**
			 * \return False when the buffer is safe to be written to from CPU. True when it might be used by the GPU.
			*/
			bool isUsedByGPU() const;

			void uploadFromHost(void const* src, size_t size, size_t dstOffset = 0);

			VkBuffer getVkBuffer() const { return buffer; }
			size_t getSize() const { return size; }
			VkBufferUsageFlags getVkBufferUsage() const { return usage; }
			VmaMemoryUsage getVmaMemoryUsage() const { return memoryUsage; }
		private:
			friend class Device;
			friend class BufferHandle;
			friend class CommandList;
			friend class StagingBufferPool;
			friend class Queue;

			Buffer(VkDevice device, u32 queueFamilyIndex, VmaAllocator allocator, BufferCreateInfo& ci);

			VkBuffer buffer;
			size_t size;
			VkBufferUsageFlags usage;
			VmaMemoryUsage memoryUsage;
			VmaAllocation allocation;
			VmaAllocator allocator;
			bool bInUseOnGPU = false;
		};

		class BufferHandle {
		public:
			BufferHandle() = default;

			Buffer& operator*() { return *buffer; }
			Buffer const& operator*() const { return *buffer; }
			Buffer* operator->() { return buffer.get(); }
			Buffer const* operator->() const { return buffer.get(); }

			operator bool() const { return buffer.operator bool(); }

			size_t getRefCount() const { return buffer.use_count(); }
		private:
			friend class Device;
			friend class StagingBufferPool;

			BufferHandle(Buffer&& buffer);

			std::shared_ptr<Buffer> buffer;
		};
	}
}