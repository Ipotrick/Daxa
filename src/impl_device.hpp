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
        DeviceVulkanInfo vk_info = {};
        DeviceInfo info = {};

        // Gpu resource table:
        GPUResourceTable gpu_table = {};

        // Zombie recycling:
        std::mutex zombie_command_lists_mtx = {};
        std::vector<std::shared_ptr<ImplCommandList>> zombie_command_lists = {};
        std::mutex zombie_binary_semaphores_mtx = {};
        std::vector<std::shared_ptr<ImplBinarySemaphore>> zombie_binary_semaphores = {};

        // Submits:
        struct Submit
        {
            VkFence vk_fence_handle = {};
            std::vector<std::shared_ptr<ImplCommandList>> command_lists = {};
            std::vector<std::shared_ptr<ImplBinarySemaphore>> binary_semaphores = {};
        };
        std::mutex submit_mtx = {};
        std::vector<Submit> submits_pool = {};
        std::vector<Submit> command_list_submits = {};

        // Main queue:
        VkQueue vk_main_queue_handle = {};
        u32 main_queue_family_index = {};
        std::atomic_uint64_t main_queue_cpu_timeline = {};
        VkSemaphore vk_main_queue_gpu_timeline_semaphore_handle = {};
        std::mutex main_queue_zombies_mtx = {};
        std::vector<std::pair<u64, BufferId>> main_queue_buffer_zombies = {};
        std::vector<std::pair<u64, ImageId>> main_queue_image_zombies = {};
        std::vector<std::pair<u64, ImageViewId>> main_queue_image_view_zombies = {};
        std::vector<std::pair<u64, SamplerId>> main_queue_sampler_zombies = {};

        ImplDevice(DeviceInfo const & info, DeviceVulkanInfo const & vk_info, std::shared_ptr<ImplContext> impl_ctx, VkPhysicalDevice physical_device);
        ~ImplDevice();

        void collect_garbage_no_lock();
        void main_queue_collect_garbage();

        auto new_buffer() -> BufferId;
        auto new_swapchain_image(VkImage swapchain_image, VkFormat format, u32 index, const std::string & debug_name) -> ImageId;
        auto new_image() -> ImageId;
        auto new_image_view() -> ImageViewId;
        auto new_sampler() -> SamplerId;

        auto slot(BufferId id) -> ImplBufferSlot &;
        auto slot(ImageId id) -> ImplImageSlot &;
        auto slot(ImageViewId id) -> ImplImageViewSlot &;
        auto slot(SamplerId id) -> ImplSamplerSlot &;

        void zombiefy_buffer(BufferId id);
        void zombiefy_image(ImageId id);
        void zombiefy_image_view(ImageViewId id);
        void zombiefy_sampler(SamplerId id);
        
        void cleanup_buffer(BufferId id);
        void cleanup_image(ImageId id);
        void cleanup_image_view(ImageViewId id);
        void cleanup_sampler(SamplerId id);
    };
} // namespace daxa
