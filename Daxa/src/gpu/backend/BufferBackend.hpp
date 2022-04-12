#include "../../DaxaCore.hpp"

#include <memory>
#include <span>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "DeviceBackend.hpp"
#include "../Buffer.hpp"

namespace daxa {

	class BufferBackend : public GraveyardRessource {
	public:
		BufferBackend(std::shared_ptr<DeviceBackend> deviceBackend, BufferCreateInfo& ci);
		BufferBackend()								= default;
		BufferBackend(BufferBackend const&) 					= delete;
		BufferBackend& operator=(BufferBackend const&) 		= delete;
		BufferBackend(BufferBackend&&) noexcept 				= delete;
		BufferBackend& operator=(BufferBackend&&) noexcept 	= delete;
		virtual ~BufferBackend();

		void upload(void const* src, size_t size, size_t dstOffset = 0);
		
		bool isMemoryMapped() const { return memoryMapCount > 0; }

		VkBuffer getVkBuffer() const { return buffer; }

		size_t getSize() const { return size; }

		//VkBufferUsageFlags getVkBufferUsage() const { return usage; }

		//VmaMemoryUsage getVmaMemoryUsage() const { return memoryUsage; }



		u32 getMemeoryMapCount() const { return memoryMapCount; }

		//u32 getDescriptorIndex() const { 
		//	return descriptorIndex;
		//}

		std::string const& getDebugName() const { return debugName; }
	private:
		friend class Device;
		friend class BufferHandle;
		friend class StagingBufferPool;
		friend class Queue;
		friend class MappedMemoryPointer;
		friend struct BufferStaticFunctionOverride;
		
		void* mapMemoryRaw();
		void unmapMemoryRaw();

		std::shared_ptr<DeviceBackend> 	deviceBackend 	= {};
		VkBuffer 						buffer 			= {};
		size_t 							size 			= {};
        MemoryType                memoryType      = {};
		VmaAllocation 					allocation 		= {};
		u32 							memoryMapCount 	= {};
		std::string 					debugName 		= {};
		u32 							descriptorIndex = {};
	};
}