#include "impl_command_list.hpp"

#include <utility>

#include "impl_sync.hpp"
#include "impl_device.hpp"
#include "impl_core.hpp"

/// --- Begin Helpers ---

auto get_vk_image_memory_barrier(daxa_ImageMemoryBarrierInfo const & image_barrier, VkImage vk_image, VkImageAspectFlags aspect_flags) -> VkImageMemoryBarrier2
{
    return VkImageMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = image_barrier.src_access.stages,
        .srcAccessMask = image_barrier.src_access.access_type,
        .dstStageMask = image_barrier.dst_access.stages,
        .dstAccessMask = image_barrier.dst_access.access_type,
        .oldLayout = static_cast<VkImageLayout>(image_barrier.src_layout),
        .newLayout = static_cast<VkImageLayout>(image_barrier.dst_layout),
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = vk_image,
        .subresourceRange = make_subresource_range(image_barrier.image_slice, aspect_flags),
    };
}

auto get_vk_memory_barrier(daxa_MemoryBarrierInfo const & memory_barrier) -> VkMemoryBarrier2
{
    return VkMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = memory_barrier.src_access.stages,
        .srcAccessMask = memory_barrier.src_access.access_type,
        .dstStageMask = memory_barrier.dst_access.stages,
        .dstAccessMask = memory_barrier.dst_access.access_type,
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

auto CommandBufferPoolPool::get(daxa_Device device) -> std::pair<VkCommandPool, VkCommandBuffer>
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

void CommandBufferPoolPool::cleanup(daxa_Device device)
{
    for (auto [pool, buffer] : pools_and_buffers)
    {
        vkDestroyCommandPool(device->vk_device, pool, nullptr);
    }
    pools_and_buffers.clear();
}

/// --- End Helpers ---

/// --- Begin API Functions ---

void daxa_cmd_copy_buffer_to_buffer(daxa_CommandList self, daxa_BufferCopyInfo const * info)
{
    self->flush_barriers();
    auto vk_buffer_copy = reinterpret_cast<VkBufferCopy const *>(&info->src_offset);
    vkCmdCopyBuffer(
        self->vk_cmd_buffer,
        self->device->slot(info->src_buffer).vk_buffer,
        self->device->slot(info->dst_buffer).vk_buffer,
        1,
        vk_buffer_copy);
}

void daxa_cmd_copy_buffer_to_image(daxa_CommandList self, daxa_BufferImageCopyInfo const * info)
{
    self->flush_barriers();
    auto const & img_slot = self->device->slot(info->image);
    VkBufferImageCopy const vk_buffer_image_copy{
        .bufferOffset = info->buffer_offset,
        // TODO(general): make sense of these parameters:
        .bufferRowLength = 0u,   // self->image_extent.x,
        .bufferImageHeight = 0u, // self->image_extent.y,
        .imageSubresource = make_subresource_layers(info->image_slice, img_slot.aspect_flags),
        .imageOffset = info->image_offset,
        .imageExtent = info->image_extent,
    };
    vkCmdCopyBufferToImage(
        self->vk_cmd_buffer,
        self->device->slot(info->buffer).vk_buffer,
        img_slot.vk_image,
        static_cast<VkImageLayout>(info->image_layout),
        1,
        &vk_buffer_image_copy);
}

void daxa_cmd_copy_image_to_buffer(daxa_CommandList self, daxa_ImageBufferCopyInfo const * info)
{
    self->flush_barriers();
    auto const & img_slot = self->device->slot(info->image);
    VkBufferImageCopy const vk_buffer_image_copy{
        .bufferOffset = info->buffer_offset,
        // TODO(general): make sense of these parameters:
        .bufferRowLength = 0u,   // info.image_extent.x,
        .bufferImageHeight = 0u, // info.image_extent.y,
        .imageSubresource = make_subresource_layers(info->image_slice, img_slot.aspect_flags),
        .imageOffset = info->image_offset,
        .imageExtent = info->image_extent,
    };
    vkCmdCopyImageToBuffer(
        self->vk_cmd_buffer,
        img_slot.vk_image,
        static_cast<VkImageLayout>(info->image_layout),
        self->device->slot(info->buffer).vk_buffer,
        1,
        &vk_buffer_image_copy);
}

void daxa_cmd_copy_image_to_image(daxa_CommandList self, daxa_ImageCopyInfo const * info)
{
    self->flush_barriers();
    auto const & src_slot = self->device->slot(info->src_image);
    auto const & dst_slot = self->device->slot(info->dst_image);
    VkImageCopy const vk_image_copy{
        .srcSubresource = make_subresource_layers(info->src_slice, src_slot.aspect_flags),
        .srcOffset = {*reinterpret_cast<VkOffset3D const *>(&info->src_offset)},
        .dstSubresource = make_subresource_layers(info->dst_slice, dst_slot.aspect_flags),
        .dstOffset = {*reinterpret_cast<VkOffset3D const *>(&info->dst_offset)},
        .extent = {*reinterpret_cast<VkExtent3D const *>(&info->extent)},
    };
    vkCmdCopyImage(
        self->vk_cmd_buffer,
        src_slot.vk_image,
        static_cast<VkImageLayout>(info->src_image_layout),
        dst_slot.vk_image,
        static_cast<VkImageLayout>(info->dst_image_layout),
        1,
        &vk_image_copy);
}

void daxa_cmd_blit_image_to_image(daxa_CommandList self, daxa_ImageBlitInfo const * info)
{
    self->flush_barriers();
    auto const & src_slot = self->device->slot(info->src_image);
    auto const & dst_slot = self->device->slot(info->dst_image);
    VkImageBlit const vk_blit{
        .srcSubresource = make_subresource_layers(info->src_slice, src_slot.aspect_flags),
        .srcOffsets = {info->src_offsets[0], info->src_offsets[1]},
        .dstSubresource = make_subresource_layers(info->dst_slice, dst_slot.aspect_flags),
        .dstOffsets = {info->dst_offsets[0], info->dst_offsets[1]},
    };
    vkCmdBlitImage(
        self->vk_cmd_buffer,
        src_slot.vk_image,
        static_cast<VkImageLayout>(info->src_image_layout),
        dst_slot.vk_image,
        static_cast<VkImageLayout>(info->dst_image_layout),
        1,
        &vk_blit,
        static_cast<VkFilter>(info->filter));
}

void daxa_cmd_clear_buffer(daxa_CommandList self, daxa_BufferClearInfo const * info)
{
    self->flush_barriers();
    vkCmdFillBuffer(
        self->vk_cmd_buffer,
        self->device->slot(info->buffer).vk_buffer,
        static_cast<VkDeviceSize>(info->offset),
        static_cast<VkDeviceSize>(info->size),
        info->clear_value);
}

daxa_Result
daxa_cmd_clear_image(daxa_CommandList self, daxa_ImageClearInfo const * info)
{
    self->flush_barriers();
    auto & img_slot = self->device->slot(info->dst_image);
    bool const is_image_depth_stencil = is_depth_format(img_slot.info.format) || is_stencil_format(img_slot.info.format);
    bool const is_clear_depth_stencil = info->clear_value.index == 3;
    if (is_clear_depth_stencil)
    {
        if (!is_image_depth_stencil)
        {
            return DAXA_RESULT_INVALID_CLEAR_VALUE;
        }
        VkImageSubresourceRange sub_range = make_subresource_range(info->dst_slice, img_slot.aspect_flags);
        vkCmdClearDepthStencilImage(
            self->vk_cmd_buffer,
            img_slot.vk_image,
            static_cast<VkImageLayout>(info->dst_image_layout),
            &info->clear_value.values.depthStencil,
            1,
            &sub_range);
    }
    else
    {
        if (is_image_depth_stencil)
        {
            return DAXA_RESULT_INVALID_CLEAR_VALUE;
        }
        VkImageSubresourceRange sub_range = make_subresource_range(info->dst_slice, img_slot.aspect_flags);
        vkCmdClearColorImage(
            self->vk_cmd_buffer,
            img_slot.vk_image,
            static_cast<VkImageLayout>(info->dst_image_layout),
            &info->clear_value.values.color,
            1,
            &sub_range);
    }
}

/// @brief  Successive pipeline barrier calls are combined.
///         As soon as a non-pipeline barrier command is recorded, the currently recorded barriers are flushed with a vkCmdPipelineBarrier2 call.
/// @param info parameters.
void daxa_cmd_pipeline_barrier(daxa_CommandList self, daxa_MemoryBarrierInfo const * info)
{
    if (self->memory_barrier_batch_count == COMMAND_LIST_BARRIER_MAX_BATCH_SIZE)
    {
        self->flush_barriers();
    }
    self->memory_barrier_batch.at(self->memory_barrier_batch_count++) = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = info->src_access.stages,
        .srcAccessMask = info->src_access.access_type,
        .dstStageMask = info->dst_access.stages,
        .dstAccessMask = info->dst_access.access_type,
    };
}

