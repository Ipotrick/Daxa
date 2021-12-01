#pragma once

#include "../DaxaCore.hpp"

#include "gpu.hpp"

namespace daxa {
	class StagingBufferPool {
	public:
		StagingBufferPool(gpu::Device* device, size_t bufferSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) 
			: device{ device }
			, bufferCI{ .size = bufferSize, .usage = usage, .memoryUsage = (VmaMemoryUsage)(memoryUsage | VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU) }
		{ }

		gpu::BufferHandle getStagingBuffer() {
			if (unusedBuffers.empty()) {
				for (auto iter = usedBuffers.begin(); iter != usedBuffers.end();) {
					if (iter->getRefCount() == 1) {
						unusedBuffers.push_back(std::move(*iter));
						iter = usedBuffers.erase(iter);
					}
					else {
						++iter;
					}
				}
			}
			if (unusedBuffers.empty()) {
				unusedBuffers.push_back(device->createBuffer(bufferCI));
			}
			auto buffer = std::move(unusedBuffers.back());
			unusedBuffers.pop_back();
			return std::move(buffer);
		}

	private:
		gpu::Device* device = nullptr;
		gpu::BufferCreateInfo bufferCI = {};
		std::vector<gpu::BufferHandle> unusedBuffers;
		std::vector<gpu::BufferHandle> usedBuffers;
	};
}
