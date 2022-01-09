#include "StagingBufferPool.hpp"

namespace daxa {
	namespace gpu {

		StagingBuffer::StagingBuffer(BufferHandle& handle, std::weak_ptr<StagingBufferPoolSharedData> pool)
			: buffer{ handle }
			, sharedData{ std::move(pool) }
		{ }
		
		void StagingBuffer::cleanup() {
			if (buffer) {
				auto data = sharedData.lock();
				if (data) {
					auto lock = std::unique_lock(data->mut);
					data->pool.push_back(std::move(buffer));
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

		StagingBufferPool::StagingBufferPool(VkDevice device, u32 queueFamilyIndex, VmaAllocator allocator)
			: device{ device }
			, queueFamilyIndex{ queueFamilyIndex }
			, allocator{ allocator }
			, sharedData{ std::make_shared<StagingBufferPoolSharedData>() }
		{ }

		StagingBuffer StagingBufferPool::getStagingBuffer() {
			auto lock = std::unique_lock(sharedData->mut);

			if (sharedData->pool.empty()) {
				BufferCreateInfo bufferCI{
					.size = STAGING_BUFFER_POOL_BUFFER_SIZE,
					.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU,
				};

				sharedData->pool.push_back(BufferHandle{ std::make_shared<Buffer>(device, queueFamilyIndex, allocator, bufferCI) });
			}

			auto stagingBuffer = StagingBuffer{ sharedData->pool.back(), sharedData };
			sharedData->pool.pop_back();
			return stagingBuffer;
		}
	}
}