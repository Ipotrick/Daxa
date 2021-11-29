#pragma once

#include <memory>

#include <deque>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "../dependencies/vk_mem_alloc.hpp"
#include "../dependencies/vulkanhelper.hpp"

#include "../../DaxaCore.hpp"

#include "Image.hpp"

namespace daxa {
	namespace gpu {

		class SwapchainImage {
		public:
			u32 getImageIndex() const { return imageIndex; }
			vk::SwapchainKHR getVkSwapchain() const { return swapchain; }
			ImageHandle& getImageHandle() { return image; }
			ImageHandle const& getImageHandle() const { return image; }
		private:
			friend class Device;
			friend class RenderWindow;

			ImageHandle image;
			u32 imageIndex;
			vk::SwapchainKHR swapchain;
		};

	}
}