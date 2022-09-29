#include "impl_command_list.hpp"

#include "impl_split_barrier.hpp"
#include "impl_device.hpp"

namespace daxa
{
    auto get_vk_image_memory_barrier(ImageBarrierInfo const & image_barrier, VkImage vk_image) -> VkImageMemoryBarrier2
    {
        return VkImageMemoryBarrier2{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = static_cast<u64>(image_barrier.awaited_pipeline_access.stages),
            .srcAccessMask = static_cast<u32>(image_barrier.awaited_pipeline_access.type),
            .dstStageMask = static_cast<u64>(image_barrier.waiting_pipeline_access.stages),
            .dstAccessMask = static_cast<u32>(image_barrier.waiting_pipeline_access.type),
            .oldLayout = static_cast<VkImageLayout>(image_barrier.before_layout),
            .newLayout = static_cast<VkImageLayout>(image_barrier.after_layout),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = vk_image,
            .subresourceRange = *reinterpret_cast<VkImageSubresourceRange const *>(&image_barrier.image_slice),
        };
    }

    auto get_vk_memory_barrier(MemoryBarrierInfo const & memory_barrier) -> VkMemoryBarrier2
    {
        return VkMemoryBarrier2{
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = static_cast<u64>(memory_barrier.awaited_pipeline_access.stages),
            .srcAccessMask = static_cast<u32>(memory_barrier.awaited_pipeline_access.type),
            .dstStageMask = static_cast<u64>(memory_barrier.waiting_pipeline_access.stages),
            .dstAccessMask = static_cast<u32>(memory_barrier.waiting_pipeline_access.type),
        };
    }

