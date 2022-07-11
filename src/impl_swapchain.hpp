#pragma once

#include <daxa/swapchain.hpp>
#include "impl_core.hpp"
#include "impl_device.hpp"

namespace daxa
{
    struct SwapchainImageResources
    {
        VkImage vk_image;
        VkImageView vk_image_view;
    };

    struct ImplSwapchain
    {
        std::shared_ptr<ImplDevice> impl_device = {};
        SwapchainInfo info = {};
        VkSwapchainKHR vk_swapchain_handle = {};
        VkSurfaceKHR vk_surface_handle = {};
        VkSurfaceFormatKHR vk_surface_format = {};
        std::vector<SwapchainImageResources> image_resources = {};

        ImplSwapchain(std::shared_ptr<ImplDevice> device_impl, SwapchainInfo const & info);
        ~ImplSwapchain();

        void recreate(SwapchainInfo const & info);
        void cleanup();
    };
} // namespace daxa