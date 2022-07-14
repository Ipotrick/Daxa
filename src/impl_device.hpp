#pragma once

#include "impl_context.hpp"

#include <daxa/daxa.hpp>
#include <daxa/device.hpp>
#include "impl_gpu_resources.hpp"

namespace daxa
{
    struct ImplCommandList;
    struct ImplBinarySemaphore;

    struct ImplDevice
    {
        std::weak_ptr<ImplContext> impl_ctx = {};
        VkPhysicalDevice vk_physical_device = {};
        VkDevice vk_device_handle = {};
        VkQueue vk_main_queue_handle = {};
        u32 main_queue_family_index = {};
        DeviceVulkanInfo vk_info = {};
        DeviceInfo info = {};
        GPUResourceTable gpu_table;

        std::mutex zombie_command_lists_mtx = {};
        std::vector<std::shared_ptr<ImplCommandList>> zombie_command_lists = {};
        std::mutex zombie_binary_semaphores_mtx = {};
        std::vector<std::shared_ptr<ImplBinarySemaphore>> zombie_binary_semaphores = {};

        struct Submit
        {
            VkFence vk_fence_handle = {};
            std::vector<std::shared_ptr<ImplCommandList>> command_lists = {};
            std::vector<std::shared_ptr<ImplBinarySemaphore>> binary_semaphores = {};
        };
        std::mutex submit_mtx = {};
        std::vector<Submit> submits_pool = {};
        std::vector<Submit> command_list_submits = {};

        ImplDevice(DeviceInfo const & info, DeviceVulkanInfo const & vk_info, std::shared_ptr<ImplContext> impl_ctx, VkPhysicalDevice physical_device);
        ~ImplDevice();

        auto new_buffer() -> BufferId;
        void cleanup_buffer(BufferId id);
        auto slot(BufferId id) -> ImplBufferSlot const &;

        auto new_swapchain_image(VkImage swapchain_image, VkFormat format, u32 index, const std::string & debug_name) -> ImageId;
        auto new_image() -> ImageId;
        void cleanup_image(ImageId id);
        auto slot(ImageId id) -> ImplImageSlot const &;

        auto new_image_view() -> ImageViewId;
        void cleanup_image_view(ImageViewId id);
        auto slot(ImageViewId id) -> ImplImageViewSlot const &;

        auto new_sampler() -> SamplerId;
        void cleanup_sampler(SamplerId id);
        auto slot(SamplerId id) -> ImplSamplerSlot const &;
    };
} // namespace daxa