/// @brief  Successive pipeline barrier calls are combined.
///         As soon as a non-pipeline barrier command is recorded, the currently recorded barriers are flushed with a vkCmdPipelineBarrier2 call.
/// @param info parameters.
void daxa_cmd_pipeline_barrier_image_transition(daxa_CommandList self, daxa_ImageMemoryBarrierInfo const * info)
{
    if (self->image_barrier_batch_count == COMMAND_LIST_BARRIER_MAX_BATCH_SIZE)
    {
        self->flush_barriers();
    }
    auto const & img_slot = self->device->slot(info->image_id);
    self->image_barrier_batch.at(self->image_barrier_batch_count++) = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = info->src_access.stages,
        .srcAccessMask = info->src_access.access_type,
        .dstStageMask = info->dst_access.stages,
        .dstAccessMask = info->dst_access.access_type,
        .oldLayout = static_cast<VkImageLayout>(info->src_layout),
        .newLayout = static_cast<VkImageLayout>(info->dst_layout),
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = img_slot.vk_image,
        .subresourceRange = make_subresource_range(info->image_slice, img_slot.aspect_flags),
    };
}
struct SplitBarrierDependencyInfoBuffer
{
    std::vector<VkImageMemoryBarrier2> vk_image_memory_barriers = {};
    std::vector<VkMemoryBarrier2> vk_memory_barriers = {};
};

