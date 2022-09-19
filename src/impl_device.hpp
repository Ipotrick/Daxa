#pragma once

#include <daxa/daxa.hpp>
#include <daxa/device.hpp>

#include "impl_core.hpp"
#include "impl_context.hpp"

#include "impl_pipeline.hpp"
#include "impl_command_list.hpp"
#include "impl_swapchain.hpp"
#include "impl_semaphore.hpp"
#include "impl_gpu_resources.hpp"

namespace daxa
{
    struct ImplDevice final : ManagedSharedState
    {
        ManagedWeakPtr impl_ctx = {};
        VkPhysicalDevice vk_physical_device = {};
        VkDevice vk_device = {};

        PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;

        VmaAllocator vma_allocator = {};
        DeviceProperties vk_info = {};
        DeviceInfo info = {};
        VkSampler vk_dummy_sampler = {};
        VkBuffer buffer_device_address_buffer = {};
        VmaAllocation buffer_device_address_buffer_allocation = {};

        // Gpu resource table:
        GPUResourceTable gpu_table = {};

        // Resource recycling:
        DAXA_ONLY_IF_THREADSAFETY(std::mutex main_queue_command_pool_buffer_recycle_mtx = {});
        CommandBufferPoolPool buffer_pool_pool = {};
        // Main queue:
        VkQueue main_queue_vk_queue = {};
        u32 main_queue_family_index = {};

        DAXA_ATOMIC_U64 main_queue_cpu_timeline = {};
        VkSemaphore vk_main_queue_gpu_timeline_semaphore = {};

        DAXA_ONLY_IF_THREADSAFETY(std::mutex main_queue_zombies_mtx = {});
        std::deque<std::pair<u64, std::vector<ManagedPtr>>> main_queue_submits_zombies = {};
        std::deque<std::pair<u64, CommandListZombie>> main_queue_command_list_zombies = {};
        std::deque<std::pair<u64, BufferId>> main_queue_buffer_zombies = {};
        std::deque<std::pair<u64, ImageId>> main_queue_image_zombies = {};
        std::deque<std::pair<u64, ImageViewId>> main_queue_image_view_zombies = {};
        std::deque<std::pair<u64, SamplerId>> main_queue_sampler_zombies = {};
        std::deque<std::pair<u64, SemaphoreZombie>> main_queue_semaphore_zombies = {};
        std::deque<std::pair<u64, PipelineZombie>> main_queue_pipeline_zombies = {};
        void main_queue_collect_garbage();
        void wait_idle();

        ImplDevice(DeviceInfo const & info, DeviceProperties const & vk_info, ManagedWeakPtr impl_ctx, VkPhysicalDevice physical_device);
        virtual ~ImplDevice() override final;

        auto validate_image_slice(ImageMipArraySlice const & slice, ImageId id) -> ImageMipArraySlice;
        auto validate_image_slice(ImageMipArraySlice const & slice, ImageViewId id) -> ImageMipArraySlice;

        auto new_buffer(BufferInfo const & info) -> BufferId;
        auto new_swapchain_image(VkImage swapchain_image, VkFormat format, u32 index, ImageUsageFlags usage, ImageInfo const & info) -> ImageId;
        auto new_image(ImageInfo const & info) -> ImageId;
        auto new_image_view(ImageViewInfo const & info) -> ImageViewId;
        auto new_sampler(SamplerInfo const & info) -> SamplerId;

        auto slot(BufferId id) -> ImplBufferSlot &;
        auto slot(ImageId id) -> ImplImageSlot &;
        auto slot(ImageViewId id) -> ImplImageViewSlot &;
        auto slot(SamplerId id) -> ImplSamplerSlot &;

        auto slot(BufferId id) const -> ImplBufferSlot const &;
        auto slot(ImageId id) const -> ImplImageSlot const &;
        auto slot(ImageViewId id) const -> ImplImageViewSlot const &;
        auto slot(SamplerId id) const -> ImplSamplerSlot const &;

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