    auto get_vk_dependency_info(
        std::vector<VkImageMemoryBarrier2> const & vk_image_memory_barriers,
        std::vector<VkMemoryBarrier2> const & vk_memory_barriers) -> VkDependencyInfo
    {
        return VkDependencyInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext = nullptr,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            .memoryBarrierCount = static_cast<u32>(vk_memory_barriers.size()),
            .pMemoryBarriers = vk_memory_barriers.data(),
            .bufferMemoryBarrierCount = 0,
            .pBufferMemoryBarriers = nullptr,
            .imageMemoryBarrierCount = static_cast<u32>(vk_image_memory_barriers.size()),
            .pImageMemoryBarriers = vk_image_memory_barriers.data(),
        };
    }

    CommandList::CommandList(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}

    void CommandList::copy_buffer_to_buffer(BufferCopyInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();

        VkBufferCopy vk_buffer_copy{
            .srcOffset = info.src_offset,
            .dstOffset = info.dst_offset,
            .size = info.size,
        };

        vkCmdCopyBuffer(
            impl.vk_cmd_buffer,
            impl.impl_device.as<ImplDevice>()->slot(info.src_buffer).vk_buffer,
            impl.impl_device.as<ImplDevice>()->slot(info.dst_buffer).vk_buffer,
            1,
            &vk_buffer_copy);
    }

    void CommandList::copy_buffer_to_image(BufferImageCopy const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();

        VkBufferImageCopy vk_buffer_image_copy{
            .bufferOffset = info.buffer_offset,
            .bufferRowLength = static_cast<u32>(info.image_extent.x * info.image_extent.z),
            .bufferImageHeight = static_cast<u32>(info.image_extent.y),
            .imageSubresource = *reinterpret_cast<VkImageSubresourceLayers const *>(&info.image_slice),
            .imageOffset = *reinterpret_cast<VkOffset3D const *>(&info.image_offset),
            .imageExtent = *reinterpret_cast<VkExtent3D const *>(&info.image_extent),
        };

        vkCmdCopyBufferToImage(
            impl.vk_cmd_buffer,
            impl.impl_device.as<ImplDevice>()->slot(info.buffer).vk_buffer,
            impl.impl_device.as<ImplDevice>()->slot(info.image).vk_image,
            static_cast<VkImageLayout>(info.image_layout),
            1,
            &vk_buffer_image_copy);
    }

    void CommandList::copy_image_to_buffer(BufferImageCopy const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();

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
            impl.impl_device.as<ImplDevice>()->slot(info.image).vk_image,
            static_cast<VkImageLayout>(info.image_layout),
            impl.impl_device.as<ImplDevice>()->slot(info.buffer).vk_buffer,
            1,
            &vk_buffer_image_copy);
    }

    void CommandList::blit_image_to_image(ImageBlitInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");
        impl.flush_barriers();

        VkImageBlit vk_blit{
            .srcSubresource = *reinterpret_cast<VkImageSubresourceLayers const *>(&info.src_slice),
            .srcOffsets = {*reinterpret_cast<VkOffset3D const *>(&info.src_offsets[0]), *reinterpret_cast<VkOffset3D const *>(&info.src_offsets[1])},
            .dstSubresource = *reinterpret_cast<VkImageSubresourceLayers const *>(&info.dst_slice),
            .dstOffsets = {*reinterpret_cast<VkOffset3D const *>(&info.dst_offsets[0]), *reinterpret_cast<VkOffset3D const *>(&info.dst_offsets[1])},
        };

        vkCmdBlitImage(
            impl.vk_cmd_buffer,
            impl.impl_device.as<ImplDevice>()->slot(info.src_image).vk_image,
            static_cast<VkImageLayout>(info.src_image_layout),
            impl.impl_device.as<ImplDevice>()->slot(info.dst_image).vk_image,
            static_cast<VkImageLayout>(info.dst_image_layout),
            1,
            &vk_blit,
            static_cast<VkFilter>(info.filter));
    }

    void CommandList::copy_image_to_image(ImageCopyInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");
        impl.flush_barriers();

        VkImageCopy vk_image_copy{
            .srcSubresource = *reinterpret_cast<VkImageSubresourceLayers const *>(&info.src_slice),
            .srcOffset = {*reinterpret_cast<VkOffset3D const *>(&info.src_offset)},
            .dstSubresource = *reinterpret_cast<VkImageSubresourceLayers const *>(&info.dst_slice),
            .dstOffset = {*reinterpret_cast<VkOffset3D const *>(&info.dst_offset)},
            .extent = {*reinterpret_cast<VkExtent3D const *>(&info.extent)},
        };

        vkCmdCopyImage(
            impl.vk_cmd_buffer,
            impl.impl_device.as<ImplDevice>()->slot(info.src_image).vk_image,
            static_cast<VkImageLayout>(info.src_image_layout),
            impl.impl_device.as<ImplDevice>()->slot(info.dst_image).vk_image,
            static_cast<VkImageLayout>(info.dst_image_layout),
            1,
            &vk_image_copy);
    }

    void CommandList::clear_image(ImageClearInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");
        impl.flush_barriers();

        if (info.dst_slice.image_aspect & ImageAspectFlagBits::COLOR)
        {
            // TODO: Also use the other 4 component values: i32 and u32!
            auto const & clear_value = std::get<std::array<f32, 4>>(info.clear_value);
            VkClearColorValue color{
                .float32 = {clear_value[0], clear_value[1], clear_value[2], clear_value[3]},
            };

            vkCmdClearColorImage(
                impl.vk_cmd_buffer,
                impl.impl_device.as<ImplDevice>()->slot(info.dst_image).vk_image,
                static_cast<VkImageLayout>(info.dst_image_layout),
                &color,
                1,
                const_cast<VkImageSubresourceRange *>(
                    reinterpret_cast<VkImageSubresourceRange const *>(&info.dst_slice)));
        }

        if (info.dst_slice.image_aspect & (ImageAspectFlagBits::DEPTH | ImageAspectFlagBits::STENCIL))
        {
            auto const & clear_value = std::get<DepthValue>(info.clear_value);
            VkClearDepthStencilValue color{
                .depth = clear_value.depth,
                .stencil = clear_value.stencil,
            };

            vkCmdClearDepthStencilImage(
                impl.vk_cmd_buffer,
                impl.impl_device.as<ImplDevice>()->slot(info.dst_image).vk_image,
                static_cast<VkImageLayout>(info.dst_image_layout),
                &color,
                1,
                const_cast<VkImageSubresourceRange *>(reinterpret_cast<VkImageSubresourceRange const *>(&info.dst_slice)));
        }
    }

    void CommandList::clear_buffer(BufferClearInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");
        impl.flush_barriers();

        vkCmdFillBuffer(
            impl.vk_cmd_buffer,
            impl.impl_device.as<ImplDevice>()->slot(info.buffer).vk_buffer,
            static_cast<VkDeviceSize>(info.offset),
            static_cast<VkDeviceSize>(info.size),
            info.clear_value);
    }

    void CommandList::push_constant(void const * data, u32 size, u32 offset)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        DAXA_DBG_ASSERT_TRUE_M(size <= MAX_PUSH_CONSTANT_BYTE_SIZE, MAX_PUSH_CONSTANT_SIZE_ERROR);
        DAXA_DBG_ASSERT_TRUE_M(size % 4 == 0, "push constant size must be a multiple of 4 bytes");
        impl.flush_barriers();

        vkCmdPushConstants(impl.vk_cmd_buffer, (*impl.pipeline_layouts)[(size + 3) / 4], VK_SHADER_STAGE_ALL, offset, size, data);
    }

    void CommandList::set_pipeline(ComputePipeline const & pipeline)
    {
        auto & impl = *as<ImplCommandList>();
        auto const & pipeline_impl = *pipeline.as<ImplComputePipeline>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();

        vkCmdBindDescriptorSets(impl.vk_cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_impl.vk_pipeline_layout, 0, 1, &impl.impl_device.as<ImplDevice>()->gpu_table.vk_descriptor_set, 0, nullptr);

        vkCmdBindPipeline(impl.vk_cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_impl.vk_pipeline);
    }

    void CommandList::set_pipeline(RasterPipeline const & pipeline)
    {
        auto & impl = *as<ImplCommandList>();
        auto const & pipeline_impl = *pipeline.as<ImplRasterPipeline>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();

        vkCmdBindDescriptorSets(impl.vk_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_impl.vk_pipeline_layout, 0, 1, &impl.impl_device.as<ImplDevice>()->gpu_table.vk_descriptor_set, 0, nullptr);

        vkCmdBindPipeline(impl.vk_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_impl.vk_pipeline);
    }

    void CommandList::dispatch(u32 group_x, u32 group_y, u32 group_z)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();

        vkCmdDispatch(impl.vk_cmd_buffer, group_x, group_y, group_z);
    }

    void CommandList::dispatch_indirect(DispatchIndirectInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();

        vkCmdDispatchIndirect(impl.vk_cmd_buffer, impl.impl_device.as<ImplDevice>()->slot(info.indirect_buffer).vk_buffer, info.offset);
    }

    void defer_destruction_helper(void * impl_void, GPUResourceId id, u8 index)
    {
        auto & impl = *reinterpret_cast<ImplCommandList *>(impl_void);
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        DAXA_DBG_ASSERT_TRUE_M(impl.deferred_destruction_count < DEFERRED_DESTRUCTION_COUNT_MAX, "can not defer the destruction of more than 32 resources per command list recording");
        impl.flush_barriers();

        impl.deferred_destructions[impl.deferred_destruction_count++] = {id, index};
    }

    void CommandList::destroy_buffer_deferred(BufferId id)
    {
        defer_destruction_helper(this->object, GPUResourceId{.index = id.index, .version = id.version}, DEFERRED_DESTRUCTION_BUFFER_INDEX);
    }

    void CommandList::destroy_image_deferred(ImageId id)
    {
        defer_destruction_helper(object, GPUResourceId{.index = id.index, .version = id.version}, DEFERRED_DESTRUCTION_IMAGE_INDEX);
    }

    void CommandList::destroy_image_view_deferred(ImageViewId id)
    {
        defer_destruction_helper(object, GPUResourceId{.index = id.index, .version = id.version}, DEFERRED_DESTRUCTION_IMAGE_VIEW_INDEX);
    }

    void CommandList::destroy_sampler_deferred(SamplerId id)
    {
        defer_destruction_helper(object, GPUResourceId{.index = id.index, .version = id.version}, DEFERRED_DESTRUCTION_SAMPLER_INDEX);
    }

    void CommandList::complete()
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();

        impl.recording_complete = true;

        vkEndCommandBuffer(impl.vk_cmd_buffer);
    }

    auto CommandList::is_complete() const -> bool
    {
        auto & impl = *as<ImplCommandList>();
        return impl.recording_complete;
    }

    auto CommandList::info() const -> CommandListInfo const &
    {
        auto & impl = *as<ImplCommandList>();
        return impl.info;
    }

    void CommandList::pipeline_barrier(MemoryBarrierInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();

        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");

        if (impl.memory_barrier_batch_count == COMMAND_LIST_BARRIER_MAX_BATCH_SIZE)
        {
            impl.flush_barriers();
        }

        impl.memory_barrier_batch[impl.memory_barrier_batch_count++] = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = static_cast<u64>(info.awaited_pipeline_access.stages),
            .srcAccessMask = static_cast<u32>(info.awaited_pipeline_access.type),
            .dstStageMask = static_cast<u64>(info.waiting_pipeline_access.stages),
            .dstAccessMask = static_cast<u32>(info.waiting_pipeline_access.type),
        };
    }

    struct SplitBarrierDependencyInfoBuffer
    {
        std::vector<VkImageMemoryBarrier2> vk_image_memory_barriers = {};
        std::vector<VkMemoryBarrier2> vk_memory_barriers = {};
    };

    inline static thread_local std::vector<SplitBarrierDependencyInfoBuffer> split_barrier_dependency_infos_aux_buffer = {};
    inline static thread_local std::vector<VkDependencyInfo> split_barrier_dependency_infos_buffer = {};
    inline static thread_local std::vector<VkEvent> split_barrier_events_buffer = {};

    void CommandList::wait_split_barriers(std::span<SplitBarrierEndInfo const> const & infos)
    {
        auto & impl = *this->as<ImplCommandList>();
        auto & device = *impl.impl_device.as<ImplDevice>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");
        impl.flush_barriers();

        for (auto const & end_info : infos)
        {
            split_barrier_dependency_infos_buffer.push_back({});
            auto & dependency_infos_aux_buffer = split_barrier_dependency_infos_aux_buffer.back();

            for (auto & image_barrier : end_info.image_barriers)
            {
                dependency_infos_aux_buffer.vk_image_memory_barriers.push_back(
                    get_vk_image_memory_barrier(image_barrier, device.slot(image_barrier.image_id).vk_image));
            }

            for (auto const & memory_barrier : end_info.memory_barriers)
            {
                dependency_infos_aux_buffer.vk_memory_barriers.push_back(get_vk_memory_barrier(memory_barrier));
            }

            split_barrier_dependency_infos_buffer.push_back(get_vk_dependency_info(
                dependency_infos_aux_buffer.vk_image_memory_barriers,
                dependency_infos_aux_buffer.vk_memory_barriers));

            split_barrier_events_buffer.push_back(reinterpret_cast<VkEvent>(end_info.split_barrier.data));
        }

        vkCmdWaitEvents2(
            impl.vk_cmd_buffer,
            static_cast<u32>(split_barrier_events_buffer.size()),
            split_barrier_events_buffer.data(),
            split_barrier_dependency_infos_buffer.data());

        split_barrier_dependency_infos_aux_buffer.clear();
        split_barrier_dependency_infos_buffer.clear();
        split_barrier_events_buffer.clear();
    }

    void CommandList::wait_split_barrier(SplitBarrierEndInfo const & info)
    {
        this->wait_split_barriers(std::span{&info, 1});
    }

    void CommandList::signal_split_barrier(SplitBarrierStartInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        auto & device = *impl.impl_device.as<ImplDevice>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");
        impl.flush_barriers();

        split_barrier_dependency_infos_buffer.push_back({});
        auto & dependency_infos_aux_buffer = split_barrier_dependency_infos_aux_buffer.back();

        for (auto & image_barrier : info.image_barriers)
        {
            dependency_infos_aux_buffer.vk_image_memory_barriers.push_back(
                get_vk_image_memory_barrier(image_barrier, device.slot(image_barrier.image_id).vk_image));
        }
        for (auto & memory_barrier : info.memory_barriers)
        {
            dependency_infos_aux_buffer.vk_memory_barriers.push_back(get_vk_memory_barrier(memory_barrier));
        }

        VkDependencyInfo vk_dependency_info = get_vk_dependency_info(
            dependency_infos_aux_buffer.vk_image_memory_barriers,
            dependency_infos_aux_buffer.vk_memory_barriers);

        vkCmdSetEvent2(impl.vk_cmd_buffer, reinterpret_cast<VkEvent>(info.split_barrier.data), &vk_dependency_info);
        split_barrier_dependency_infos_aux_buffer.clear();
    }

    void CommandList::pipeline_barrier_image_transition(ImageBarrierInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();

        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");

        if (impl.image_barrier_batch_count == COMMAND_LIST_BARRIER_MAX_BATCH_SIZE)
        {
            impl.flush_barriers();
        }

        impl.image_barrier_batch[impl.image_barrier_batch_count++] = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = static_cast<u64>(info.awaited_pipeline_access.stages),
            .srcAccessMask = static_cast<u32>(info.awaited_pipeline_access.type),
            .dstStageMask = static_cast<u64>(info.waiting_pipeline_access.stages),
            .dstAccessMask = static_cast<u32>(info.waiting_pipeline_access.type),
            .oldLayout = static_cast<VkImageLayout>(info.before_layout),
            .newLayout = static_cast<VkImageLayout>(info.after_layout),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = impl.impl_device.as<ImplDevice>()->slot(info.image_id).vk_image,
            .subresourceRange = *reinterpret_cast<VkImageSubresourceRange const *>(&info.image_slice),
        };
    }

    void CommandList::begin_renderpass(RenderPassBeginInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();

        auto fill_rendering_attachment_info = [&](RenderAttachmentInfo const & in, VkRenderingAttachmentInfo & out)
        {
            DAXA_DBG_ASSERT_TRUE_M(!in.image_view.is_empty(), "must provide either image view to render attachment");
            VkImageView vk_image_view = VK_NULL_HANDLE;

            out = VkRenderingAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .pNext = nullptr,
                .imageView = impl.impl_device.as<ImplDevice>()->slot(in.image_view).vk_image_view,
                .imageLayout = *reinterpret_cast<VkImageLayout const *>(&in.layout),
                .resolveMode = VkResolveModeFlagBits::VK_RESOLVE_MODE_NONE,
                .resolveImageView = VK_NULL_HANDLE,
                .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .loadOp = static_cast<VkAttachmentLoadOp>(in.load_op),
                .storeOp = static_cast<VkAttachmentStoreOp>(in.store_op),
                .clearValue = *reinterpret_cast<VkClearValue const *>(&in.clear_value),
            };
        };

        DAXA_DBG_ASSERT_TRUE_M(info.color_attachments.size() <= COMMAND_LIST_COLOR_ATTACHMENT_MAX, "too many color attachments, make pull request to bump maximum");
        std::array<VkRenderingAttachmentInfo, COMMAND_LIST_COLOR_ATTACHMENT_MAX> vk_color_attachments = {};

        for (usize i = 0; i < info.color_attachments.size(); ++i)
        {
            fill_rendering_attachment_info(info.color_attachments[i], vk_color_attachments[i]);
        }

        VkRenderingAttachmentInfo depth_attachment_info = {};
        if (info.depth_attachment.has_value())
        {
            fill_rendering_attachment_info(info.depth_attachment.value(), depth_attachment_info);
        };

        VkRenderingAttachmentInfo stencil_attachment_info = {};
        if (info.stencil_attachment.has_value())
        {
            fill_rendering_attachment_info(info.stencil_attachment.value(), stencil_attachment_info);
        };

        VkRenderingInfo vk_rendering_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .pNext = nullptr,
            .flags = {},
            .renderArea = *reinterpret_cast<VkRect2D const *>(&info.render_area),
            .layerCount = 1,
            .viewMask = {},
            .colorAttachmentCount = static_cast<u32>(info.color_attachments.size()),
            .pColorAttachments = vk_color_attachments.data(),
            .pDepthAttachment = info.depth_attachment.has_value() ? &depth_attachment_info : nullptr,
            .pStencilAttachment = info.stencil_attachment.has_value() ? &stencil_attachment_info : nullptr,
        };

        vkCmdSetScissor(impl.vk_cmd_buffer, 0, 1, reinterpret_cast<VkRect2D const *>(&info.render_area));

        VkViewport vk_viewport = {
            .x = static_cast<f32>(info.render_area.x),
            .y = static_cast<f32>(info.render_area.y),
            .width = static_cast<f32>(info.render_area.width),
            .height = static_cast<f32>(info.render_area.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        vkCmdSetViewport(impl.vk_cmd_buffer, 0, 1, &vk_viewport);

        vkCmdBeginRendering(impl.vk_cmd_buffer, &vk_rendering_info);
    }

    void CommandList::end_renderpass()
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();
        vkCmdEndRendering(impl.vk_cmd_buffer);
    }

    void CommandList::set_viewport(ViewportInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();
        vkCmdSetViewport(impl.vk_cmd_buffer, 0, 1, reinterpret_cast<VkViewport const *>(&info));
    }

    void CommandList::set_scissor(Rect2D const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();
        vkCmdSetScissor(impl.vk_cmd_buffer, 0, 1, reinterpret_cast<VkRect2D const *>(&info));
    }

    void CommandList::set_index_buffer(BufferId id, usize offset, usize index_type_byte_size)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();

        VkIndexType vk_index_type = {};
        switch (index_type_byte_size)
        {
        case 2: vk_index_type = VK_INDEX_TYPE_UINT16; break;
        case 4: vk_index_type = VK_INDEX_TYPE_UINT32; break;
        default: DAXA_DBG_ASSERT_TRUE_M(false, "only index byte sizes 2 and 4 are supported");
        }
        vkCmdBindIndexBuffer(impl.vk_cmd_buffer, impl.impl_device.as<ImplDevice>()->slot(id).vk_buffer, static_cast<VkDeviceSize>(offset), vk_index_type);
    }

    void CommandList::draw(DrawInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();
        vkCmdDraw(impl.vk_cmd_buffer, info.vertex_count, info.instance_count, info.first_vertex, info.first_instance);
    }

    void CommandList::draw_indexed(DrawIndexedInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();
        vkCmdDrawIndexed(impl.vk_cmd_buffer, info.index_count, info.instance_count, info.first_index, info.vertex_offset, info.first_instance);
    }

    void CommandList::draw_indirect(DrawIndirectInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();
        vkCmdDrawIndirect(impl.vk_cmd_buffer, impl.impl_device.as<ImplDevice>()->slot(info.indirect_buffer).vk_buffer, info.offset, info.draw_count, info.stride);
    }

    void CommandList::write_timestamp(WriteTimestampInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(info.query_index < info.query_pool.info().query_count, "query_index is out of bounds for the query pool");
        impl.flush_barriers();
        vkCmdWriteTimestamp(impl.vk_cmd_buffer, static_cast<VkPipelineStageFlagBits>(info.pipeline_stage), info.query_pool.as<ImplTimelineQueryPool>()->vk_timeline_query_pool, info.query_index);
    }

    void CommandList::reset_timestamps(ResetTimestampsInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(info.start_index < info.query_pool.info().query_count, "reset index is out of bounds for the query pool");
        impl.flush_barriers();
        vkCmdResetQueryPool(impl.vk_cmd_buffer, info.query_pool.as<ImplTimelineQueryPool>()->vk_timeline_query_pool, info.start_index, info.count);
    }

    auto CommandBufferPoolPool::get(ImplDevice * device) -> std::pair<VkCommandPool, VkCommandBuffer>
    {
        std::pair<VkCommandPool, VkCommandBuffer> pair = {};
        if (pools_and_buffers.empty())
        {
            VkCommandPool pool = {};
            VkCommandBuffer buffer = {};
            VkCommandPoolCreateInfo vk_command_pool_create_info{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = device->main_queue_family_index,
            };

            vkCreateCommandPool(device->vk_device, &vk_command_pool_create_info, nullptr, &pool);

            VkCommandBufferAllocateInfo vk_command_buffer_allocate_info{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .pNext = nullptr,
                .commandPool = pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };

            vkAllocateCommandBuffers(device->vk_device, &vk_command_buffer_allocate_info, &buffer);
            pair = {pool, buffer};
        }
        else
        {
            pair = pools_and_buffers.back();
            pools_and_buffers.pop_back();
        }
        return pair;
    }

    void CommandBufferPoolPool::put_back(std::pair<VkCommandPool, VkCommandBuffer> pool_and_buffer)
    {
        pools_and_buffers.push_back(pool_and_buffer);
    }

    void CommandBufferPoolPool::cleanup(ImplDevice * device)
    {
        for (auto [pool, buffer] : pools_and_buffers)
        {
            vkDestroyCommandPool(device->vk_device, pool, nullptr);
        }
        pools_and_buffers.clear();
    }

    void ImplCommandList::flush_barriers()
    {
        if (memory_barrier_batch_count > 0 || image_barrier_batch_count > 0)
        {
            VkDependencyInfo vk_dependency_info{
                .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .pNext = nullptr,
                .dependencyFlags = {},
                .memoryBarrierCount = static_cast<u32>(memory_barrier_batch_count),
                .pMemoryBarriers = memory_barrier_batch.data(),
                .bufferMemoryBarrierCount = 0,
                .pBufferMemoryBarriers = nullptr,
                .imageMemoryBarrierCount = static_cast<u32>(image_barrier_batch_count),
                .pImageMemoryBarriers = image_barrier_batch.data(),
            };

            vkCmdPipelineBarrier2(vk_cmd_buffer, &vk_dependency_info);

            memory_barrier_batch_count = 0;
            image_barrier_batch_count = 0;
        }
    }

    ImplCommandList::ImplCommandList(ManagedWeakPtr a_impl_device, VkCommandPool pool, VkCommandBuffer buffer, CommandListInfo const & a_info)
        : impl_device{std::move(a_impl_device)}, pipeline_layouts{&(impl_device.as<ImplDevice>()->gpu_table.pipeline_layouts)}, vk_cmd_pool{pool}, vk_cmd_buffer{buffer}
    {
        initialize(a_info);
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

        if (this->impl_device.as<ImplDevice>()->impl_ctx.as<ImplContext>()->enable_debug_names && this->info.debug_name.size() > 0)
        {
            auto cmd_buffer_name = this->info.debug_name + std::string(" [Daxa CommandBuffer]");
            VkDebugUtilsObjectNameInfoEXT cmd_buffer_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_COMMAND_BUFFER,
                .objectHandle = reinterpret_cast<uint64_t>(this->vk_cmd_buffer),
                .pObjectName = cmd_buffer_name.c_str(),
            };
            this->impl_device.as<ImplDevice>()->vkSetDebugUtilsObjectNameEXT(this->impl_device.as<ImplDevice>()->vk_device, &cmd_buffer_name_info);

            auto cmd_pool_name = this->info.debug_name + std::string(" [Daxa CommandPool]");
            VkDebugUtilsObjectNameInfoEXT cmd_pool_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_COMMAND_POOL,
                .objectHandle = reinterpret_cast<uint64_t>(this->vk_cmd_pool),
                .pObjectName = cmd_pool_name.c_str(),
            };
            this->impl_device.as<ImplDevice>()->vkSetDebugUtilsObjectNameEXT(this->impl_device.as<ImplDevice>()->vk_device, &cmd_pool_name_info);
        }
    }

    void ImplCommandList::reset()
    {
        vkResetCommandPool(impl_device.as<ImplDevice>()->vk_device, this->vk_cmd_pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
        deferred_destruction_count = 0;
    }

    ImplCommandList::~ImplCommandList()
    {
        auto device = this->impl_device.as<ImplDevice>();

        vkResetCommandPool(device->vk_device, this->vk_cmd_pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

        DAXA_ONLY_IF_THREADSAFETY(std::unique_lock lock{device->main_queue_zombies_mtx});
        u64 main_queue_cpu_timeline = DAXA_ATOMIC_FETCH(device->main_queue_cpu_timeline);

        device->main_queue_command_list_zombies.push_front({
            main_queue_cpu_timeline,
            CommandListZombie{
                .vk_cmd_buffer = vk_cmd_buffer,
                .vk_cmd_pool = vk_cmd_pool,
            },
        });
    }
} // namespace daxa