inline static thread_local std::vector<SplitBarrierDependencyInfoBuffer> tl_split_barrier_dependency_infos_aux_buffer = {}; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
inline static thread_local std::vector<VkDependencyInfo> tl_split_barrier_dependency_infos_buffer = {};                     // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
inline static thread_local std::vector<VkEvent> tl_split_barrier_events_buffer = {};

void daxa_cmd_signal_event(daxa_CommandList self, daxa_EventSignalInfo const * info)
{
    self->flush_barriers();
    tl_split_barrier_dependency_infos_aux_buffer.push_back({});
    auto & dependency_infos_aux_buffer = tl_split_barrier_dependency_infos_aux_buffer.back();
    for (u64 i = 0; i < info->memory_barrier_count; ++i)
    {
        auto & memory_barrier = info->memory_barriers[i];
        dependency_infos_aux_buffer.vk_memory_barriers.push_back(get_vk_memory_barrier(memory_barrier));
    }
    for (u64 i = 0; i < info->image_memory_barrier_count; ++i)
    {
        auto & image_memory_barrier = info->image_memory_barriers[i];
        dependency_infos_aux_buffer.vk_image_memory_barriers.push_back(
            get_vk_image_memory_barrier(
                image_memory_barrier,
                self->device->slot(image_memory_barrier.image_id).vk_image,
                self->device->slot(image_memory_barrier.image_id).aspect_flags));
    }
    VkDependencyInfo const vk_dependency_info = get_vk_dependency_info(
        dependency_infos_aux_buffer.vk_image_memory_barriers,
        dependency_infos_aux_buffer.vk_memory_barriers);
    vkCmdSetEvent2(self->vk_cmd_buffer, info->event->vk_event, &vk_dependency_info);
    tl_split_barrier_dependency_infos_aux_buffer.clear();
}

void daxa_cmd_wait_events(daxa_CommandList self, daxa_EventWaitInfo const * infos, size_t info_count)
{
    self->flush_barriers();
    for (u64 i = 0; i < info_count; ++i)
    {
        auto & end_info = infos[i];
        tl_split_barrier_dependency_infos_aux_buffer.push_back({});
        auto & dependency_infos_aux_buffer = tl_split_barrier_dependency_infos_aux_buffer.back();
        for (u64 j = 0; j < end_info.memory_barrier_count; ++j)
        {
            auto & memory_barrier = end_info.memory_barriers[j];
            dependency_infos_aux_buffer.vk_memory_barriers.push_back(get_vk_memory_barrier(memory_barrier));
        }
        for (u64 j = 0; j < end_info.image_memory_barrier_count; ++j)
        {
            auto & image_barrier = end_info.image_memory_barriers[j];
            dependency_infos_aux_buffer.vk_image_memory_barriers.push_back(get_vk_image_memory_barrier(
                image_barrier,
                self->device->slot(image_barrier.image_id).vk_image,
                self->device->slot(image_barrier.image_id).aspect_flags));
        }
        tl_split_barrier_dependency_infos_buffer.push_back(get_vk_dependency_info(
            dependency_infos_aux_buffer.vk_image_memory_barriers,
            dependency_infos_aux_buffer.vk_memory_barriers));

        tl_split_barrier_events_buffer.push_back(end_info.event->vk_event);
    }
    vkCmdWaitEvents2(
        self->vk_cmd_buffer,
        static_cast<u32>(tl_split_barrier_events_buffer.size()),
        tl_split_barrier_events_buffer.data(),
        tl_split_barrier_dependency_infos_buffer.data());
    tl_split_barrier_dependency_infos_aux_buffer.clear();
    tl_split_barrier_dependency_infos_buffer.clear();
    tl_split_barrier_events_buffer.clear();
}

void daxa_cmd_wait_event(daxa_CommandList self, daxa_EventWaitInfo const * info)
{
    daxa_cmd_wait_events(self, info, 1);
}

void daxa_cmd_reset_event(daxa_CommandList self, daxa_ResetEventInfo const * info)
{
    self->flush_barriers();
    vkCmdResetEvent2(
        self->vk_cmd_buffer,
        info->barrier->vk_event,
        info->stage_masks);
}

void daxa_cmd_push_constant(daxa_CommandList self, void const * data, uint32_t size, uint32_t offset)
{
    self->flush_barriers();
    u64 layout_index = (size + sizeof(u32) - 1) / sizeof(u32);
    // TODO(general): The size can be smaller then the layouts size... Is that a problem? I remember renderdoc complaining sometimes.
    vkCmdPushConstants(self->vk_cmd_buffer, self->device->gpu_shader_resource_table.pipeline_layouts.at(layout_index), VK_SHADER_STAGE_ALL, 0, size, data);
}

