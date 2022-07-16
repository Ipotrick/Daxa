#include "impl_command_list.hpp"
#include "impl_device.hpp"

namespace daxa
{
    CommandList::CommandList(std::shared_ptr<void> a_impl) : Handle(std::move(a_impl)) {}

    CommandList::~CommandList()
    {
        if (this->impl.use_count() == 1)
        {
            std::shared_ptr<ImplCommandList> impl = std::static_pointer_cast<ImplCommandList>(this->impl);
            impl->reset();
            std::unique_lock lock{DAXA_LOCK_WEAK(impl->impl_device)->command_list_recyclable_list.mtx};
            DAXA_LOCK_WEAK(impl->impl_device)->command_list_recyclable_list.recyclables.push_back(impl);
        }
    }

    void CommandList::blit_image_to_image(ImageBlitInfo & info)
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(this->impl.get());

        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");

        VkImageBlit vk_blit{
            .srcSubresource = *reinterpret_cast<VkImageSubresourceLayers *>(&info.src_slice),
            .srcOffsets = {*reinterpret_cast<VkOffset3D *>(&info.src_offsets[0]), *reinterpret_cast<VkOffset3D *>(&info.src_offsets[1])},
            .dstSubresource = *reinterpret_cast<VkImageSubresourceLayers *>(&info.dst_slice),
            .dstOffsets = {*reinterpret_cast<VkOffset3D *>(&info.dst_offsets[0]), *reinterpret_cast<VkOffset3D *>(&info.dst_offsets[1])},
        };

