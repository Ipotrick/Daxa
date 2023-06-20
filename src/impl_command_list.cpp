#include "impl_command_list.hpp"

#include <utility>

#include "impl_split_barrier.hpp"
#include "impl_device.hpp"

namespace daxa
{
    auto get_vk_image_memory_barrier(ImageBarrierInfo const & image_barrier, VkImage vk_image) -> VkImageMemoryBarrier2
    {
        return VkImageMemoryBarrier2{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = image_barrier.src_access.stages.data,
            .srcAccessMask = image_barrier.src_access.type.data,
            .dstStageMask = image_barrier.dst_access.stages.data,
            .dstAccessMask = image_barrier.dst_access.type.data,
            .oldLayout = static_cast<VkImageLayout>(image_barrier.src_layout),
            .newLayout = static_cast<VkImageLayout>(image_barrier.dst_layout),
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
            .srcStageMask = memory_barrier.src_access.stages.data,
            .srcAccessMask = memory_barrier.src_access.type.data,
            .dstStageMask = memory_barrier.dst_access.stages.data,
            .dstAccessMask = memory_barrier.dst_access.type.data,
        };
    }

    auto get_vk_dependency_info(
        std::vector<VkImageMemoryBarrier2> const & vk_image_memory_barriers,
        std::vector<VkMemoryBarrier2> const & vk_memory_barriers) -> VkDependencyInfo
    {
        return VkDependencyInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext = nullptr,
            .dependencyFlags = {},
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

        VkBufferCopy const vk_buffer_copy{
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

    void CommandList::copy_buffer_to_image(BufferImageCopyInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();

        VkBufferImageCopy const vk_buffer_image_copy{
            .bufferOffset = info.buffer_offset,
            .bufferRowLength = 0u,   // info.image_extent.x,
            .bufferImageHeight = 0u, // info.image_extent.y,
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

    void CommandList::copy_image_to_buffer(ImageBufferCopyInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();

        VkBufferImageCopy const vk_buffer_image_copy{
            .bufferOffset = info.buffer_offset,
            .bufferRowLength = 0u,   // info.image_extent.x,
            .bufferImageHeight = 0u, // info.image_extent.y,
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

        VkImageBlit const vk_blit{
            .srcSubresource = *reinterpret_cast<VkImageSubresourceLayers const *>(&info.src_slice),
            .srcOffsets = {*reinterpret_cast<VkOffset3D const *>(info.src_offsets.data()), *reinterpret_cast<VkOffset3D const *>(&info.src_offsets[1])},
            .dstSubresource = *reinterpret_cast<VkImageSubresourceLayers const *>(&info.dst_slice),
            .dstOffsets = {*reinterpret_cast<VkOffset3D const *>(info.dst_offsets.data()), *reinterpret_cast<VkOffset3D const *>(&info.dst_offsets[1])},
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

        VkImageCopy const vk_image_copy{
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

        if ((info.dst_slice.image_aspect & ImageAspectFlagBits::COLOR) != ImageAspectFlagBits::NONE)
        {
            DAXA_DBG_ASSERT_TRUE_M(
                !std::holds_alternative<DepthValue>(info.clear_value),
                "Provided a depth clear value for an image with a color aspect!");

            VkClearColorValue color;

            std::visit(
                [&color](auto && clear_value)
                {
                    using T = std::decay_t<decltype(clear_value)>;
                    if constexpr (std::is_same_v<T, std::array<f32, 4>>)
                    {
                        color = {.float32 = {clear_value[0], clear_value[1], clear_value[2], clear_value[3]}};
                    }
                    else if constexpr (std::is_same_v<T, std::array<i32, 4>>)
                    {
                        color = {.int32 = {clear_value[0], clear_value[1], clear_value[2], clear_value[3]}};
                    }
                    else if constexpr (std::is_same_v<T, std::array<u32, 4>>)
                    {
                        color = {.uint32 = {clear_value[0], clear_value[1], clear_value[2], clear_value[3]}};
                    }
                },
                info.clear_value);

            vkCmdClearColorImage(
                impl.vk_cmd_buffer,
                impl.impl_device.as<ImplDevice>()->slot(info.dst_image).vk_image,
                static_cast<VkImageLayout>(info.dst_image_layout),
                &color,
                1,
                reinterpret_cast<VkImageSubresourceRange const *>(&info.dst_slice));
        }

        if ((info.dst_slice.image_aspect & (ImageAspectFlagBits::DEPTH | ImageAspectFlagBits::STENCIL)) != ImageAspectFlagBits::NONE)
        {
            DAXA_DBG_ASSERT_TRUE_M(
                std::holds_alternative<DepthValue>(info.clear_value),
                "Provided a color clear value for an image with a depth / stencil aspect!");

            auto const & clear_value = std::get<DepthValue>(info.clear_value);
            VkClearDepthStencilValue const color{
                .depth = clear_value.depth,
                .stencil = clear_value.stencil,
            };

            vkCmdClearDepthStencilImage(
                impl.vk_cmd_buffer,
                impl.impl_device.as<ImplDevice>()->slot(info.dst_image).vk_image,
                static_cast<VkImageLayout>(info.dst_image_layout),
                &color,
                1,
                reinterpret_cast<VkImageSubresourceRange const *>(&info.dst_slice));
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

    void CommandList::push_constant_vptr(void const * data, u32 size, u32 offset)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        DAXA_DBG_ASSERT_TRUE_M(size <= MAX_PUSH_CONSTANT_BYTE_SIZE, MAX_PUSH_CONSTANT_SIZE_ERROR);
        DAXA_DBG_ASSERT_TRUE_M(size % 4 == 0, "push constant size must be a multiple of 4 bytes");
        impl.flush_barriers();

        vkCmdPushConstants(impl.vk_cmd_buffer, (*impl.pipeline_layouts).at((size + 3) / 4), VK_SHADER_STAGE_ALL, offset, size, data);
    }

    void CommandList::set_pipeline(ComputePipeline const & pipeline)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(pipeline.object != nullptr, "invalid pipeline handle - valid handle must be retrieved from the pipeline compiler before use");
        auto const & pipeline_impl = *pipeline.as<ImplComputePipeline>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();

        impl.flush_constant_buffer_bindings(VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_impl.vk_pipeline_layout);

        vkCmdBindDescriptorSets(impl.vk_cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_impl.vk_pipeline_layout, 0, 1, &impl.impl_device.as<ImplDevice>()->gpu_shader_resource_table.vk_descriptor_set, 0, nullptr);

        vkCmdBindPipeline(impl.vk_cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_impl.vk_pipeline);
    }

    void CommandList::set_uniform_buffer(SetConstantBufferInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        auto & impl_device = *impl.impl_device.as<ImplDevice>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        DAXA_DBG_ASSERT_TRUE_M(info.size > 0, "the set constant buffer range must be greater then 0");
        DAXA_DBG_ASSERT_TRUE_M(info.offset % impl_device.vk_info.limits.min_uniform_buffer_offset_alignment == 0, "must respect the alignment requirements of uniform buffer bindings for constant buffer offsets!");
        const usize buffer_size = impl_device.slot(info.buffer).info.size;
        [[maybe_unused]] const bool binding_in_range = info.size + info.offset <= buffer_size;
        DAXA_DBG_ASSERT_TRUE_M(binding_in_range, "The given offset and size of the buffer binding is outside of the bounds of the given buffer");
        DAXA_DBG_ASSERT_TRUE_M(info.slot < CONSTANT_BUFFER_BINDINGS_COUNT, "there are only 8 binding slots available for constant buffers");
        impl.current_constant_buffer_bindings[info.slot] = info;
    }

    void CommandList::set_pipeline(RasterPipeline const & pipeline)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(pipeline.object != nullptr, "invalid pipeline handle - valid handle must be retrieved from the pipeline compiler before use");
        auto const & pipeline_impl = *pipeline.as<ImplRasterPipeline>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();

        vkCmdBindDescriptorSets(impl.vk_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_impl.vk_pipeline_layout, 0, 1, &impl.impl_device.as<ImplDevice>()->gpu_shader_resource_table.vk_descriptor_set, 0, nullptr);

        impl.flush_constant_buffer_bindings(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_impl.vk_pipeline_layout);

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
        // DAXA_DBG_ASSERT_TRUE_M(impl.deferred_destruction_count < DEFERRED_DESTRUCTION_COUNT_MAX, "can not defer the destruction of more than 32 resources per command list recording");
        impl.deferred_destructions.emplace_back(id, index);
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
        auto const & impl = *as<ImplCommandList>();
        return impl.recording_complete;
    }

    auto CommandList::info() const -> CommandListInfo const &
    {
        auto const & impl = *as<ImplCommandList>();
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

        impl.memory_barrier_batch.at(impl.memory_barrier_batch_count++) = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = info.src_access.stages.data,
            .srcAccessMask = info.src_access.type.data,
            .dstStageMask = info.dst_access.stages.data,
            .dstAccessMask = info.dst_access.type.data,
        };
    }

    struct SplitBarrierDependencyInfoBuffer
    {
        std::vector<VkImageMemoryBarrier2> vk_image_memory_barriers = {};
        std::vector<VkMemoryBarrier2> vk_memory_barriers = {};
    };

    inline static thread_local std::vector<SplitBarrierDependencyInfoBuffer> tl_split_barrier_dependency_infos_aux_buffer = {}; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    inline static thread_local std::vector<VkDependencyInfo> tl_split_barrier_dependency_infos_buffer = {};                     // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    inline static thread_local std::vector<VkEvent> tl_split_barrier_events_buffer = {};                                        // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    void CommandList::wait_split_barriers(std::span<SplitBarrierWaitInfo const> const & infos)
    {
        auto & impl = *this->as<ImplCommandList>();
        auto & device = *impl.impl_device.as<ImplDevice>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");
        impl.flush_barriers();
        for (auto const & end_info : infos)
        {
            tl_split_barrier_dependency_infos_aux_buffer.push_back({});
            auto & dependency_infos_aux_buffer = tl_split_barrier_dependency_infos_aux_buffer.back();
            for (auto & image_barrier : end_info.image_barriers)
            {
                dependency_infos_aux_buffer.vk_image_memory_barriers.push_back(
                    get_vk_image_memory_barrier(image_barrier, device.slot(image_barrier.image_id).vk_image));
            }
            for (auto const & memory_barrier : end_info.memory_barriers)
            {
                dependency_infos_aux_buffer.vk_memory_barriers.push_back(get_vk_memory_barrier(memory_barrier));
            }
            tl_split_barrier_dependency_infos_buffer.push_back(get_vk_dependency_info(
                dependency_infos_aux_buffer.vk_image_memory_barriers,
                dependency_infos_aux_buffer.vk_memory_barriers));

            tl_split_barrier_events_buffer.push_back(reinterpret_cast<VkEvent>(end_info.split_barrier.data));
        }
        vkCmdWaitEvents2(
            impl.vk_cmd_buffer,
            static_cast<u32>(tl_split_barrier_events_buffer.size()),
            tl_split_barrier_events_buffer.data(),
            tl_split_barrier_dependency_infos_buffer.data());
        tl_split_barrier_dependency_infos_aux_buffer.clear();
        tl_split_barrier_dependency_infos_buffer.clear();
        tl_split_barrier_events_buffer.clear();
    }

    void CommandList::wait_split_barrier(SplitBarrierWaitInfo const & info)
    {
        this->wait_split_barriers(std::span{&info, 1});
    }

    void CommandList::signal_split_barrier(SplitBarrierSignalInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        auto & device = *impl.impl_device.as<ImplDevice>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");
        impl.flush_barriers();

        tl_split_barrier_dependency_infos_aux_buffer.push_back({});
        auto & dependency_infos_aux_buffer = tl_split_barrier_dependency_infos_aux_buffer.back();

        for (auto & image_barrier : info.image_barriers)
        {
            dependency_infos_aux_buffer.vk_image_memory_barriers.push_back(
                get_vk_image_memory_barrier(image_barrier, device.slot(image_barrier.image_id).vk_image));
        }
        for (auto & memory_barrier : info.memory_barriers)
        {
            dependency_infos_aux_buffer.vk_memory_barriers.push_back(get_vk_memory_barrier(memory_barrier));
        }

        VkDependencyInfo const vk_dependency_info = get_vk_dependency_info(
            dependency_infos_aux_buffer.vk_image_memory_barriers,
            dependency_infos_aux_buffer.vk_memory_barriers);

        vkCmdSetEvent2(impl.vk_cmd_buffer, reinterpret_cast<VkEvent>(info.split_barrier.data), &vk_dependency_info);
        tl_split_barrier_dependency_infos_aux_buffer.clear();
    }

    void CommandList::reset_split_barrier(ResetSplitBarrierInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");
        impl.flush_barriers();
        vkCmdResetEvent2(
            impl.vk_cmd_buffer,
            reinterpret_cast<VkEvent>(info.barrier.data),
            info.stage_masks.data);
    }

    void CommandList::pipeline_barrier_image_transition(ImageBarrierInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");

        if (impl.image_barrier_batch_count == COMMAND_LIST_BARRIER_MAX_BATCH_SIZE)
        {
            impl.flush_barriers();
        }

        impl.image_barrier_batch.at(impl.image_barrier_batch_count++) = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = info.src_access.stages.data,
            .srcAccessMask = info.src_access.type.data,
            .dstStageMask = info.dst_access.stages.data,
            .dstAccessMask = info.dst_access.type.data,
            .oldLayout = static_cast<VkImageLayout>(info.src_layout),
            .newLayout = static_cast<VkImageLayout>(info.dst_layout),
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

            VkClearValue clear_value{};
            std::visit(
                [&clear_value](auto && daxa_clear_value)
                {
                    using T = std::decay_t<decltype(daxa_clear_value)>;
                    if constexpr (std::is_same_v<T, std::array<f32, 4>>)
                    {
                        clear_value = {.color = {.float32 = {daxa_clear_value[0], daxa_clear_value[1], daxa_clear_value[2], daxa_clear_value[3]}}};
                    }
                    else if constexpr (std::is_same_v<T, std::array<i32, 4>>)
                    {
                        clear_value = {.color = {.int32 = {daxa_clear_value[0], daxa_clear_value[1], daxa_clear_value[2], daxa_clear_value[3]}}};
                    }
                    else if constexpr (std::is_same_v<T, std::array<u32, 4>>)
                    {
                        clear_value = {.color = {.uint32 = {daxa_clear_value[0], daxa_clear_value[1], daxa_clear_value[2], daxa_clear_value[3]}}};
                    }
                    else if constexpr (std::is_same_v<T, DepthValue>)
                    {
                        clear_value = {.depthStencil = {.depth = daxa_clear_value.depth, .stencil = daxa_clear_value.stencil}};
                    }
                },
                in.clear_value);

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
                .clearValue = clear_value,
            };
        };

        DAXA_DBG_ASSERT_TRUE_M(info.color_attachments.size() <= COMMAND_LIST_COLOR_ATTACHMENT_MAX, "too many color attachments, make pull request to bump maximum");
        std::array<VkRenderingAttachmentInfo, COMMAND_LIST_COLOR_ATTACHMENT_MAX> vk_color_attachments = {};

        for (usize i = 0; i < info.color_attachments.size(); ++i)
        {
            fill_rendering_attachment_info(info.color_attachments[i], vk_color_attachments.at(i));
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

        VkRenderingInfo const vk_rendering_info{
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

        VkViewport const vk_viewport = {
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

    void CommandList::set_depth_bias(DepthBiasInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();
        vkCmdSetDepthBias(impl.vk_cmd_buffer, info.constant_factor, info.clamp, info.slope_factor);
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
        vkCmdBindIndexBuffer(impl.vk_cmd_buffer, impl.impl_device.as<ImplDevice>()->slot(id).vk_buffer, offset, vk_index_type);
    }

    void CommandList::draw(DrawInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only record to uncompleted command list");
        vkCmdDraw(impl.vk_cmd_buffer, info.vertex_count, info.instance_count, info.first_vertex, info.first_instance);
    }

    void CommandList::draw_indexed(DrawIndexedInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only record to uncompleted command list");
        vkCmdDrawIndexed(impl.vk_cmd_buffer, info.index_count, info.instance_count, info.first_index, info.vertex_offset, info.first_instance);
    }

    void CommandList::draw_indirect(DrawIndirectInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only record to uncompleted command list");
        if (info.is_indexed)
        {
            vkCmdDrawIndexedIndirect(
                impl.vk_cmd_buffer,
                impl.impl_device.as<ImplDevice>()->slot(info.draw_command_buffer).vk_buffer,
                info.draw_command_buffer_read_offset,
                info.draw_count,
                info.draw_command_stride);
        }
        else
        {
            vkCmdDrawIndirect(
                impl.vk_cmd_buffer,
                impl.impl_device.as<ImplDevice>()->slot(info.draw_command_buffer).vk_buffer,
                info.draw_command_buffer_read_offset,
                info.draw_count,
                info.draw_command_stride);
        }
    }

    void CommandList::draw_indirect_count(DrawIndirectCountInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only record to uncompleted command list");
        if (info.is_indexed)
        {
            vkCmdDrawIndexedIndirectCount(
                impl.vk_cmd_buffer,
                impl.impl_device.as<ImplDevice>()->slot(info.draw_command_buffer).vk_buffer,
                info.draw_command_buffer_read_offset,
                impl.impl_device.as<ImplDevice>()->slot(info.draw_count_buffer).vk_buffer,
                info.draw_count_buffer_read_offset,
                info.max_draw_count,
                info.draw_command_stride);
        }
        else
        {
            vkCmdDrawIndirectCount(
                impl.vk_cmd_buffer,
                impl.impl_device.as<ImplDevice>()->slot(info.draw_command_buffer).vk_buffer,
                info.draw_command_buffer_read_offset,
                impl.impl_device.as<ImplDevice>()->slot(info.draw_count_buffer).vk_buffer,
                info.draw_count_buffer_read_offset,
                info.max_draw_count,
                info.draw_command_stride);
        }
    }

    void CommandList::write_timestamp(WriteTimestampInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only record to uncompleted command list");
        DAXA_DBG_ASSERT_TRUE_M(info.query_index < info.query_pool.info().query_count, "query_index is out of bounds for the query pool");
        impl.flush_barriers();
        vkCmdWriteTimestamp(impl.vk_cmd_buffer, static_cast<VkPipelineStageFlagBits>(info.pipeline_stage.data), info.query_pool.as<ImplTimelineQueryPool>()->vk_timeline_query_pool, info.query_index);
    }

    void CommandList::reset_timestamps(ResetTimestampsInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only record to uncompleted command list");
        DAXA_DBG_ASSERT_TRUE_M(info.start_index < info.query_pool.info().query_count, "reset index is out of bounds for the query pool");
        impl.flush_barriers();
        vkCmdResetQueryPool(impl.vk_cmd_buffer, info.query_pool.as<ImplTimelineQueryPool>()->vk_timeline_query_pool, info.start_index, info.count);
    }

    void CommandList::begin_label(CommandLabelInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only record to uncompleted command list");
        impl.flush_barriers();
        VkDebugUtilsLabelEXT const vk_debug_label_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
            .pNext = {},
            .pLabelName = info.label_name.c_str(),
            .color = {
                info.label_color[0],
                info.label_color[1],
                info.label_color[2],
                info.label_color[3]},
        };
        impl.impl_device.as<ImplDevice>()->vkCmdBeginDebugUtilsLabelEXT(impl.vk_cmd_buffer, &vk_debug_label_info);
    }

    void CommandList::end_label()
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only record to uncompleted command list");
        impl.flush_barriers();
        impl.impl_device.as<ImplDevice>()->vkCmdEndDebugUtilsLabelEXT(impl.vk_cmd_buffer);
    }

    auto CommandBufferPoolPool::get(ImplDevice * device) -> std::pair<VkCommandPool, VkCommandBuffer>
    {
        std::pair<VkCommandPool, VkCommandBuffer> pair = {};
        if (pools_and_buffers.empty())
        {
            VkCommandPool pool = {};
            VkCommandBuffer buffer = {};
            VkCommandPoolCreateInfo const vk_command_pool_create_info{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = device->main_queue_family_index,
            };

            vkCreateCommandPool(device->vk_device, &vk_command_pool_create_info, nullptr, &pool);

            VkCommandBufferAllocateInfo const vk_command_buffer_allocate_info{
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
            VkDependencyInfo const vk_dependency_info{
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

    void ImplCommandList::flush_constant_buffer_bindings(VkPipelineBindPoint bind_point, VkPipelineLayout pipeline_layout)
    {
        auto & device = *this->impl_device.as<ImplDevice>();
        std::array<VkDescriptorBufferInfo, CONSTANT_BUFFER_BINDINGS_COUNT> descriptor_buffer_info = {};
        std::array<VkWriteDescriptorSet, CONSTANT_BUFFER_BINDINGS_COUNT> descriptor_writes = {};
        for (u32 index = 0; index < CONSTANT_BUFFER_BINDINGS_COUNT; ++index)
        {
            if (this->current_constant_buffer_bindings[index].buffer.is_empty())
            {
                descriptor_buffer_info[index] = VkDescriptorBufferInfo{
                    .buffer = VK_NULL_HANDLE,
                    .offset = {},
                    .range = VK_WHOLE_SIZE,
                };
            }
            else
            {
                descriptor_buffer_info[index] = VkDescriptorBufferInfo{
                    .buffer = device.slot(current_constant_buffer_bindings[index].buffer).vk_buffer,
                    .offset = current_constant_buffer_bindings[index].offset,
                    .range = current_constant_buffer_bindings[index].size,
                };
            }
            descriptor_writes[index] = VkWriteDescriptorSet{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = {},
                .dstSet = {}, // Not needed for push descriptors.
                .dstBinding = index,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pImageInfo = {},
                .pBufferInfo = &descriptor_buffer_info[index],
                .pTexelBufferView = {},
            };
        }

        device.vkCmdPushDescriptorSetKHR(this->vk_cmd_buffer, bind_point, pipeline_layout, CONSTANT_BUFFER_BINDING_SET, static_cast<u32>(descriptor_writes.size()), descriptor_writes.data());
        for (u32 index = 0; index < CONSTANT_BUFFER_BINDINGS_COUNT; ++index)
        {
            this->current_constant_buffer_bindings.at(index) = {};
        }
    }

    ImplCommandList::ImplCommandList(ManagedWeakPtr device_impl, VkCommandPool pool, VkCommandBuffer buffer, CommandListInfo a_info)
        : impl_device{std::move(device_impl)},
          info{std::move(a_info)},
          vk_cmd_buffer{buffer},
          vk_cmd_pool{pool},
          pipeline_layouts{&(impl_device.as<ImplDevice>()->gpu_shader_resource_table.pipeline_layouts)}
    {
        initialize();
    }

    void ImplCommandList::initialize()
    {
        VkCommandBufferBeginInfo const vk_command_buffer_begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = {},
        };

        vkBeginCommandBuffer(this->vk_cmd_buffer, &vk_command_buffer_begin_info);

        if (this->impl_device.as<ImplDevice>()->impl_ctx.as<ImplContext>()->enable_debug_names && this->info.name.empty())
        {
            auto cmd_buffer_name = this->info.name;
            VkDebugUtilsObjectNameInfoEXT const cmd_buffer_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_COMMAND_BUFFER,
                .objectHandle = reinterpret_cast<uint64_t>(this->vk_cmd_buffer),
                .pObjectName = cmd_buffer_name.c_str(),
            };
            this->impl_device.as<ImplDevice>()->vkSetDebugUtilsObjectNameEXT(this->impl_device.as<ImplDevice>()->vk_device, &cmd_buffer_name_info);

            auto cmd_pool_name = this->info.name;
            VkDebugUtilsObjectNameInfoEXT const cmd_pool_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_COMMAND_POOL,
                .objectHandle = reinterpret_cast<uint64_t>(this->vk_cmd_pool),
                .pObjectName = cmd_pool_name.c_str(),
            };
            this->impl_device.as<ImplDevice>()->vkSetDebugUtilsObjectNameEXT(this->impl_device.as<ImplDevice>()->vk_device, &cmd_pool_name_info);
        }
    }

    ImplCommandList::~ImplCommandList() // NOLINT(bugprone-exception-escape)
    {
        auto & device = *this->impl_device.as<ImplDevice>();

        vkResetCommandPool(device.vk_device, this->vk_cmd_pool, {});

        DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const lock{device.main_queue_zombies_mtx});
        u64 const main_queue_cpu_timeline = DAXA_ATOMIC_FETCH(device.main_queue_cpu_timeline);

        device.main_queue_command_list_zombies.push_front({
            main_queue_cpu_timeline,
            CommandListZombie{
                .vk_cmd_buffer = vk_cmd_buffer,
                .vk_cmd_pool = vk_cmd_pool,
            },
        });
    }
} // namespace daxa