/// @brief  Binds a buffer region to the uniform buffer slot.
///         There are 8 uniform buffer slots (indices range from 0 to 7).
///         The buffer range is user managed, The buffer MUST not be destroyed before the command list is submitted!
///         Changes to these bindings only become visible to commands AFTER a pipeline is bound!
///         This is in stark contrast to OpenGl like bindings which are visible immediately to all commands after binding.
///         This is deliberate to discourage overuse of uniform buffers over descriptor sets.
///         Set uniform buffer slots are cleared after a pipeline is bound.
///         Before setting another pipeline, they need to be set again.
/// @param info parameters.
void daxa_cmd_set_uniform_buffer(daxa_CommandList self, daxa_SetUniformBufferInfo const * info)
{
}

void daxa_cmd_set_compute_pipeline(daxa_CommandList self, daxa_ComputePipeline pipeline)
{
    self->flush_barriers();
    self->flush_uniform_buffer_bindings(VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->vk_pipeline_layout);
    vkCmdBindDescriptorSets(self->vk_cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->vk_pipeline_layout, 0, 1, &self->device->gpu_shader_resource_table.vk_descriptor_set, 0, nullptr);
    vkCmdBindPipeline(self->vk_cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->vk_pipeline);
}

void daxa_cmd_set_raster_pipeline(daxa_CommandList self, daxa_RasterPipeline pipeline)
{
    self->flush_barriers();
    self->flush_uniform_buffer_bindings(VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->vk_pipeline_layout);
    vkCmdBindDescriptorSets(self->vk_cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->vk_pipeline_layout, 0, 1, &self->device->gpu_shader_resource_table.vk_descriptor_set, 0, nullptr);
    vkCmdBindPipeline(self->vk_cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->vk_pipeline);
}

void daxa_cmd_dispatch(daxa_CommandList self, uint32_t x, uint32_t y, uint32_t z)
{
    vkCmdDispatch(self->vk_cmd_buffer, x, y, z);
}

void daxa_cmd_dispatch_indirect(daxa_CommandList self, daxa_DispatchIndirectInfo const * info)
{
    vkCmdDispatchIndirect(self->vk_cmd_buffer, self->device->slot(info->indirect_buffer).vk_buffer, info->offset);
}

/// @brief  Destroys the buffer AFTER the gpu is finished executing the command list.
///         Useful for large uploads exceeding staging memory pools.
/// @param id buffer to be destroyed after command list finishes.
void daxa_cmd_destroy_buffer_deferred(daxa_CommandList self, daxa_BufferId id)
{
}

/// @brief  Destroys the image AFTER the gpu is finished executing the command list.
///         Useful for large uploads exceeding staging memory pools.
/// @param id image to be destroyed after command list finishes.
void daxa_cmd_destroy_image_deferred(daxa_CommandList self, daxa_ImageId id)
{
}

/// @brief  Destroys the image view AFTER the gpu is finished executing the command list.
///         Useful for large uploads exceeding staging memory pools.
/// @param id image view to be destroyed after command list finishes.
void daxa_cmd_destroy_image_view_deferred(daxa_CommandList self, daxa_ImageViewId id)
{
}

/// @brief  Destroys the sampler AFTER the gpu is finished executing the command list.
///         Useful for large uploads exceeding staging memory pools.
/// @param id image sampler be destroyed after command list finishes.
void daxa_cmd_destroy_sampler_deferred(daxa_CommandList self, daxa_SamplerId id)
{
}

/// @brief  Starts a renderpass scope akin to the dynamic rendering feature in vulkan.
///         Between the begin and end renderpass commands, the renderpass persists and draw-calls can be recorded.
/// @param info parameters.
void daxa_cmd_begin_renderpass(daxa_CommandList self, daxa_RenderPassBeginInfo const * info)
{
}

/// @brief  Starts a renderpass scope akin to the dynamic rendering feature in vulkan.
///         Between the begin and end renderpass commands, the renderpass persists and draw-calls can be recorded.
void daxa_cmd_end_renderpass(daxa_CommandList self)
{
}

void daxa_cmd_set_viewport(daxa_CommandList self, VkViewport const * info)
{
}

void daxa_cmd_set_scissor(daxa_CommandList self, VkRect2D const * info)
{
}

void daxa_cmd_set_depth_bias(daxa_CommandList self, daxa_DepthBiasInfo const * info)
{
}

void daxa_cmd_set_index_buffer(daxa_CommandList self, daxa_BufferId id, size_t offset, size_t index_type_byte_size)
{
}

void daxa_cmd_draw(daxa_CommandList self, daxa_DrawInfo const * info)
{
}

void daxa_cmd_draw_indexed(daxa_CommandList self, daxa_DrawIndexedInfo const * info)
{
}

void daxa_cmd_draw_indirect(daxa_CommandList self, daxa_DrawIndirectInfo const * info)
{
}

void daxa_cmd_draw_indirect_count(daxa_CommandList self, daxa_DrawIndirectCountInfo const * info)
{
}

void daxa_cmd_draw_mesh_tasks(daxa_CommandList self, uint32_t x, uint32_t y, uint32_t z)
{
}

