#pragma once

#include "../DaxaCore.hpp"

#include <memory>
#include <span>

#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>

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
			Buffer(VkDevice device, u32 queueFamilyIndex, VmaAllocator allocator, BufferCreateInfo& ci);
			Buffer()								= default;
			Buffer(Buffer const&) 					= delete;
			Buffer& operator=(Buffer const&) 		= delete;
			Buffer(Buffer&&) noexcept 				= delete;
			Buffer& operator=(Buffer&&) noexcept 	= delete;
			~Buffer();

			/**
			 * \return False when the buffer is safe to be written to from CPU. True when it might be used by the GPU.
			*/
			bool isUsedByGPU() const;

			void upload(void const* src, size_t size, size_t dstOffset = 0);
			
			void* mapMemory();

			void unmapMemory();

			bool isMemoryMapped() const { return memoryMapCount > 0; }

			VkBuffer getVkBuffer() const { return buffer; }

			size_t getSize() const { return size; }

			VkBufferUsageFlags getVkBufferUsage() const { return usage; }

			VmaMemoryUsage getVmaMemoryUsage() const { return memoryUsage; }

		private:
			friend class Device;
			friend class BufferHandle;
			friend class StagingBufferPool;
			friend class Queue;

			VkBuffer buffer = VK_NULL_HANDLE;
			size_t size = {};
			VkBufferUsageFlags usage = {};
			VmaMemoryUsage memoryUsage = {};
			VmaAllocation allocation = {};
			VmaAllocator allocator = {};
			u32 usesOnGPU = {};
			u32 memoryMapCount = 0;
		};

		class BufferHandle {
		public:
			BufferHandle(std::shared_ptr<Buffer> buffer)
				:buffer{ std::move(buffer) }
			{}
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
			friend class Queue;

			std::shared_ptr<Buffer> buffer = {};
		};
	}
}