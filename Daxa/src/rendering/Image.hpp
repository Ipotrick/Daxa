#pragma once

#include "Vulkan.hpp"

#include <iostream>

namespace daxa { 
	class Image {
	public:
		Image() = default;

		Image(Image&& other) noexcept;

		Image& operator=(Image&& other) noexcept;

		~Image();

		void reset();

		bool valid() const;

		operator bool() const;

		vk::ImageViewCreateInfo viewInfo{};
		vk::ImageCreateInfo info{};
		vk::UniqueImageView view{};
		vk::Image image{};
		VmaAllocation allocation{};
		VmaAllocator allocator{};
	};

	Image loadImage2d(vk::CommandBuffer& cmd, vk::Fence fence, std::string& path, vk::Device device = VulkanGlobals::device, VmaAllocator allocator = VulkanGlobals::allocator);

	struct Image2dCreateInfo {
		// layout:
		vk::Format format{ vk::Format::eR8G8B8A8Srgb };
		vk::Extent2D size{1,1};
		u32 mipLevels{ 1 };
		// usage:
		vk::ImageUsageFlags usage;
		vk::ImageAspectFlags  viewImageAspectFlags{ vk::ImageAspectFlagBits::eColor };
		// memory settings:
		VmaMemoryUsage memoryUsage{ VMA_MEMORY_USAGE_GPU_ONLY };
		u32* familityQIndices{ nullptr };
		u32 familtyQIndcexCount{ 1 };
		vk::SharingMode familyQIndexSharingMode{ vk::SharingMode::eExclusive };
	};

	Image createImage2d(GPUContext& gpu, const Image2dCreateInfo& createInfo);

	void uploadImage2d(GPUContext& gpu, vk::CommandBuffer cmd, Image& image, const u8* data);

	void updateImageLayoutToShaderReadOnly(GPUContext& gpu, vk::CommandBuffer cmd, Image& image);
}
