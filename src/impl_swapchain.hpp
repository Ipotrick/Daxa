#pragma once

#include <daxa/swapchain.hpp>

#include "impl_core.hpp"

namespace daxa
{
    struct ImplDevice;

    struct ImplSwapchain final : ManagedSharedState
    {
        ManagedWeakPtr impl_device = {};
        SwapchainInfo info = {};
        VkSwapchainKHR vk_swapchain = VK_NULL_HANDLE;
        VkSurfaceKHR vk_surface = {};
        VkSurfaceFormatKHR vk_surface_format = {};
        std::vector<ImageId> image_resources = {};

        u32 current_image_index;

        ImplSwapchain(ManagedWeakPtr device_impl, SwapchainInfo const & info);
        ~ImplSwapchain();

        void recreate();
        void cleanup();
        void recreate_surface();
    };
} // namespace daxa
