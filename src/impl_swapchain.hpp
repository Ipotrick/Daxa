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
        std::vector<ImageId> images = {};
        std::vector<BinarySemaphore> acquire_semaphores = {};
        std::vector<BinarySemaphore> present_semaphores = {};
        // Monotonically increasing frame index.
        usize cpu_frame_timeline = {};
        // cpu_frame_timeline % frames in flight. used to index the acquire semaphores.
        usize acquire_semaphore_index = {};
        // Gpu timeline semaphore used to track how far behind the gpu is.
        // Used to limit frames in flight.
        TimelineSemaphore gpu_frame_timeline;
        // This is the swapchain image index that acquire returns. THis is not necessarily linear.
        // This index must be used for present semaphores as they are paired to the images.
        u32 current_image_index = {};
        usize frames_in_flight = {};

        auto get_index_of_image(ImageId image) const -> usize;

        ImplSwapchain(ManagedWeakPtr device_impl, SwapchainInfo info);
        ~ImplSwapchain();

        void recreate();
        void cleanup();
        void recreate_surface();
    };
} // namespace daxa
