#pragma once

#include <memory>
#include <vector>
#include <deque>
#include <span>

#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>

#include "../DaxaCore.hpp"

#include "Image.hpp"
#include "SwapchainImage.hpp"
#include "Signal.hpp"

namespace daxa {
	namespace gpu {

		VkSurfaceKHR createSurface(void* sdlWindowHandle, VkInstance instance);

		class Swapchain {
		public:
			Swapchain() 								= default;
			Swapchain(Swapchain&&) noexcept;
			Swapchain& operator=(Swapchain&&) noexcept;
			Swapchain(Swapchain const&) 				= delete;
			Swapchain& operator=(Swapchain const&) 		= delete;
			~Swapchain();

			void resize(VkExtent2D newSize);

			void setPresentMode(VkPresentModeKHR newPresentMode);

			SwapchainImage aquireNextImage();

			VkExtent2D getSize() const { return size; }

			VkFormat getVkFormat() const { return swapchainImageFormat; }
		private:
			friend class Device;

			void construct(VkDevice device, VkPhysicalDevice physicalDevice, VkInstance instance, VkSurfaceKHR surface, u32 width, u32 height, VkPresentModeKHR presentmode);

			VkDevice device 							= VK_NULL_HANDLE;
			VkPhysicalDevice physicalDevice 			= VK_NULL_HANDLE;
			VkInstance instance 						= VK_NULL_HANDLE;
			VkFence aquireFence 						= VK_NULL_HANDLE;
			VkPresentModeKHR presentMode 				= VK_PRESENT_MODE_FIFO_KHR;
			VkSurfaceKHR surface 						= VK_NULL_HANDLE;
			VkSwapchainKHR swapchain 					= VK_NULL_HANDLE; 
			VkFormat swapchainImageFormat 				= {};
			std::vector<ImageHandle> swapchainImages	= {}; // TODO REPLACE WITH STACK ALLOCATED VECTOR TYPE
			VkExtent2D size 							= {};
		};

		class SwapchainHandle {
		public:
			SwapchainHandle(std::shared_ptr<Swapchain> swapchain) 
				: swapchain{ std::move(swapchain) }
			{}
			SwapchainHandle() = default;

			Swapchain const& operator*() const { return *swapchain; }
			Swapchain& operator*() { return *swapchain; }
			Swapchain const* operator->() const { return swapchain.get(); }
			Swapchain* operator->() { return swapchain.get(); }

			size_t getRefCount() const { return swapchain.use_count(); }

			operator bool() const { return swapchain.operator bool(); }
			bool operator!() const { return !swapchain; }
			bool valid() const { return *this; }
		private:
			std::shared_ptr<Swapchain> swapchain = {};
		};
	}
}
