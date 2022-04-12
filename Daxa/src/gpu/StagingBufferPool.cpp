#include "StagingBufferPool.hpp"
#include "Instance.hpp"
#include "backend/DeviceBackend.hpp"
#include "backend/BufferBackend.hpp"

namespace daxa {
	StagingBuffer::StagingBuffer(BufferHandle& handle, std::weak_ptr<StagingBufferPoolSharedData> pool)
		: buffer{ handle }
		, sharedData{ std::move(pool) }
	{ }
	
	void StagingBuffer::cleanup() {
		if (buffer) {
			auto data = sharedData.lock();
			if (data) {
				auto lock = std::unique_lock(data->mut);
				data->pool.push_back(std::move(*buffer));
			}
			buffer = {};
		}
	}

	StagingBuffer& StagingBuffer::operator=(StagingBuffer&& other) noexcept {
		cleanup();
		this->buffer = std::move(other.buffer);
		this->sharedData = std::move(other.sharedData);
		this->usedUpSize = std::move(other.usedUpSize);
		return *this;
	}

	StagingBuffer::~StagingBuffer() {
		cleanup();
	}

	StagingBufferPool::StagingBufferPool(
		std::shared_ptr<DeviceBackend> deviceBackend, 
		size_t size, 
		//VkBufferUsageFlags usages, 
		MemoryType memoryType
	)
		: deviceBackend{ std::move(deviceBackend) }
		, sharedData{ std::make_shared<StagingBufferPoolSharedData>() }
		//, usages{ usages }
		//, memoryUsages{ memoryUsages }
		, memoryType{ memoryType }
		, size{ size }
	{ }

	StagingBuffer StagingBufferPool::getStagingBuffer() {
		auto lock = std::unique_lock(sharedData->mut);

		if (sharedData->pool.empty()) {
			BufferCreateInfo bufferCI{
				.size = size,
				.memoryType = memoryType
				//.usage = usages,
				//.memoryUsage = memoryUsages,
			};

			if (instance->pfnSetDebugUtilsObjectNameEXT) {
				bufferCI.debugName = "staging buffer";
			}

			sharedData->pool.push_back(BufferHandle{ std::make_shared<BufferBackend>(deviceBackend, bufferCI)});
		}

		auto stagingBuffer = StagingBuffer{ sharedData->pool.back(), sharedData };
		sharedData->pool.pop_back();
		return stagingBuffer;
	}
}