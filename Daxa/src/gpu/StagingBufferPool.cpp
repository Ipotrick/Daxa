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
				data->pool.push_back(*buffer);
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
		std::shared_ptr<void> deviceBackend, 
		size_t size, 
		MemoryType memoryType
	)
		: deviceBackend{ std::move(deviceBackend) }
		, sharedData{ std::make_shared<StagingBufferPoolSharedData>() }
		, memoryType{ memoryType }
		, size{ size }
	{ }

	StagingBuffer StagingBufferPool::getStagingBuffer() {
		auto lock = std::unique_lock(sharedData->mut);

		if (sharedData->pool.empty()) {
			BufferInfo bufferCI{
				.size = size,
				.memoryType = memoryType
			};

			if (instance->pfnSetDebugUtilsObjectNameEXT) {
				bufferCI.debugName = "staging buffer";
			}

			DeviceBackend& backend = *reinterpret_cast<DeviceBackend*>(this->deviceBackend.get());

			sharedData->pool.push_back( backend.createBuffer(bufferCI) );
		}

		auto stagingBuffer = StagingBuffer{ sharedData->pool.back(), sharedData };
		sharedData->pool.pop_back();
		return stagingBuffer;
	}

	StagingBufferPool::~StagingBufferPool() {
		for (auto buffer: sharedData->pool) {
			DeviceBackend& backend = *reinterpret_cast<DeviceBackend*>(this->deviceBackend.get());
			backend.gpuHandleGraveyard.zombifyBuffer(backend.gpuRessources, buffer);
		}
	}
}