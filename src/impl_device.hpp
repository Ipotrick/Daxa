#pragma once

#include <daxa/daxa.hpp>
#include <daxa/device.hpp>

#include "impl_context.hpp"
#include "impl_recyclable_list.hpp"

#include "impl_pipeline.hpp"
#include "impl_command_list.hpp"
#include "impl_swapchain.hpp"
#include "impl_semaphore.hpp"
#include "impl_gpu_resources.hpp"

namespace daxa
{
    struct ImplDevice
    {
        std::weak_ptr<ImplContext> impl_ctx = {};
        VkPhysicalDevice vk_physical_device = {};
        VkDevice vk_device_handle = {};
        DeviceVulkanInfo vk_info = {};
        DeviceInfo info = {};

        // Gpu resource table:
        GPUResourceTable gpu_table = {};

        // Resource recycling:
        RecyclableList<ImplCommandList> command_list_recyclable_list = {};
        RecyclableList<ImplBinarySemaphore> binary_semaphore_recyclable_list = {};

        // Submits:
        struct Submit
        {
            u64 timeline_value = {};
            std::vector<std::shared_ptr<ImplCommandList>> command_lists = {};
        };
        std::mutex submit_mtx = {};
        std::vector<Submit> submits_pool = {};

        // Main queue:
        std::vector<Submit> main_queue_command_list_submits = {};
        VkQueue main_queue_vk_queue_handle = {};
        u32 main_queue_family_index = {};
        std::atomic_uint64_t main_queue_cpu_timeline = {};
        VkSemaphore vk_main_queue_gpu_timeline_semaphore_handle = {};
        std::mutex main_queue_zombies_mtx = {};
        std::vector<std::pair<u64, BufferId>> main_queue_buffer_zombies = {};
        std::vector<std::pair<u64, ImageId>> main_queue_image_zombies = {};
        std::vector<std::pair<u64, ImageViewId>> main_queue_image_view_zombies = {};
        std::vector<std::pair<u64, SamplerId>> main_queue_sampler_zombies = {};
        std::vector<std::pair<u64, std::shared_ptr<ImplBinarySemaphore>>> main_queue_binary_semaphore_zombies = {};
        std::vector<std::pair<u64, std::shared_ptr<ImplComputePipeline>>> main_queue_compute_pipeline_zombies = {};
        void main_queue_housekeeping_api_handles_no_lock();
        void main_queue_clean_dead_zombies();

        ImplDevice(DeviceInfo const & info, DeviceVulkanInfo const & vk_info, std::shared_ptr<ImplContext> impl_ctx, VkPhysicalDevice physical_device);
        ~ImplDevice();

        auto new_buffer() -> BufferId;
        auto new_swapchain_image(VkImage swapchain_image, VkFormat format, u32 index, ImageUsageFlags usage, const std::string & debug_name) -> ImageId;
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