void daxa_cmd_draw_mesh_tasks_indirect(daxa_CommandList self, daxa_DrawMeshTasksIndirectInfo const * info)
{
}

void daxa_cmd_draw_mesh_tasks_indirect_count(daxa_CommandList self, daxa_DrawMeshTasksIndirectCountInfo const * info)
{
}

void daxa_cmd_write_timestamp(daxa_CommandList self, daxa_WriteTimestampInfo const * info)
{
}

void daxa_cmd_reset_timestamps(daxa_CommandList self, daxa_ResetTimestampsInfo const * info)
{
}

void daxa_cmd_begin_label(daxa_CommandList self, daxa_CommandLabelInfo const * info)
{
}

void daxa_cmd_end_label(daxa_CommandList self, daxa_CommandLabelInfo label)
{
}

// Is called by all other commands. Flushes internal pipeline barrier list to actual vulkan call.
void daxa_cmd_flush_barriers(daxa_CommandList self)
{
}

/// @brief  Consumes the command list. Creates backed commands that can be submitted to the device.
///         After calling complete, the command list CAN NOT BE USED any longer,
///         that includes destroying it, it is fully consumed by complete!
daxa_BakedCommands
daxa_cmd_complete(daxa_CommandList self)
{
    self->recording_complete = true;
    return reinterpret_cast<daxa_BakedCommands>(self);
}

daxa_CommandListInfo const *
daxa_cmd_info(daxa_CommandList self)
{
    return &self->info;
}

VkCommandBuffer
daxa_cmd_get_vk_command_buffer(daxa_CommandList self)
{
    return self->vk_cmd_buffer;
}

VkCommandPool
daxa_cmd_get_vk_command_pool(daxa_CommandList self)
{
    return self->vk_cmd_pool;
}

void daxa_destroy_command_list(daxa_CommandList self)
{
    vkResetCommandPool(self->device->vk_device, self->vk_cmd_pool, {});

    u64 const main_queue_cpu_timeline = self->device->main_queue_cpu_timeline.load(std::memory_order::relaxed);
    std::unique_lock const lock{self->device->main_queue_zombies_mtx};

    self->device->main_queue_command_list_zombies.push_front({
        main_queue_cpu_timeline,
        CommandListZombie{
            .vk_cmd_buffer = self->vk_cmd_buffer,
            .vk_cmd_pool = self->vk_cmd_pool,
        },
    });
}

void daxa_destroy_baked_commands(daxa_BakedCommands baked_commands)
{
}

/// --- End API Functions ---

/// --- Begin Internals ---

auto daxa_ImplCommandList::create(daxa_Device device, daxa_CommandListInfo const * info, VkCommandBuffer vk_cmd_buffer, VkCommandPool vk_cmd_pool) -> std::pair<daxa_CommandList, daxa_Result>
{
    auto * self = new daxa_ImplCommandList;
    auto & ret = *self;
    ret.device = device,
    ret.info = *info,
    ret.vk_cmd_buffer = vk_cmd_buffer,
    ret.vk_cmd_pool = vk_cmd_pool,
    ret.info_name = {info->name.data, info->name.size};
    ret.info.name = {ret.info_name.data(), ret.info_name.size()};
    VkCommandBufferBeginInfo const vk_command_buffer_begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = {},
    };
    vkBeginCommandBuffer(ret.vk_cmd_buffer, &vk_command_buffer_begin_info);
    if ((ret.device->instance->info.flags & DAXA_INSTANCE_FLAG_DEBUG_UTIL) != 0 && ret.info.name.size != 0)
    {
        auto cmd_buffer_name = ret.info.name;
        VkDebugUtilsObjectNameInfoEXT const cmd_buffer_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_COMMAND_BUFFER,
            .objectHandle = reinterpret_cast<uint64_t>(ret.vk_cmd_buffer),
            .pObjectName = cmd_buffer_name.data,
        };
        ret.device->vkSetDebugUtilsObjectNameEXT(ret.device->vk_device, &cmd_buffer_name_info);

        auto cmd_pool_name = ret.info.name;
        VkDebugUtilsObjectNameInfoEXT const cmd_pool_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_COMMAND_POOL,
            .objectHandle = reinterpret_cast<uint64_t>(ret.vk_cmd_pool),
            .pObjectName = cmd_pool_name.data,
        };
        ret.device->vkSetDebugUtilsObjectNameEXT(ret.device->vk_device, &cmd_pool_name_info);
    }
    return std::pair<daxa_CommandList, daxa_Result>{self, DAXA_RESULT_SUCCESS};
}

void daxa_ImplCommandList::flush_barriers()
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

