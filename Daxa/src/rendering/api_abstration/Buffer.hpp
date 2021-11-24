#pragma once

#include <memory>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "../dependencies/vk_mem_alloc.hpp"
#include "../dependencies/vulkanhelper.hpp"

#include "../../DaxaCore.hpp"

namespace daxa {
	namespace gpu {
		struct BufferCreateInfo {
			uz size;
			vk::BufferUsageFlags usage;
			VmaMemoryUsage memoryUsage;
		};

		class Buffer {
		public:
			Buffer();
			Buffer(Buffer&&) noexcept;
			Buffer& operator=(Buffer&&) noexcept;

			~Buffer();

			vk::Buffer getVkBuffer() const { return buffer; }
		private:
			Buffer(vk::Device device, u32 queueFamilyIndex, VmaAllocator allocator, BufferCreateInfo ci);
			friend class Device;
			friend class BufferHandle;
			vk::Buffer buffer;
			VmaAllocation allocation;
			VmaAllocator allocator;
		};

		class BufferHandle {
		public:
			Buffer& operator*() { return *buffer; }
			Buffer* operator->() { return buffer.get(); }
		private:
			friend class Device;
			BufferHandle(std::shared_ptr<Buffer> buffer);
			std::shared_ptr<Buffer> buffer;
		};
	}
}