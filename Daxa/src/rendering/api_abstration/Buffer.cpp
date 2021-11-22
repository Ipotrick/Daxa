#include "Buffer.hpp"

namespace gpu {

	Buffer::Buffer(vk::Device device, u32 queueFamilyIndex, VmaAllocator allocator, BufferCreateInfo ci) {

		vk::BufferCreateInfo bci{};
		bci.size = ci.size;
		bci.usage = ci.usage;
		bci.pQueueFamilyIndices = &queueFamilyIndex;
		bci.queueFamilyIndexCount = 1;

		VmaAllocationCreateInfo aci{};
		aci.usage = ci.memoryUsage;

		vmaCreateBuffer(allocator, (VkBufferCreateInfo*)&bci, &aci, (VkBuffer*)&*buffer, &allocation, nullptr);

		allocator = allocator;
	}

	Buffer::Buffer() {
		std::memset(this, 0, sizeof(buffer));
	}

	Buffer::Buffer(Buffer&& rhs) noexcept  {
		std::memcpy(this, &rhs, sizeof(Buffer));
		std::memset(&rhs, 0, sizeof(buffer));
	}

	Buffer& Buffer::operator=(Buffer&& rhs) noexcept {
		std::memcpy(this, &rhs, sizeof(Buffer));
		std::memset(&rhs, 0, sizeof(buffer));
		return *this;
	}

	Buffer::~Buffer() {
		if (allocator) {
			vmaDestroyBuffer(allocator, buffer, allocation);
			allocator = nullptr;
			buffer = vk::Buffer{};
			allocation = VmaAllocation{};
		}
	}

	BufferHandle::BufferHandle(std::shared_ptr<Buffer> buffer) {
		this->buffer = buffer;
	}
}