void daxa_ImplCommandList::flush_uniform_buffer_bindings(VkPipelineBindPoint bind_point, VkPipelineLayout pipeline_layout)
{
    auto & device = *this->device;
    std::array<VkDescriptorBufferInfo, DAXA_UNIFORM_BUFFER_BINDINGS_COUNT> descriptor_buffer_info = {};
    std::array<VkWriteDescriptorSet, DAXA_UNIFORM_BUFFER_BINDINGS_COUNT> descriptor_writes = {};
    for (u32 index = 0; index < DAXA_UNIFORM_BUFFER_BINDINGS_COUNT; ++index)
    {
        if (this->current_constant_buffer_bindings[index].buffer.value == 0)
        {
            descriptor_buffer_info[index] = VkDescriptorBufferInfo{
                .buffer = device.vk_null_buffer,
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
    for (u32 index = 0; index < DAXA_UNIFORM_BUFFER_BINDINGS_COUNT; ++index)
    {
        this->current_constant_buffer_bindings.at(index) = {};
    }
}

/*
daxa_ImplCommandList::daxa_ImplCommandList(daxa_Device a_device, VkCommandPool pool, VkCommandBuffer buffer, CommandListInfo a_info)
    : device{a_device},
        info{std::move(a_info)},
        vk_cmd_buffer{buffer},
        vk_cmd_pool{pool},
        pipeline_layouts{&(this->device->gpu_shader_resource_table.pipeline_layouts)}
{
    initialize();
}

ImplCommandList::~ImplCommandList() // NOLINT(bugprone-exception-escape)
{
    vkResetCommandPool(device->vk_device, this->vk_cmd_pool, {});

    u64 const main_queue_cpu_timeline = device->main_queue_cpu_timeline.load(std::memory_order::relaxed);
    std::unique_lock const lock{device->main_queue_zombies_mtx};

    device->main_queue_command_list_zombies.push_front({
        main_queue_cpu_timeline,
        CommandListZombie{
            .vk_cmd_buffer = vk_cmd_buffer,
            .vk_cmd_pool = vk_cmd_pool,
        },
    });
}*/

/// --- End Internals ---

/*
namespace daxa
{

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
            impl.device->slot(info.src_buffer).vk_buffer,
            impl.device->slot(info.dst_buffer).vk_buffer,
            1,
            &vk_buffer_copy);
    }

    void CommandList::copy_buffer_to_image(BufferImageCopyInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();
        auto const & img_slot = impl.device->slot(info.image);
        VkBufferImageCopy const vk_buffer_image_copy{
            .bufferOffset = info.buffer_offset,
            .bufferRowLength = 0u,   // info.image_extent.x,
            .bufferImageHeight = 0u, // info.image_extent.y,
            .imageSubresource = make_subresource_layers(info.image_slice, img_slot.aspect_flags),
            .imageOffset = *reinterpret_cast<VkOffset3D const *>(&info.image_offset),
            .imageExtent = *reinterpret_cast<VkExtent3D const *>(&info.image_extent),
        };
        vkCmdCopyBufferToImage(
            impl.vk_cmd_buffer,
            impl.device->slot(info.buffer).vk_buffer,
            img_slot.vk_image,
            static_cast<VkImageLayout>(info.image_layout),
            1,
            &vk_buffer_image_copy);
    }

    void CommandList::copy_image_to_buffer(ImageBufferCopyInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();
        auto const & img_slot = impl.device->slot(info.image);
        VkBufferImageCopy const vk_buffer_image_copy{
            .bufferOffset = info.buffer_offset,
            .bufferRowLength = 0u,   // info.image_extent.x,
            .bufferImageHeight = 0u, // info.image_extent.y,
            .imageSubresource = make_subresource_layers(info.image_slice, img_slot.aspect_flags),
            .imageOffset = *reinterpret_cast<VkOffset3D const *>(&info.image_offset),
            .imageExtent = *reinterpret_cast<VkExtent3D const *>(&info.image_extent),
        };
        vkCmdCopyImageToBuffer(
            impl.vk_cmd_buffer,
            img_slot.vk_image,
            static_cast<VkImageLayout>(info.image_layout),
            impl.device->slot(info.buffer).vk_buffer,
            1,
            &vk_buffer_image_copy);
    }

    void CommandList::blit_image_to_image(ImageBlitInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");
        impl.flush_barriers();
        auto const & src_slot = impl.device->slot(info.src_image);
        auto const & dst_slot = impl.device->slot(info.dst_image);
        VkImageBlit const vk_blit{
            .srcSubresource = make_subresource_layers(info.src_slice, src_slot.aspect_flags),
            .srcOffsets = {*reinterpret_cast<VkOffset3D const *>(info.src_offsets.data()), *reinterpret_cast<VkOffset3D const *>(&info.src_offsets[1])},
            .dstSubresource = make_subresource_layers(info.dst_slice, dst_slot.aspect_flags),
            .dstOffsets = {*reinterpret_cast<VkOffset3D const *>(info.dst_offsets.data()), *reinterpret_cast<VkOffset3D const *>(&info.dst_offsets[1])},
        };
        vkCmdBlitImage(
            impl.vk_cmd_buffer,
            src_slot.vk_image,
            static_cast<VkImageLayout>(info.src_image_layout),
            dst_slot.vk_image,
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
        auto const & src_slot = impl.device->slot(info.src_image);
        auto const & dst_slot = impl.device->slot(info.dst_image);
        VkImageCopy const vk_image_copy{
            .srcSubresource = make_subresource_layers(info.src_slice, src_slot.aspect_flags),
            .srcOffset = {*reinterpret_cast<VkOffset3D const *>(&info.src_offset)},
            .dstSubresource = make_subresource_layers(info.dst_slice, dst_slot.aspect_flags),
            .dstOffset = {*reinterpret_cast<VkOffset3D const *>(&info.dst_offset)},
            .extent = {*reinterpret_cast<VkExtent3D const *>(&info.extent)},
        };

        vkCmdCopyImage(
            impl.vk_cmd_buffer,
            src_slot.vk_image,
            static_cast<VkImageLayout>(info.src_image_layout),
            dst_slot.vk_image,
            static_cast<VkImageLayout>(info.dst_image_layout),
            1,
            &vk_image_copy);
    }

    void CommandList::clear_image(ImageClearInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        auto & img_slot = impl.device->slot(info.dst_image);
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");
        impl.flush_barriers();
        if (is_depth_format(img_slot.info.format) || is_stencil_format(img_slot.info.format))
        {
            DAXA_DBG_ASSERT_TRUE_M(
                std::holds_alternative<DepthValue>(info.clear_value),
                "provided a color clear value for an image with a depth / stencil format!");

            auto const & clear_value = std::get<DepthValue>(info.clear_value);
            VkClearDepthStencilValue const color{
                .depth = clear_value.depth,
                .stencil = clear_value.stencil,
            };
            VkImageSubresourceRange sub_range = make_subresource_range(info.dst_slice, img_slot.aspect_flags);
            vkCmdClearDepthStencilImage(
                impl.vk_cmd_buffer,
                impl.device->slot(info.dst_image).vk_image,
                static_cast<VkImageLayout>(info.dst_image_layout),
                &color,
                1,
                &sub_range);
        }
        else
        {
            DAXA_DBG_ASSERT_TRUE_M(
                !std::holds_alternative<DepthValue>(info.clear_value),
                "provided a depth clear value for an image with a color format!");
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

            VkImageSubresourceRange sub_range = make_subresource_range(info.dst_slice, img_slot.aspect_flags);
            vkCmdClearColorImage(
                impl.vk_cmd_buffer,
                img_slot.vk_image,
                static_cast<VkImageLayout>(info.dst_image_layout),
                &color,
                1,
                &sub_range);
        }
    }

    void CommandList::clear_buffer(BufferClearInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");
        impl.flush_barriers();

        vkCmdFillBuffer(
            impl.vk_cmd_buffer,
            impl.device->slot(info.buffer).vk_buffer,
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

        impl.flush_uniform_buffer_bindings(VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_impl.vk_pipeline_layout);

        vkCmdBindDescriptorSets(impl.vk_cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_impl.vk_pipeline_layout, 0, 1, &impl.device->gpu_shader_resource_table.vk_descriptor_set, 0, nullptr);

        vkCmdBindPipeline(impl.vk_cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_impl.vk_pipeline);
    }

    void CommandList::set_uniform_buffer(SetConstantBufferInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        DAXA_DBG_ASSERT_TRUE_M(info.size > 0, "the set constant buffer range must be greater then 0");
        DAXA_DBG_ASSERT_TRUE_M(info.offset % impl.device->vk_physical_device_properties2.properties.limits.minUniformBufferOffsetAlignment == 0, "must respect the alignment requirements of uniform buffer bindings for constant buffer offsets!");
        const usize buffer_size = impl.device->slot(info.buffer).info.size;
        [[maybe_unused]] bool const binding_in_range = info.size + info.offset <= buffer_size;
        DAXA_DBG_ASSERT_TRUE_M(binding_in_range, "The given offset and size of the buffer binding is outside of the bounds of the given buffer");
        DAXA_DBG_ASSERT_TRUE_M(info.slot < DAXA_UNIFORM_BUFFER_BINDINGS_COUNT, "there are only 8 binding slots available for constant buffers");
        impl.current_constant_buffer_bindings[info.slot] = info;
    }

    void CommandList::set_pipeline(RasterPipeline const & pipeline)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(pipeline.object != nullptr, "invalid pipeline handle - valid handle must be retrieved from the pipeline compiler before use");
        auto const & pipeline_impl = *pipeline.as<ImplRasterPipeline>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only complete uncompleted command list");
        impl.flush_barriers();

        vkCmdBindDescriptorSets(impl.vk_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_impl.vk_pipeline_layout, 0, 1, &impl.device->gpu_shader_resource_table.vk_descriptor_set, 0, nullptr);

        impl.flush_uniform_buffer_bindings(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_impl.vk_pipeline_layout);

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

        vkCmdDispatchIndirect(impl.vk_cmd_buffer, impl.device->slot(info.indirect_buffer).vk_buffer, info.offset);
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
        auto & device = *impl.device;
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");
        impl.flush_barriers();
        for (auto const & end_info : infos)
        {
            tl_split_barrier_dependency_infos_aux_buffer.push_back({});
            auto & dependency_infos_aux_buffer = tl_split_barrier_dependency_infos_aux_buffer.back();
            for (auto & image_barrier : end_info.image_barriers)
            {
                dependency_infos_aux_buffer.vk_image_memory_barriers.push_back(
                    get_vk_image_memory_barrier(image_barrier, device.slot(image_barrier.image_id).vk_image, device.slot(image_barrier.image_id).aspect_flags));
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
        auto & device = *impl.device;
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can not record commands to completed command list");
        impl.flush_barriers();

        tl_split_barrier_dependency_infos_aux_buffer.push_back({});
        auto & dependency_infos_aux_buffer = tl_split_barrier_dependency_infos_aux_buffer.back();

        for (auto & image_barrier : info.image_barriers)
        {
            dependency_infos_aux_buffer.vk_image_memory_barriers.push_back(
                get_vk_image_memory_barrier(image_barrier, device.slot(image_barrier.image_id).vk_image, device.slot(image_barrier.image_id).aspect_flags));
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
        auto const & img_slot = impl.device->slot(info.image_id);
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
            .image = img_slot.vk_image,
            .subresourceRange = make_subresource_range(info.image_slice, img_slot.aspect_flags),
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
                .imageView = impl.device->slot(in.image_view).vk_image_view,
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
        vkCmdBindIndexBuffer(impl.vk_cmd_buffer, impl.device->slot(id).vk_buffer, offset, vk_index_type);
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
                impl.device->slot(info.draw_command_buffer).vk_buffer,
                info.draw_command_buffer_read_offset,
                info.draw_count,
                info.draw_command_stride);
        }
        else
        {
            vkCmdDrawIndirect(
                impl.vk_cmd_buffer,
                impl.device->slot(info.draw_command_buffer).vk_buffer,
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
                impl.device->slot(info.draw_command_buffer).vk_buffer,
                info.draw_command_buffer_read_offset,
                impl.device->slot(info.draw_count_buffer).vk_buffer,
                info.draw_count_buffer_read_offset,
                info.max_draw_count,
                info.draw_command_stride);
        }
        else
        {
            vkCmdDrawIndirectCount(
                impl.vk_cmd_buffer,
                impl.device->slot(info.draw_command_buffer).vk_buffer,
                info.draw_command_buffer_read_offset,
                impl.device->slot(info.draw_count_buffer).vk_buffer,
                info.draw_count_buffer_read_offset,
                info.max_draw_count,
                info.draw_command_stride);
        }
    }

    void CommandList::draw_mesh_tasks(u32 x, u32 y, u32 z)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only record to uncompleted command list");
        DAXA_DBG_ASSERT_TRUE_M((impl.device->info.flags & DAXA_DEVICE_FLAG_MESH_SHADER_BIT) != 0, "must enable mesh shading in device creation in order to use draw mesh tasks draw calls");
        impl.device->vkCmdDrawMeshTasksEXT(impl.vk_cmd_buffer, x, y, z);
    }

    void CommandList::draw_mesh_tasks_indirect(DrawMeshTasksIndirectInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only record to uncompleted command list");
        DAXA_DBG_ASSERT_TRUE_M((impl.device->info.flags & DAXA_DEVICE_FLAG_MESH_SHADER_BIT) != 0, "must enable mesh shading in device creation in order to use draw mesh tasks draw calls");
        impl.device->vkCmdDrawMeshTasksIndirectEXT(impl.vk_cmd_buffer, impl.device->slot(info.indirect_buffer).vk_buffer, info.offset, info.draw_count, info.stride);
    }

    void CommandList::draw_mesh_tasks_indirect_count(DrawMeshTasksIndirectCountInfo const & info)
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only record to uncompleted command list");
        DAXA_DBG_ASSERT_TRUE_M((impl.device->info.flags & DAXA_DEVICE_FLAG_MESH_SHADER_BIT) != 0, "must enable mesh shading in device creation in order to use draw mesh tasks draw calls");
        impl.device->vkCmdDrawMeshTasksIndirectCountEXT(
            impl.vk_cmd_buffer,
            impl.device->slot(info.indirect_buffer).vk_buffer,
            info.offset,
            impl.device->slot(info.count_buffer).vk_buffer,
            info.count_offset,
            info.max_count,
            info.stride);
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
            .pLabelName = info.name.c_str(),
            .color = {
                info.label_color[0],
                info.label_color[1],
                info.label_color[2],
                info.label_color[3]},
        };

        if ((impl.device->instance->info.flags & DAXA_INSTANCE_FLAG_DEBUG_UTIL) != 0)
        {
            impl.device->vkCmdBeginDebugUtilsLabelEXT(impl.vk_cmd_buffer, &vk_debug_label_info);
        }
    }

    void CommandList::end_label()
    {
        auto & impl = *as<ImplCommandList>();
        DAXA_DBG_ASSERT_TRUE_M(impl.recording_complete == false, "can only record to uncompleted command list");
        impl.flush_barriers();
        if ((impl.device->instance->info.flags & DAXA_INSTANCE_FLAG_DEBUG_UTIL) != 0)
        {
            impl.device->vkCmdEndDebugUtilsLabelEXT(impl.vk_cmd_buffer);
        }
    }
} // namespace daxa
*/