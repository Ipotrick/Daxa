#pragma once

#include "../Vulkan.hpp"

#include <iostream>

namespace daxa { 
	namespace vkh {
		class Image {
		public:
			Image() : info{}, image{}, allocation{}, allocator{} {}

			Image(Image&& other) noexcept;

			Image& operator=(Image&& other) noexcept;

			~Image();

			void reset();

			bool valid() const;

			vk::ImageCreateInfo info;
			vk::Image image;
			VmaAllocation allocation;
			VmaAllocator allocator;
		};

		Image loadImage(vk::CommandBuffer& cmd, vk::Fence fence, std::string& path, vk::Device device = vkh::device, VmaAllocator allocator = vkh::allocator);

		Image makeImage(
			const vk::ImageCreateInfo& createInfo,
			const VmaAllocationCreateInfo& allocInfo = {
				.usage = VMA_MEMORY_USAGE_GPU_ONLY,
				.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
			},
			VmaAllocator allocator = vkh::allocator
		);
	}
}
