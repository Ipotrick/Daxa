#pragma once

#include "Vulkan.hpp"

#include <iostream>

namespace daxa { 
	class Image {
	public:
		Image() : info{}, image{}, allocation{}, allocator{} {}

		Image(Image&& other) noexcept;

		Image& operator=(Image&& other) noexcept;

		~Image();

		void reset();

		bool valid() const;

		vk::ImageViewCreateInfo viewInfo;
		vk::ImageCreateInfo info;
		vk::UniqueImageView view;
		vk::Image image;
		VmaAllocation allocation;
		VmaAllocator allocator;
	};

	Image loadImage(vk::CommandBuffer& cmd, vk::Fence fence, std::string& path, vk::Device device = vkh_old::device, VmaAllocator allocator = vkh_old::allocator);

	Image makeImage(
		const vk::ImageCreateInfo& createInfo,
		const vk::ImageViewCreateInfo viewCreateInfo,
		const VmaAllocationCreateInfo& allocInfo = {
			.usage = VMA_MEMORY_USAGE_GPU_ONLY,
			.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
		},
		vk::Device device = vkh_old::device,
		VmaAllocator allocator = vkh_old::allocator
	);
}
