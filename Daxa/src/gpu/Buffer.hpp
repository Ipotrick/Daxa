#pragma once

#include "../DaxaCore.hpp"

#include <memory>
#include <span>

#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>

#include "Handle.hpp"
#include "DeviceBackend.hpp"
#include "DeviceBackend.hpp"

namespace daxa {
	struct BufferCreateInfo {
		uz 						size				= {};
		VkBufferUsageFlags 		usage				= {};
		VmaMemoryUsage 			memoryUsage			= {};
		VkMemoryPropertyFlags 	memoryProperties 	= {};
		char const* 			debugName 			= {};
	};

	class Buffer : public GraveyardRessource {
	public:
		Buffer(std::shared_ptr<DeviceBackend> deviceBackend, BufferCreateInfo& ci);
		Buffer()								= default;
		Buffer(Buffer const&) 					= delete;
		Buffer& operator=(Buffer const&) 		= delete;
		Buffer(Buffer&&) noexcept 				= delete;
		Buffer& operator=(Buffer&&) noexcept 	= delete;
		virtual ~Buffer();

		void upload(void const* src, size_t size, size_t dstOffset = 0);
		
		bool isMemoryMapped() const { return memoryMapCount > 0; }

		VkBuffer getVkBuffer() const { return buffer; }

		size_t getSize() const { return size; }

		VkBufferUsageFlags getVkBufferUsage() const { return usage; }

		VmaMemoryUsage getVmaMemoryUsage() const { return memoryUsage; }

		u32 getMemeoryMapCount() const { return memoryMapCount; }

		u32 getDescriptorIndex() const { 
			return descriptorIndex;
		}

		std::string const& getDebugName() const { return debugName; }
	private:
		friend class Device;
		friend class BufferHandle;
		friend class StagingBufferPool;
		friend class Queue;
		template<typename ValueT>
		friend struct MappedMemoryPointer;
		friend struct BufferStaticFunctionOverride;
		
		void* mapMemory();
		void unmapMemory();

		std::shared_ptr<DeviceBackend> 	deviceBackend 	= {};
		VkBuffer 						buffer 			= {};
		size_t 							size 			= {};
		VkBufferUsageFlags 				usage 			= {};
		VmaMemoryUsage 					memoryUsage 	= {};
		VmaAllocation 					allocation 		= {};
		u32 							memoryMapCount 	= {};
		std::string 					debugName 		= {};
		u32 							descriptorIndex = {};
	};

	template<typename ValueT = u8>
	class MappedMemoryPointer {
	public:
		MappedMemoryPointer(ValueT* hostPtr, size_t size, std::shared_ptr<Buffer> buffer)
			: hostPtr{ hostPtr }
			, size{ size }
			, owningBuffer{ std::move(buffer) }
		{}
		MappedMemoryPointer(MappedMemoryPointer const&) 			= delete;
		MappedMemoryPointer& operator=(MappedMemoryPointer const&) 	= delete;
		MappedMemoryPointer(MappedMemoryPointer&&) 					= default;
		MappedMemoryPointer& operator=(MappedMemoryPointer&&) 		= default;
		~MappedMemoryPointer() {
			if (owningBuffer) {
				unmap();
			}
		}

		void unmap() {
			DAXA_ASSERT_M(owningBuffer, "can only unmap a valid memory mapped ptr");
			owningBuffer->unmapMemory();
			owningBuffer = {};
			hostPtr = std::numeric_limits<ValueT*>::max();
			size 	= std::numeric_limits<size_t>::max();
		}

		ValueT* hostPtr = std::numeric_limits<ValueT*>::max();
		size_t 	size 	= std::numeric_limits<size_t>::max();
	private:
		friend class BufferHandle;
		friend class CommandList;

		std::shared_ptr<Buffer> owningBuffer = {};
	};

	struct BufferStaticFunctionOverride {
		static void cleanup(std::shared_ptr<Buffer>& value) {
			if (value && value.use_count() == 1) {
				std::unique_lock lock(value->deviceBackend->graveyard.mtx);
				for (auto& zombieList : value->deviceBackend->graveyard.activeZombieLists) {
					zombieList->zombies.push_back(value);
				}
			}
		}
	};

	class BufferHandle : public SharedHandle<Buffer, BufferStaticFunctionOverride> {
	public:
		template<typename T = u8>
		MappedMemoryPointer<T> mapMemory() {
			void* ptr = value->mapMemory();
			return std::move(MappedMemoryPointer<T>{ static_cast<T*>(ptr), value->getSize(), value });
		}
	};
}