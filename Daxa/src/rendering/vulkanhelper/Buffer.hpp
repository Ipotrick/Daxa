#pragma once

#include "../Vulkan.hpp"

#include <iostream>

namespace daxa {
	namespace vkh {
		struct Buffer {
			Buffer() = default;

			Buffer(Buffer&& other) noexcept
			{
				std::swap(this->buffer, other.buffer);
				std::swap(this->allocation, other.allocation);
				other.buffer = vk::Buffer{};
				other.allocation = {};
			}

			Buffer& operator=(Buffer&& other)
			{
				Buffer::~Buffer();
				new(this) Buffer(std::move(other));
				return *this;
			}

			~Buffer()
			{
				if (buffer) {
					vmaDestroyBuffer(vkh::allocator, buffer, allocation);
				}
			}

			vk::Buffer buffer;
			VmaAllocation allocation;
		};

		inline Buffer createBuffer(uz allocSize, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocator allocator = vkh::allocator)
		{
			//allocate vertex buffer
			vk::BufferCreateInfo bufferInfo{
				.size = allocSize,
				.usage = usage,
			};

			VmaAllocationCreateInfo vmaallocInfo = {};
			vmaallocInfo.usage = memoryUsage;

			Buffer newBuffer;

			//allocate the buffer
			VK_CHECK(vmaCreateBuffer(allocator, (VkBufferCreateInfo*)&bufferInfo, &vmaallocInfo,
				(VkBuffer*)&newBuffer.buffer,
				&newBuffer.allocation,
				nullptr));

			return newBuffer;
		}
	}
}
