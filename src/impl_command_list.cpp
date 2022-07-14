#include "impl_command_list.hpp"

namespace daxa
{
    CommandList::CommandList(std::shared_ptr<void> impl) : Handle(impl) {}

    CommandList::~CommandList()
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(this->impl.get());

        if (this->impl.use_count() == 1)
        {
            std::unique_lock lock{impl.impl_device->command_list_zombies.mut};

            impl.impl_device->command_list_zombies.zombies.push_back(std::static_pointer_cast<ImplCommandList>(this->impl));
        }
    }

    void CommandList::blit_image_to_image(ImageBlitInfo & info)
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(this->impl.get());

        VkImageBlit vk_blit{
            .srcSubresource = *reinterpret_cast<VkImageSubresourceLayers *>(&info.src_slice),
            .srcOffsets = {*reinterpret_cast<VkOffset3D *>(&info.src_offsets[0]), *reinterpret_cast<VkOffset3D *>(&info.src_offsets[1])},
            .dstSubresource = *reinterpret_cast<VkImageSubresourceLayers *>(&info.dst_slice),
            .dstOffsets = {*reinterpret_cast<VkOffset3D *>(&info.dst_offsets[0]), *reinterpret_cast<VkOffset3D *>(&info.dst_offsets[1])},
        };

        vkCmdBlitImage(
            impl.vk_cmd_buffer_handle,
            impl.impl_device->slot(info.src_image).vk_image_handle,
            static_cast<VkImageLayout>(info.src_image_layout),
            impl.impl_device->slot(info.dst_image).vk_image_handle,
            static_cast<VkImageLayout>(info.dst_image_layout),
            1,
            &vk_blit,
            static_cast<VkFilter>(info.filter));
    }

    void CommandList::copy_image_to_image(ImageCopyInfo & info)
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(this->impl.get());

        VkImageCopy vk_image_copy{
            .srcSubresource = *reinterpret_cast<VkImageSubresourceLayers *>(&info.src_slice),
            .srcOffset = {*reinterpret_cast<VkOffset3D *>(&info.src_offset)},
            .dstSubresource = *reinterpret_cast<VkImageSubresourceLayers *>(&info.dst_slice),
            .dstOffset = {*reinterpret_cast<VkOffset3D *>(&info.dst_offset)},
            .extent = {*reinterpret_cast<VkExtent3D *>(&info.extent)},
        };

        vkCmdCopyImage(
            impl.vk_cmd_buffer_handle,
            impl.impl_device->slot(info.src_image).vk_image_handle,
            static_cast<VkImageLayout>(info.src_image_layout),
            impl.impl_device->slot(info.dst_image).vk_image_handle,
            static_cast<VkImageLayout>(info.dst_image_layout),
            1,
            &vk_image_copy);
    }

    void CommandList::clear_image(ImageClearInfo const & info)
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(this->impl.get());

        if (info.dst_slice.image_aspect & ImageAspectFlags::COLOR)
        {
            VkClearColorValue color{
                .float32 = {info.clear_color.f32_value[0], info.clear_color.f32_value[1], info.clear_color.f32_value[2], info.clear_color.f32_value[3]},
            };

            vkCmdClearColorImage(
                impl.vk_cmd_buffer_handle,
                impl.impl_device->slot(info.dst_image).vk_image_handle,
                static_cast<VkImageLayout>(info.dst_image_layout),
                &color,
                1,
                const_cast<VkImageSubresourceRange *>(
                    reinterpret_cast<VkImageSubresourceRange const *>(&info.dst_slice)));
        }

        if (info.dst_slice.image_aspect & (ImageAspectFlags::DEPTH | ImageAspectFlags::STENCIL))
        {
            VkClearDepthStencilValue color{
                .depth = info.clear_color.depth_stencil.depth,
                .stencil = info.clear_color.depth_stencil.stencil,
            };

            vkCmdClearDepthStencilImage(
                impl.vk_cmd_buffer_handle,
                impl.impl_device->slot(info.dst_image).vk_image_handle,
                static_cast<VkImageLayout>(info.dst_image_layout),
                &color,
                1,
                const_cast<VkImageSubresourceRange *>(reinterpret_cast<VkImageSubresourceRange const *>(&info.dst_slice)));
        }
    }

    void CommandList::complete()
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(this->impl.get());

        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete command list once");

        impl.recording_complete = true;

        vkEndCommandBuffer(impl.vk_cmd_buffer_handle);
    }

    ImplCommandList::ImplCommandList(std::shared_ptr<ImplDevice> a_impl_device, CommandListInfo const & a_info)
        : impl_device{a_impl_device}, info{a_info}
    {
        VkCommandPoolCreateInfo vk_command_pool_create_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .queueFamilyIndex = impl_device->main_queue_family_index,
        };

        vkCreateCommandPool(impl_device->vk_device_handle, &vk_command_pool_create_info, nullptr, &this->vk_cmd_pool_handle);

        VkCommandBufferAllocateInfo vk_command_buffer_allocate_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = this->vk_cmd_pool_handle,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        vkAllocateCommandBuffers(impl_device->vk_device_handle, &vk_command_buffer_allocate_info, &this->vk_cmd_buffer_handle);
    }

    ImplCommandList::~ImplCommandList()
    {
        vkDestroyCommandPool(impl_device->vk_device_handle, this->vk_cmd_pool_handle, nullptr);
    }

    void ImplCommandList::init()
    {
        vkResetCommandPool(impl_device->vk_device_handle, this->vk_cmd_pool_handle, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

        VkCommandBufferBeginInfo vk_command_buffer_begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        vkBeginCommandBuffer(this->vk_cmd_buffer_handle, &vk_command_buffer_begin_info);

        recording_complete = false;
    }
} // namespace daxa
