#pragma once

#include <daxa/swapchain.hpp>
#include "impl_core.hpp"
#include "impl_device.hpp"

namespace daxa
{
    struct ImplSwapchain
    {
        std::shared_ptr<ImplDevice> impl_device = {};
        SwapchainInfo info = {};
        VkSwapchainKHR vk_swapchain_handle = VK_NULL_HANDLE;
        VkSurfaceKHR vk_surface_handle = {};
        VkSurfaceFormatKHR vk_surface_format = {};
        std::vector<ImageId> image_resources = {};

        ImplSwapchain(std::shared_ptr<ImplDevice> device_impl, SwapchainInfo const & info);
        ~ImplSwapchain();

        void recreate(SwapchainInfo const & info);
        void cleanup();
    };
} // namespace daxa