        vkCmdBlitImage(
            impl.vk_cmd_buffer_handle,
            DAXA_LOCK_WEAK(impl.impl_device)->slot(info.src_image).vk_image_handle,
            static_cast<VkImageLayout>(info.src_image_layout),
            DAXA_LOCK_WEAK(impl.impl_device)->slot(info.dst_image).vk_image_handle,
            static_cast<VkImageLayout>(info.dst_image_layout),
            1,
            &vk_blit,
            static_cast<VkFilter>(info.filter));
    }

    void CommandList::copy_image_to_image(ImageCopyInfo & info)
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(this->impl.get());

        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");

        VkImageCopy vk_image_copy{
            .srcSubresource = *reinterpret_cast<VkImageSubresourceLayers *>(&info.src_slice),
            .srcOffset = {*reinterpret_cast<VkOffset3D *>(&info.src_offset)},
            .dstSubresource = *reinterpret_cast<VkImageSubresourceLayers *>(&info.dst_slice),
            .dstOffset = {*reinterpret_cast<VkOffset3D *>(&info.dst_offset)},
            .extent = {*reinterpret_cast<VkExtent3D *>(&info.extent)},
        };

        vkCmdCopyImage(
            impl.vk_cmd_buffer_handle,
            DAXA_LOCK_WEAK(impl.impl_device)->slot(info.src_image).vk_image_handle,
            static_cast<VkImageLayout>(info.src_image_layout),
            DAXA_LOCK_WEAK(impl.impl_device)->slot(info.dst_image).vk_image_handle,
            static_cast<VkImageLayout>(info.dst_image_layout),
            1,
            &vk_image_copy);
    }

    void CommandList::clear_image(ImageClearInfo const & info)
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(this->impl.get());

        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");

        if (info.dst_slice.image_aspect & ImageAspectFlagBits::COLOR)
        {
            VkClearColorValue color{
                .float32 = {info.clear_color.f32_value[0], info.clear_color.f32_value[1], info.clear_color.f32_value[2], info.clear_color.f32_value[3]},
            };

            vkCmdClearColorImage(
                impl.vk_cmd_buffer_handle,
                DAXA_LOCK_WEAK(impl.impl_device)->slot(info.dst_image).vk_image_handle,
                static_cast<VkImageLayout>(info.dst_image_layout),
                &color,
                1,
                const_cast<VkImageSubresourceRange *>(
                    reinterpret_cast<VkImageSubresourceRange const *>(&info.dst_slice)));
        }

        if (info.dst_slice.image_aspect & (ImageAspectFlagBits::DEPTH | ImageAspectFlagBits::STENCIL))
        {
            VkClearDepthStencilValue color{
                .depth = info.clear_color.depth_stencil.depth,
                .stencil = info.clear_color.depth_stencil.stencil,
            };

            vkCmdClearDepthStencilImage(
                impl.vk_cmd_buffer_handle,
                DAXA_LOCK_WEAK(impl.impl_device)->slot(info.dst_image).vk_image_handle,
                static_cast<VkImageLayout>(info.dst_image_layout),
                &color,
                1,
                const_cast<VkImageSubresourceRange *>(reinterpret_cast<VkImageSubresourceRange const *>(&info.dst_slice)));
        }
    }

    void CommandList::complete()
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(this->impl.get());

        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");

        impl.recording_complete = true;

        vkEndCommandBuffer(impl.vk_cmd_buffer_handle);
    }

    void CommandList::pipeline_barrier(PipelineBarrierInfo const & info)
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(this->impl.get());

        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");

        VkMemoryBarrier2 vk_memory_barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = 0x0111'1111'1111'1111ull & info.awaited_pipeline_access,
            .srcAccessMask = (info.awaited_pipeline_access & PipelineStageAccessFlagBits::WRITE_ACCESS ? VK_ACCESS_2_MEMORY_WRITE_BIT : 0ull) | (info.awaited_pipeline_access & PipelineStageAccessFlagBits::READ_ACCESS ? VK_ACCESS_2_MEMORY_READ_BIT : 0ull),
            .dstStageMask = 0x0111'1111'1111'1111ull & info.waiting_pipeline_access,
            .dstAccessMask = (info.waiting_pipeline_access & PipelineStageAccessFlagBits::WRITE_ACCESS ? VK_ACCESS_2_MEMORY_WRITE_BIT : 0ull) | (info.waiting_pipeline_access & PipelineStageAccessFlagBits::READ_ACCESS ? VK_ACCESS_2_MEMORY_READ_BIT : 0ull),
        };

        VkDependencyInfo vk_dependency_info{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext = nullptr,
            .dependencyFlags = {},
            .memoryBarrierCount = 1,
            .pMemoryBarriers = &vk_memory_barrier,
            .bufferMemoryBarrierCount = 0,
            .pBufferMemoryBarriers = nullptr,
            .imageMemoryBarrierCount = 0,
            .pImageMemoryBarriers = nullptr,
        };

        vkCmdPipelineBarrier2(impl.vk_cmd_buffer_handle, &vk_dependency_info);
    }

    void CommandList::pipeline_barrier_image_transition(PipelineBarrierImageTransitionInfo const & info)
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(this->impl.get());

        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");

        VkImageMemoryBarrier2 vk_image_memory_barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = 0x0111'1111'1111'1111ull & info.awaited_pipeline_access,
            .srcAccessMask = (info.awaited_pipeline_access & PipelineStageAccessFlagBits::WRITE_ACCESS ? VK_ACCESS_2_MEMORY_WRITE_BIT : 0ull) | (info.awaited_pipeline_access & PipelineStageAccessFlagBits::READ_ACCESS ? VK_ACCESS_2_MEMORY_READ_BIT : 0ull),
            .dstStageMask = 0x0111'1111'1111'1111ull & info.waiting_pipeline_access,
            .dstAccessMask = (info.waiting_pipeline_access & PipelineStageAccessFlagBits::WRITE_ACCESS ? VK_ACCESS_2_MEMORY_WRITE_BIT : 0ull) | (info.waiting_pipeline_access & PipelineStageAccessFlagBits::READ_ACCESS ? VK_ACCESS_2_MEMORY_READ_BIT : 0ull),
            .oldLayout = static_cast<VkImageLayout>(info.before_layout),
            .newLayout = static_cast<VkImageLayout>(info.after_layout),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = DAXA_LOCK_WEAK(impl.impl_device)->slot(info.image_id).vk_image_handle,
            .subresourceRange = *reinterpret_cast<VkImageSubresourceRange const *>(&info.image_slice),
        };

        VkDependencyInfo vk_dependency_info{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext = nullptr,
            .dependencyFlags = {},
            .memoryBarrierCount = 0,
            .pMemoryBarriers = nullptr,
            .bufferMemoryBarrierCount = 0,
            .pBufferMemoryBarriers = nullptr,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &vk_image_memory_barrier,
        };

        vkCmdPipelineBarrier2(impl.vk_cmd_buffer_handle, &vk_dependency_info);
    }

    ImplCommandList::ImplCommandList(std::weak_ptr<ImplDevice> a_impl_device)
        : impl_device{a_impl_device}
    {
        VkCommandPoolCreateInfo vk_command_pool_create_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .queueFamilyIndex = DAXA_LOCK_WEAK(impl_device)->main_queue_family_index,
        };

        vkCreateCommandPool(DAXA_LOCK_WEAK(impl_device)->vk_device_handle, &vk_command_pool_create_info, nullptr, &this->vk_cmd_pool_handle);

        VkCommandBufferAllocateInfo vk_command_buffer_allocate_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = this->vk_cmd_pool_handle,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        vkAllocateCommandBuffers(DAXA_LOCK_WEAK(impl_device)->vk_device_handle, &vk_command_buffer_allocate_info, &this->vk_cmd_buffer_handle);
    }

    ImplCommandList::~ImplCommandList()
    {
        vkDestroyCommandPool(DAXA_LOCK_WEAK(impl_device)->vk_device_handle, this->vk_cmd_pool_handle, nullptr);
    }

    void ImplCommandList::initialize(CommandListInfo const & a_info)
    {
        VkCommandBufferBeginInfo vk_command_buffer_begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        vkBeginCommandBuffer(this->vk_cmd_buffer_handle, &vk_command_buffer_begin_info);

        recording_complete = false;

        if (DAXA_LOCK_WEAK(DAXA_LOCK_WEAK(this->impl_device)->impl_ctx)->enable_debug_names && this->info.debug_name.size() > 0)
        {
            VkDebugUtilsObjectNameInfoEXT cmd_buffer_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_COMMAND_BUFFER,
                .objectHandle = reinterpret_cast<uint64_t>(this->vk_cmd_buffer_handle),
                .pObjectName = this->info.debug_name.c_str(),
            };
            vkSetDebugUtilsObjectNameEXT(DAXA_LOCK_WEAK(this->impl_device)->vk_device_handle, &cmd_buffer_name_info);

            VkDebugUtilsObjectNameInfoEXT cmd_pool_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_COMMAND_POOL,
                .objectHandle = reinterpret_cast<uint64_t>(this->vk_cmd_pool_handle),
                .pObjectName = this->info.debug_name.c_str(),
            };
            vkSetDebugUtilsObjectNameEXT(DAXA_LOCK_WEAK(this->impl_device)->vk_device_handle, &cmd_pool_name_info);
        }
    }

    void ImplCommandList::reset()
    {
        vkResetCommandPool(DAXA_LOCK_WEAK(impl_device)->vk_device_handle, this->vk_cmd_pool_handle, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
    }
} // namespace daxa
