#pragma once

#include "../DaxaCore.hpp"

#include <memory>
#include <span>

#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>

namespace daxa {
	namespace gpu {
		struct BufferCreateInfo {
			uz 						size				= {};
			VkBufferUsageFlags 		usage				= {};
			VmaMemoryUsage 			memoryUsage			= {};
			VkMemoryPropertyFlags 	memoryProperties 	= {};
			char const* 			debugName 			= {};
		};

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
			
			bool isMemoryMapped() const { return memoryMapCount > 0; }

			VkBuffer getVkBuffer() const { return buffer; }

			size_t getSize() const { return size; }

			VkBufferUsageFlags getVkBufferUsage() const { return usage; }

			VmaMemoryUsage getVmaMemoryUsage() const { return memoryUsage; }

			std::string const& getDebugName() const { return debugName; }
		private:
			friend class Device;
			friend class BufferHandle;
			friend class StagingBufferPool;
			friend class Queue;
			template<typename ValueT>
			friend struct MappedMemoryPointer;
			
			void unmapMemory();

			VkBuffer 			buffer 			= VK_NULL_HANDLE;
			size_t 				size 			= {};
			VkBufferUsageFlags 	usage 			= {};
			VmaMemoryUsage 		memoryUsage 	= {};
			VmaAllocation 		allocation 		= {};
			VmaAllocator 		allocator 		= {};
			u32 				usesOnGPU 		= {};
			u32 				memoryMapCount 	= {};
			std::string 		debugName 		= {};
		};

		template<typename ValueT>
		class MappedMemoryPointer {
		public:
			ValueT *hostPtr;
			size_t size;
		private:
			friend class BufferHandle;
			friend class CommandList;
			std::shared_ptr<Buffer> owningBuffer;

		public:
			MappedMemoryPointer(ValueT* hostPtr, size_t size, std::shared_ptr<Buffer> buffer)
				: hostPtr{ hostPtr }
				, size{size}
				, owningBuffer{ std::move(buffer) }
			{}
			MappedMemoryPointer(const MappedMemoryPointer &) = delete;
			MappedMemoryPointer & operator=(const MappedMemoryPointer &) = delete;
			MappedMemoryPointer(MappedMemoryPointer &&) = default;
			MappedMemoryPointer & operator=(MappedMemoryPointer &&) = default;
			~MappedMemoryPointer();
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
		
			template<typename ValueT = u8>
			MappedMemoryPointer<ValueT> mapMemory() {
				auto ret = mapMemoryVoid();
				return {static_cast<ValueT*>(ret.hostPtr), ret.size, ret.owningBuffer};
			}
		private:
			friend class Device;
			friend class StagingBufferPool;
			friend class Queue;

			MappedMemoryPointer<void> mapMemoryVoid();

			std::shared_ptr<Buffer> buffer = {};
		};
	
		template<typename ValueT>
		MappedMemoryPointer<ValueT>::~MappedMemoryPointer() {
			if (owningBuffer)
				owningBuffer->unmapMemory();
		}
	}
}