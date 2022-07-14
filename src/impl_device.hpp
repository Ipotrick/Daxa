#pragma once

#include "impl_context.hpp"

#include <daxa/daxa.hpp>
#include <daxa/device.hpp>
#include "impl_gpu_resources.hpp"

namespace daxa
{
    struct ImplCommandList;

    struct ZombieCommandLists
    {
        std::mutex mut = {};
        std::vector<std::shared_ptr<ImplCommandList>> zombies;
    };

    struct ImplDevice
    {
        VkPhysicalDevice vk_physical_device = {};
        VkDevice vk_device_handle = {};
        VkQueue vk_main_queue_handle = {};
        u32 main_queue_family_index = {};
        std::shared_ptr<ImplContext> impl_ctx = {};
        DeviceInfo info = {};
        GPUResourceTable gpu_table;
        ZombieCommandLists command_list_zombies = {};

        ImplDevice(DeviceInfo const & info, std::shared_ptr<ImplContext> impl_ctx, VkPhysicalDevice physical_device);
        ~ImplDevice();

        auto new_buffer() -> BufferId;
        void cleanup_buffer(BufferId id);
        auto slot(BufferId id) -> ImplBufferSlot const &;

        auto new_swapchain_image(VkImage swapchain_image, VkFormat format, u32 index) -> ImageId;
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
