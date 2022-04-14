#include "Buffer.hpp"
#include "Instance.hpp"
#include "backend/DeviceBackend.hpp"
#include "backend/BufferBackend.hpp"

namespace daxa {

	MappedMemoryPointer BufferHandle::mapMemory() {
		u8* ptr = static_cast<u8*>(value->mapMemoryRaw());
		return std::move(MappedMemoryPointer{ ptr, value->getSize(), value });
	}
	
	const MappedMemoryPointer BufferHandle::mapMemory() const {
		u8* ptr = static_cast<u8*>(value->mapMemoryRaw());
		return std::move(MappedMemoryPointer{ ptr, value->getSize(), value });
	}

	void MappedMemoryPointer::unmap() {
		DAXA_ASSERT_M(owningBuffer, "can only unmap a valid memory mapped ptr");
		owningBuffer->unmapMemoryRaw();
		owningBuffer = {};
		hostPtr = nullptr;
		size 	= 0;
	}
	
	void BufferStaticFunctionOverride::cleanup(std::shared_ptr<BufferBackend>& value) {
		if (value && value.use_count() == 1) {
			std::unique_lock lock(value->deviceBackend->graveyard.mtx);
			for (auto& zombieList : value->deviceBackend->graveyard.activeZombieLists) {
				zombieList->zombies.push_back(value);
			}
		}
	}
	

	void BufferHandle::upload(void const* src, size_t size, size_t dstOffset) {
		value->upload(src, size, dstOffset);
	}

	bool BufferHandle::isMemoryMapped() const { 
		return value->isMemoryMapped();
	}

	size_t BufferHandle::getSize() const { 
		return value->size;
	}

	u32 BufferHandle::getMemeoryMapCount() const { 
		return value->memoryMapCount;
	}

	void* BufferHandle::getVkBuffer() const {
		return value->buffer;
	}

	u32 BufferHandle::getDescriptorIndex() const { 
		return value->descriptorIndex;
	}

	std::string const& BufferHandle::getDebugName() const { 
		return value->debugName;
	}

	MemoryType BufferHandle::getMemoryType() const {
		return value->memoryType;
	}
}
