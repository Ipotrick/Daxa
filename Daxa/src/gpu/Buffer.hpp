#pragma once

#include "../DaxaCore.hpp"

#include <memory>
#include <span>

#include "Bindless.hpp"
#include "Handle.hpp"
#include "Graveyard.hpp"

namespace daxa {
	class BufferBackend;

	enum class MemoryType {
		GPU_ONLY,
		CPU_ONLY,
		CPU_TO_GPU,
		GPU_TO_CPU
	};

	struct BufferCreateInfo {
		uz 				size				= {};
		MemoryType		memoryType 			= MemoryType::GPU_ONLY;
		char const* 	debugName 			= {};
	};

	class MappedMemoryPointer {
	public:
		MappedMemoryPointer(u8* hostPtr, size_t size, std::shared_ptr<BufferBackend> buffer)
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

		void unmap();

		u8* 	hostPtr = {};
		size_t 	size 	= {};
	private:
		friend class BufferHandle;
		friend class CommandListBackend;

		std::shared_ptr<BufferBackend> owningBuffer = {};
	};

	struct BufferStaticFunctionOverride {
		static void cleanup(std::shared_ptr<BufferBackend>& value);
	};



	class BufferHandle : public SharedHandle<BufferBackend, BufferStaticFunctionOverride> {
	public:
		MappedMemoryPointer mapMemory();
		const MappedMemoryPointer mapMemory() const;

		void* operator->() { return 0; }
		void const* operator->() const { return 0; }

        int operator*() { return 0; }
        int operator*() const { return 0; }

		void upload(void const* src, size_t size, size_t dstOffset = 0);

		bool isMemoryMapped() const;

		size_t getSize() const;

		void* getVkBuffer() const;

		MemoryType getMemoryType() const;

		u32 getMemeoryMapCount() const;

		DescriptorIndex getDescriptorIndex() const;

		std::string const& getDebugName() const;
	};
}