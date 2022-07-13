#pragma once

#include "impl_context.hpp"

#include <daxa/daxa.hpp>
#include <daxa/device.hpp>
#include "impl_gpu_resources.hpp"

namespace daxa
{
    struct ImplDevice
    {
        VkPhysicalDevice vk_physical_device = {};
        VkDevice vk_device_handle = {};
        VkQueue vk_main_queue_handle = {};
        u32 main_queue_family_index = {};
        std::shared_ptr<ImplContext> impl_ctx = {};
        DeviceInfo info = {};
        GPUResourceTable gpu_table;

        ImplDevice(DeviceInfo const & info, std::shared_ptr<ImplContext> impl_ctx, VkPhysicalDevice physical_device);
        ~ImplDevice();

        auto new_buffer() -> BufferId;
        void cleanup_buffer(BufferId id);
        auto info_buffer(BufferId id) -> BufferInfo const &;

        auto new_swapchain_image(VkImage swapchain_image, VkFormat format) -> ImageId;
        auto new_image() -> ImageId;
        void cleanup_image(ImageId id);
        auto info_image(ImageId id) -> ImageInfo const &;

        auto new_image_view() -> ImageViewId;
        void cleanup_image_view(ImageViewId id);
        auto info_image_view(ImageViewId id) -> ImageViewInfo const &;

        auto new_sampler() -> SamplerId;
        void cleanup_sampler(SamplerId id);
        auto info_sampler(SamplerId id) -> SamplerInfo const &;
    };
} // namespace daxa
