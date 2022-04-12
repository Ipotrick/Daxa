#pragma once

#include "../DaxaCore.hpp"

#include <mutex>
#include <vector>
#include <optional>

#include <vulkan/vulkan.h>

#include "Buffer.hpp"

namespace daxa {
	constexpr inline size_t STAGING_BUFFER_POOL_BUFFER_SIZE = 1 << 26;	// 67 MB, about 4k texture size

	namespace {
		struct StagingBufferPoolSharedData {
			std::mutex mut = {};
			std::vector<BufferHandle> pool = {};
		};
	}

	class StagingBuffer {
	public:
		StagingBuffer(BufferHandle& handle, std::weak_ptr<StagingBufferPoolSharedData> pool);
		StagingBuffer(StagingBuffer&&) noexcept													= default;
		StagingBuffer(StagingBuffer const&) 													= delete;
		StagingBuffer& operator=(StagingBuffer&&) noexcept;
		StagingBuffer& operator=(StagingBuffer const&) 											= delete;
		~StagingBuffer();

		size_t getLeftOverSize() const { return STAGING_BUFFER_POOL_BUFFER_SIZE - usedUpSize; }

		size_t usedUpSize = {};
		std::optional<BufferHandle> buffer = {};
	private:
		void cleanup();
		std::weak_ptr<StagingBufferPoolSharedData> sharedData = {};
	};

	class StagingBufferPool {
	public:
		StagingBufferPool(
			std::shared_ptr<DeviceBackend> deviceBackend, 
			size_t size = STAGING_BUFFER_POOL_BUFFER_SIZE, 
			//VkBufferUsageFlags usages = VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			//memoryType = VMA_MEMORY_USAGE_CPU_TO_GPU
			MemoryType memoryType = MemoryType::CPU_TO_GPU
		);

		StagingBuffer getStagingBuffer();
	private:
		std::shared_ptr<DeviceBackend> deviceBackend = {};
		std::shared_ptr<StagingBufferPoolSharedData> sharedData = {};
		size_t size;
		//VkBufferUsageFlags usages; 
		//VmaMemoryUsage memoryUsages;
		MemoryType memoryType;
	};
}