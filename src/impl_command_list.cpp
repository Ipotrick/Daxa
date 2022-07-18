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
#if defined(DAXA_ENABLE_THREADSAFETY)
            std::unique_lock lock{DAXA_LOCK_WEAK(impl->impl_device)->command_list_recyclable_list.mtx};
#endif
            DAXA_LOCK_WEAK(impl->impl_device)->command_list_recyclable_list.recyclables.push_back(impl);
        }
    }

    void CommandList::copy_buffer_to_buffer(BufferCopyInfo const & info)
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(this->impl.get());

        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");

        VkBufferCopy vk_buffer_copy{
            .srcOffset = info.src_offset,
            .dstOffset = info.dst_offset,
            .size = info.size,
        };

        vkCmdCopyBuffer(
            impl.vk_cmd_buffer, 
            DAXA_LOCK_WEAK(impl.impl_device)->slot(info.src_buffer).vk_buffer, 
            DAXA_LOCK_WEAK(impl.impl_device)->slot(info.dst_buffer).vk_buffer,
            1,
            &vk_buffer_copy
        );
    }

    void CommandList::copy_buffer_to_image(BufferImageCopy const & info)
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(this->impl.get());

        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");

        VkBufferImageCopy vk_buffer_image_copy{
            .bufferOffset = info.buffer_offset,
            .bufferRowLength = static_cast<u32>(info.image_extent.x * info.image_extent.y * info.image_extent.z),
            .bufferImageHeight = 1u,
            .imageSubresource = *reinterpret_cast<VkImageSubresourceLayers const *>(&info.image_slice),
            .imageOffset = *reinterpret_cast<VkOffset3D const *>(&info.image_offset),
            .imageExtent = *reinterpret_cast<VkExtent3D const *>(&info.image_extent),
        };

        vkCmdCopyBufferToImage(
            impl.vk_cmd_buffer,
            DAXA_LOCK_WEAK(impl.impl_device)->slot(info.buffer).vk_buffer,
            DAXA_LOCK_WEAK(impl.impl_device)->slot(info.image).vk_image,
            static_cast<VkImageLayout>(info.image_layout),
            1,
            &vk_buffer_image_copy
        );
    }

    void CommandList::copy_image_to_buffer(BufferImageCopy const & info)
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(this->impl.get());

        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");

        VkBufferImageCopy vk_buffer_image_copy{
            .bufferOffset = info.buffer_offset,
            .bufferRowLength = static_cast<u32>(info.image_extent.x * info.image_extent.y * info.image_extent.z),
            .bufferImageHeight = 1u,
            .imageSubresource = *reinterpret_cast<VkImageSubresourceLayers const *>(&info.image_slice),
            .imageOffset = *reinterpret_cast<VkOffset3D const *>(&info.image_offset),
            .imageExtent = *reinterpret_cast<VkExtent3D const *>(&info.image_extent),
        };

        vkCmdCopyImageToBuffer(
            impl.vk_cmd_buffer,
            DAXA_LOCK_WEAK(impl.impl_device)->slot(info.image).vk_image,
            static_cast<VkImageLayout>(info.image_layout),
            DAXA_LOCK_WEAK(impl.impl_device)->slot(info.buffer).vk_buffer,
            1,
            &vk_buffer_image_copy
        );
    }


    void CommandList::blit_image_to_image(ImageBlitInfo const & info)
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(this->impl.get());

        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");

        VkImageBlit vk_blit{
            .srcSubresource = *reinterpret_cast<VkImageSubresourceLayers const *>(&info.src_slice),
            .srcOffsets = {*reinterpret_cast<VkOffset3D const *>(&info.src_offsets[0]), *reinterpret_cast<VkOffset3D const *>(&info.src_offsets[1])},
            .dstSubresource = *reinterpret_cast<VkImageSubresourceLayers const *>(&info.dst_slice),
            .dstOffsets = {*reinterpret_cast<VkOffset3D const *>(&info.dst_offsets[0]), *reinterpret_cast<VkOffset3D const *>(&info.dst_offsets[1])},
        };

        vkCmdBlitImage(
            impl.vk_cmd_buffer,
            DAXA_LOCK_WEAK(impl.impl_device)->slot(info.src_image).vk_image,
            static_cast<VkImageLayout>(info.src_image_layout),
            DAXA_LOCK_WEAK(impl.impl_device)->slot(info.dst_image).vk_image,
            static_cast<VkImageLayout>(info.dst_image_layout),
            1,
            &vk_blit,
            static_cast<VkFilter>(info.filter));
    }

    void CommandList::copy_image_to_image(ImageCopyInfo const & info)
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(this->impl.get());

        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");

        VkImageCopy vk_image_copy{
            .srcSubresource = *reinterpret_cast<VkImageSubresourceLayers const *>(&info.src_slice),
            .srcOffset = {*reinterpret_cast<VkOffset3D const *>(&info.src_offset)},
            .dstSubresource = *reinterpret_cast<VkImageSubresourceLayers const *>(&info.dst_slice),
            .dstOffset = {*reinterpret_cast<VkOffset3D const *>(&info.dst_offset)},
            .extent = {*reinterpret_cast<VkExtent3D const *>(&info.extent)},
        };

        vkCmdCopyImage(
            impl.vk_cmd_buffer,
            DAXA_LOCK_WEAK(impl.impl_device)->slot(info.src_image).vk_image,
            static_cast<VkImageLayout>(info.src_image_layout),
            DAXA_LOCK_WEAK(impl.impl_device)->slot(info.dst_image).vk_image,
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
                impl.vk_cmd_buffer,
                DAXA_LOCK_WEAK(impl.impl_device)->slot(info.dst_image).vk_image,
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
                impl.vk_cmd_buffer,
                DAXA_LOCK_WEAK(impl.impl_device)->slot(info.dst_image).vk_image,
                static_cast<VkImageLayout>(info.dst_image_layout),
                &color,
                1,
                const_cast<VkImageSubresourceRange *>(reinterpret_cast<VkImageSubresourceRange const *>(&info.dst_slice)));
        }
    }

    void CommandList::push_constant(void const * data, u32 size, u32 offset)
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(this->impl.get());

        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");

        DAXA_DBG_ASSERT_TRUE_M(size <= MAX_PUSH_CONSTANT_BYTE_SIZE, MAX_PUSH_CONSTANT_SIZE_ERROR);
        DAXA_DBG_ASSERT_TRUE_M(size % 4 == 0, "push constant size must be a multiple of 4 bytes");

        usize push_constant_device_word_size = size / 4;

        vkCmdPushConstants(impl.vk_cmd_buffer, impl.pipeline_layouts[push_constant_device_word_size], VK_SHADER_STAGE_ALL, offset, size, data);
    }
    void CommandList::bind_pipeline(ComputePipeline const & pipeline)
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(this->impl.get());
        auto & pipeline_impl = *reinterpret_cast<ImplComputePipeline *>(pipeline.impl.get());

        vkCmdBindDescriptorSets(impl.vk_cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_impl.vk_pipeline_layout, 0, 1, &DAXA_LOCK_WEAK(impl.impl_device)->gpu_table.vk_descriptor_set, 0, nullptr);

        vkCmdBindPipeline(impl.vk_cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_impl.vk_pipeline);
    }
    void CommandList::dispatch(u32 group_x, u32 group_y, u32 group_z)
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(this->impl.get());

        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");

        vkCmdDispatch(impl.vk_cmd_buffer, group_x, group_y, group_z);
    }

    void defer_destruction_helper(void* impl_coid, GPUResourceId id, u8 index)
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(impl_coid);

        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        DAXA_DBG_ASSERT_TRUE_M(impl.deferred_destruction_count < DEFERRED_DESTRUCTION_COUNT_MAX, "can not defer the destruction of more than 32 resources per command list recording");

        impl.deferred_destructions[impl.deferred_destruction_count++] = { id, index };
    }

    void CommandList::destroy_buffer_deferred(BufferId id)
    {
        defer_destruction_helper(impl.get(), GPUResourceId{ .index = id.index, .version = id.version }, DEFERRED_DESTRUCTION_BUFFER_INDEX);
    }

    void CommandList::destroy_image_deferred(ImageId id)
    {
        defer_destruction_helper(impl.get(), GPUResourceId{ .index = id.index, .version = id.version }, DEFERRED_DESTRUCTION_IMAGE_INDEX);
    }

    void CommandList::destroy_image_view_deferred(ImageViewId id)
    {
        defer_destruction_helper(impl.get(), GPUResourceId{ .index = id.index, .version = id.version }, DEFERRED_DESTRUCTION_IMAGE_VIEW_INDEX);
    }

    void CommandList::destroy_sampler_deferred(SamplerId id)
    {
        defer_destruction_helper(impl.get(), GPUResourceId{ .index = id.index, .version = id.version }, DEFERRED_DESTRUCTION_SAMPLER_INDEX);
    }


    void CommandList::complete()
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(this->impl.get());

        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");

        impl.recording_complete = true;

        vkEndCommandBuffer(impl.vk_cmd_buffer);
    }

    void CommandList::pipeline_barrier(PipelineBarrierInfo const & info)
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(this->impl.get());

        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");

        VkMemoryBarrier2 vk_memory_barrier{
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
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

        vkCmdPipelineBarrier2(impl.vk_cmd_buffer, &vk_dependency_info);
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
            .image = DAXA_LOCK_WEAK(impl.impl_device)->slot(info.image_id).vk_image,
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

        vkCmdPipelineBarrier2(impl.vk_cmd_buffer, &vk_dependency_info);
    }

    ImplCommandList::ImplCommandList(std::weak_ptr<ImplDevice> a_impl_device)
        : impl_device{a_impl_device}, pipeline_layouts{DAXA_LOCK_WEAK(impl_device)->gpu_table.pipeline_layouts}
    {
        VkCommandPoolCreateInfo vk_command_pool_create_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .queueFamilyIndex = DAXA_LOCK_WEAK(impl_device)->main_queue_family_index,
        };

        vkCreateCommandPool(DAXA_LOCK_WEAK(impl_device)->vk_device, &vk_command_pool_create_info, nullptr, &this->vk_cmd_pool);

        VkCommandBufferAllocateInfo vk_command_buffer_allocate_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = this->vk_cmd_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        vkAllocateCommandBuffers(DAXA_LOCK_WEAK(impl_device)->vk_device, &vk_command_buffer_allocate_info, &this->vk_cmd_buffer);
    }

    ImplCommandList::~ImplCommandList()
    {
        vkDestroyCommandPool(DAXA_LOCK_WEAK(impl_device)->vk_device, this->vk_cmd_pool, nullptr);
    }

    void ImplCommandList::initialize(CommandListInfo const & a_info)
    {
        VkCommandBufferBeginInfo vk_command_buffer_begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        vkBeginCommandBuffer(this->vk_cmd_buffer, &vk_command_buffer_begin_info);

        recording_complete = false;

        if (DAXA_LOCK_WEAK(DAXA_LOCK_WEAK(this->impl_device)->impl_ctx)->enable_debug_names && this->info.debug_name.size() > 0)
        {
            VkDebugUtilsObjectNameInfoEXT cmd_buffer_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_COMMAND_BUFFER,
                .objectHandle = reinterpret_cast<uint64_t>(this->vk_cmd_buffer),
                .pObjectName = this->info.debug_name.c_str(),
            };
            vkSetDebugUtilsObjectNameEXT(DAXA_LOCK_WEAK(this->impl_device)->vk_device, &cmd_buffer_name_info);

            VkDebugUtilsObjectNameInfoEXT cmd_pool_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_COMMAND_POOL,
                .objectHandle = reinterpret_cast<uint64_t>(this->vk_cmd_pool),
                .pObjectName = this->info.debug_name.c_str(),
            };
            vkSetDebugUtilsObjectNameEXT(DAXA_LOCK_WEAK(this->impl_device)->vk_device, &cmd_pool_name_info);
        }
    }

    void ImplCommandList::reset()
    {
        vkResetCommandPool(DAXA_LOCK_WEAK(impl_device)->vk_device, this->vk_cmd_pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
        deferred_destruction_count = 0;
    }
} // namespace daxa
