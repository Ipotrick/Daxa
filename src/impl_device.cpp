#include "impl_device.hpp"

#include <utility>
#include "impl_features.hpp"

namespace
{
    auto initialize_image_create_info_from_image_info(daxa_ImageInfo const & image_info, u32 const * queue_family_index_ptr) -> VkImageCreateInfo
    {
        DAXA_DBG_ASSERT_TRUE_M(std::popcount(image_info.sample_count) == 1 && image_info.sample_count <= 64, "image samples must be power of two and between 1 and 64(inclusive)");
        DAXA_DBG_ASSERT_TRUE_M(
            image_info.size.width > 0 &&
                image_info.size.height > 0 &&
                image_info.size.depth > 0,
            "image (x,y,z) dimensions must be greater then 0");
        DAXA_DBG_ASSERT_TRUE_M(image_info.array_layer_count > 0, "image array layer count must be greater then 0");
        DAXA_DBG_ASSERT_TRUE_M(image_info.mip_level_count > 0, "image mip level count must be greater then 0");

        auto const vk_image_type = static_cast<VkImageType>(image_info.dimensions - 1);

        VkImageCreateFlags vk_image_create_flags = static_cast<VkImageCreateFlags>(image_info.flags);

        VkImageCreateInfo const vk_image_create_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = vk_image_create_flags,
            .imageType = vk_image_type,
            .format = static_cast<VkFormat>(image_info.format),
            .extent = std::bit_cast<VkExtent3D>(image_info.size),
            .mipLevels = image_info.mip_level_count,
            .arrayLayers = image_info.array_layer_count,
            .samples = static_cast<VkSampleCountFlagBits>(image_info.sample_count),
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = image_info.usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = queue_family_index_ptr,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        return vk_image_create_info;
    }
    using namespace daxa::types;
} // namespace

#if 0

    auto daxa_dvc_get_vk_buffer(daxa_Device self, daxa_BufferId buffer, VkBuffer * out_vk_buffer) -> daxa_Result
    auto daxa_dvc_get_vk_image(daxa_Device self, daxa_ImageId image, VkImage * out_vk_image) -> daxa_Result
    auto daxa_dvc_get_default_vk_image_view(daxa_Device self, daxa_ImageId image, VkImageView * out_vk_image_view) -> daxa_Result
    auto daxa_dvc_get_vk_image_view(daxa_Device self, daxa_ImageViewId image_view, VkImageView * out_vk_image_view) -> daxa_Result
    auto daxa_dvc_get_vk_sampler(daxa_Device self, daxa_SamplerId sampler, VkSampler * out_vk_sampler) -> daxa_Result
    auto daxa_dvc_get_vk_device(daxa_Device self) -> VkDevice

    // auto Device::properties() const -> DeviceProperties const &
    // {
    //     auto const & impl = *as<ImplDevice>();
    //     return self->vk_info;
    // }
    // auto Device::mesh_shader_properties() const -> MeshShaderDeviceProperties const &
    // {
    //     auto const & impl = *as<ImplDevice>();
    //     return self->mesh_shader_properties;
    // }

#endif

#include "impl_device.hpp"

#include <utility>

// --- Begin API Functions ---

auto daxa_dvc_create_memory(daxa_Device self, daxa_MemoryBlockInfo const * info, daxa_MemoryBlock * out_memory_block) -> daxa_Result
{
    if (info->requirements.memoryTypeBits == 0)
    {
        DAXA_DBG_ASSERT_TRUE_M(false, "memory_type_bits must be non zero");
        return DAXA_RESULT_UNKNOWN;
    }

    VmaAllocationCreateInfo create_info{
        .flags = info->flags,
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = {}, // idk what this is...
        .preferredFlags = {},
        .memoryTypeBits = {}, // idk what this is....
        .pool = {},
        .pUserData = {},
        .priority = 0.5f,
    };
    VmaAllocation allocation = {};
    VmaAllocationInfo allocation_info = {};
    vmaAllocateMemory(self->vma_allocator, &info->requirements, &create_info, &allocation, &allocation_info);

    // TODO(capi): This needs to return a result, but thought needs to be given to how to handle the managed pointer
    // return MemoryBlock{ManagedPtr{
    //     new ImplMemoryBlock(self, *reinterpret_cast<MemoryBlockInfo const *>(info), allocation, allocation_info),
    //     [](daxa_ImplHandle * self_ptr)
    //     {
    //         delete reinterpret_cast<ImplMemoryBlock *>(self_ptr);
    //     }}};
    return DAXA_RESULT_UNKNOWN;
}
auto daxa_dvc_buffer_memory_requirements(daxa_Device self, daxa_BufferInfo const * info) -> VkMemoryRequirements
{
    VkBufferCreateInfo const vk_buffer_create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = {},
        .size = static_cast<VkDeviceSize>(info->size),
        .usage = BUFFER_USE_FLAGS,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &self->main_queue_family_index,
    };
    VkDeviceBufferMemoryRequirements buffer_requirement_info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS,
        .pNext = {},
        .pCreateInfo = &vk_buffer_create_info,
    };
    VkMemoryRequirements2 mem_requirements = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
        .pNext = {},
        .memoryRequirements = {},
    };
    vkGetDeviceBufferMemoryRequirements(self->vk_device, &buffer_requirement_info, &mem_requirements);
    // MemoryRequirements ret = std::bit_cast<MemoryRequirements>(mem_requirements.memoryRequirements);
    return mem_requirements.memoryRequirements;
}
auto daxa_dvc_image_memory_requirements(daxa_Device self, daxa_ImageInfo const * info) -> VkMemoryRequirements
{
    VkImageCreateInfo vk_image_create_info = initialize_image_create_info_from_image_info(*info, &self->main_queue_family_index);
    VkDeviceImageMemoryRequirements image_requirement_info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS,
        .pNext = {},
        .pCreateInfo = &vk_image_create_info,
        .planeAspect = static_cast<VkImageAspectFlagBits>(infer_aspect_from_format(info->format)),
    };
    VkMemoryRequirements2 mem_requirements{
        .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
        .pNext = {},
        .memoryRequirements = {},
    };
    vkGetDeviceImageMemoryRequirements(self->vk_device, &image_requirement_info, &mem_requirements);
    return mem_requirements.memoryRequirements;
}

auto daxa_dvc_info(daxa_Device self) -> daxa_DeviceInfo const *
{
    return &self->info;
}

auto daxa_dvc_wait_idle(daxa_Device self) -> daxa_Result
{
    self->wait_idle();
    // TODO(capi) get result from c++ api
    return DAXA_RESULT_SUCCESS;
}
auto daxa_dvc_submit(daxa_Device self, daxa_CommandSubmitInfo const * info) -> daxa_Result
{
    self->main_queue_collect_garbage();

    u64 const current_main_queue_cpu_timeline_value = DAXA_ATOMIC_FETCH_INC(self->main_queue_cpu_timeline) + 1;

    std::pair<u64, std::vector<ManagedPtr>> submit = {current_main_queue_cpu_timeline_value, {}};

    // TODO(capi): bruh
    // for (auto const & command_list : submit_info.command_lists)
    // {
    //     auto const * cmd_list = command_list.as<ImplCommandList>();
    //     for (auto [id, index] : cmd_list->deferred_destructions)
    //     {
    //         switch (index)
    //         {
    //         case DEFERRED_DESTRUCTION_BUFFER_INDEX: self->main_queue_buffer_zombies.push_front({current_main_queue_cpu_timeline_value, BufferId{id}}); break;
    //         case DEFERRED_DESTRUCTION_IMAGE_INDEX: self->main_queue_image_zombies.push_front({current_main_queue_cpu_timeline_value, ImageId{id}}); break;
    //         case DEFERRED_DESTRUCTION_IMAGE_VIEW_INDEX: self->main_queue_image_view_zombies.push_front({current_main_queue_cpu_timeline_value, ImageViewId{id}}); break;
    //         case DEFERRED_DESTRUCTION_SAMPLER_INDEX: self->main_queue_sampler_zombies.push_front({current_main_queue_cpu_timeline_value, SamplerId{id}}); break;
    //         default: DAXA_DBG_ASSERT_TRUE_M(false, "unreachable");
    //         }
    //     }
    // }

    // std::vector<VkCommandBuffer> submit_vk_command_buffers = {};
    // for (auto const & command_list : submit_info.command_lists)
    // {
    //     auto const & impl_cmd_list = *command_list.as<ImplCommandList>();
    //     DAXA_DBG_ASSERT_TRUE_M(impl_cmd_list.recording_complete, "all submitted command lists must be completed before submission");
    //     submit.second.push_back(command_list);
    //     submit_vk_command_buffers.push_back(impl_cmd_list.vk_cmd_buffer);
    // }

    // std::vector<VkSemaphore> submit_semaphore_signals = {}; // All timeline semaphores come first, then binary semaphores follow.
    // std::vector<u64> submit_semaphore_signal_values = {};   // Used for timeline semaphores. Ignored (push dummy value) for binary semaphores.

    // // Add main queue timeline signaling as first timeline semaphore signaling:
    // submit_semaphore_signals.push_back(self->vk_main_queue_gpu_timeline_semaphore);
    // submit_semaphore_signal_values.push_back(current_main_queue_cpu_timeline_value);

    // for (auto const & [timeline_semaphore, signal_value] : submit_info.signal_timeline_semaphores)
    // {
    //     auto const & impl_timeline_semaphore = *timeline_semaphore.as<ImplTimelineSemaphore>();
    //     submit_semaphore_signals.push_back(impl_timeline_semaphore.vk_semaphore);
    //     submit_semaphore_signal_values.push_back(signal_value);
    // }

    // for (auto const & binary_semaphore : submit_info.signal_binary_semaphores)
    // {
    //     auto const & impl_binary_semaphore = *binary_semaphore.as<ImplBinarySemaphore>();
    //     submit_semaphore_signals.push_back(impl_binary_semaphore.vk_semaphore);
    //     submit_semaphore_signal_values.push_back(0); // The vulkan spec requires to have dummy values for binary semaphores.
    // }

    // // used to synchronize with previous submits:
    // std::vector<VkSemaphore> submit_semaphore_waits = {}; // All timeline semaphores come first, then binary semaphores follow.
    // std::vector<VkPipelineStageFlags> submit_semaphore_wait_stage_masks = {};
    // std::vector<u64> submit_semaphore_wait_values = {}; // Used for timeline semaphores. Ignored (push dummy value) for binary semaphores.

    // for (auto const & [timeline_semaphore, wait_value] : submit_info.wait_timeline_semaphores)
    // {
    //     auto const & impl_timeline_semaphore = *timeline_semaphore.as<ImplTimelineSemaphore>();
    //     submit_semaphore_waits.push_back(impl_timeline_semaphore.vk_semaphore);
    //     submit_semaphore_wait_stage_masks.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    //     submit_semaphore_wait_values.push_back(wait_value);
    // }

    // for (auto const & binary_semaphore : submit_info.wait_binary_semaphores)
    // {
    //     auto const & impl_binary_semaphore = *binary_semaphore.as<ImplBinarySemaphore>();
    //     submit_semaphore_waits.push_back(impl_binary_semaphore.vk_semaphore);
    //     submit_semaphore_wait_stage_masks.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    //     submit_semaphore_wait_values.push_back(0);
    // }

    // VkTimelineSemaphoreSubmitInfo timeline_info{
    //     .sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
    //     .pNext = nullptr,
    //     .waitSemaphoreValueCount = static_cast<u32>(submit_semaphore_wait_values.size()),
    //     .pWaitSemaphoreValues = submit_semaphore_wait_values.data(),
    //     .signalSemaphoreValueCount = static_cast<u32>(submit_semaphore_signal_values.size()),
    //     .pSignalSemaphoreValues = submit_semaphore_signal_values.data(),
    // };

    // VkSubmitInfo const vk_submit_info{
    //     .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    //     .pNext = reinterpret_cast<void *>(&timeline_info),
    //     .waitSemaphoreCount = static_cast<u32>(submit_semaphore_waits.size()),
    //     .pWaitSemaphores = submit_semaphore_waits.data(),
    //     .pWaitDstStageMask = submit_semaphore_wait_stage_masks.data(),
    //     .commandBufferCount = static_cast<u32>(submit_vk_command_buffers.size()),
    //     .pCommandBuffers = submit_vk_command_buffers.data(),
    //     .signalSemaphoreCount = static_cast<u32>(submit_semaphore_signals.size()),
    //     .pSignalSemaphores = submit_semaphore_signals.data(),
    // };
    // vkQueueSubmit(self->main_queue_vk_queue, 1, &vk_submit_info, VK_NULL_HANDLE);

    // DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const lock{self->main_queue_zombies_mtx});
    // self->main_queue_submits_zombies.push_front(std::move(submit));

    return DAXA_RESULT_SUCCESS;
}
auto daxa_dvc_present(daxa_Device self, daxa_PresentInfo const * info) -> daxa_Result
{
    // auto & impl = *as<ImplDevice>();

    // TODO(capi): impl
    // auto const & swapchain_impl = *info.swapchain.as<ImplSwapchain>();

    // // used to synchronize with previous submits:
    // std::vector<VkSemaphore> submit_semaphore_waits = {};

    // for (auto const & binary_semaphore : info.wait_binary_semaphores)
    // {
    //     auto const & impl_binary_semaphore = *binary_semaphore.as<ImplBinarySemaphore>();
    //     submit_semaphore_waits.push_back(impl_binary_semaphore.vk_semaphore);
    // }

    // VkPresentInfoKHR const present_info{
    //     .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    //     .pNext = nullptr,
    //     .waitSemaphoreCount = static_cast<u32>(submit_semaphore_waits.size()),
    //     .pWaitSemaphores = submit_semaphore_waits.data(),
    //     .swapchainCount = static_cast<u32>(1),
    //     .pSwapchains = &swapchain_impl.vk_swapchain,
    //     .pImageIndices = &swapchain_impl.current_image_index,
    //     .pResults = {},
    // };

    // [[maybe_unused]] VkResult const err = vkQueuePresentKHR(self->main_queue_vk_queue, &present_info);
    // // We currently ignore VK_ERROR_OUT_OF_DATE_KHR, VK_ERROR_SURFACE_LOST_KHR and VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT
    // // because supposedly these kinds of things are not specified within the spec. This is also handled in Swapchain::acquire_next_image()
    // DAXA_DBG_ASSERT_TRUE_M(err == VK_SUCCESS || err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_ERROR_SURFACE_LOST_KHR || err == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT, "Daxa should never be in a situation where Present fails");

    self->main_queue_collect_garbage();
    return DAXA_RESULT_UNKNOWN;
}

daxa_Result
daxa_dvc_collect_garbage(daxa_Device self)
{
    // TODO(capi): Return proper error here!
    self->main_queue_collect_garbage();
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_create_swapchain(daxa_Device self, daxa_SwapchainInfo const * info, daxa_Swapchain * out_swapchain) -> daxa_Result
{
// TODO(capi): switch to create function when it's ready
#if 0
    return Swapchain{
        ManagedPtr{
            new ImplSwapchain(self, *info),
            [](daxa_ImplHandle * self_ptr)
            {
                delete reinterpret_cast<ImplSwapchain *>(self_ptr);
            }}};
#endif
    return DAXA_RESULT_UNKNOWN;
}
auto daxa_dvc_create_raster_pipeline(daxa_Device self, daxa_RasterPipelineInfo const * info, daxa_RasterPipeline * out_pipeline) -> daxa_Result
{
// TODO(capi): switch to create function when it's ready
#if 0
    return RasterPipeline{
        ManagedPtr{
            new daxa_ImplRasterPipeline(self, *info),
            [](daxa_ImplHandle * self_ptr)
            {
                delete reinterpret_cast<daxa_ImplRasterPipeline *>(self_ptr);
            }}};
#endif
    return DAXA_RESULT_UNKNOWN;
}
auto daxa_dvc_create_compute_pipeline(daxa_Device self, daxa_ComputePipelineInfo const * info, daxa_ComputePipeline * out_pipeline) -> daxa_Result
{
// TODO(capi): switch to create function when it's ready
#if 0
    return ComputePipeline{
        ManagedPtr{
            new daxa_ImplComputePipeline(self, *info),
            [](daxa_ImplHandle * self_ptr)
            {
                delete reinterpret_cast<daxa_ImplComputePipeline *>(self_ptr);
            }}};
#endif
    return DAXA_RESULT_UNKNOWN;
}
auto daxa_dvc_create_command_list(daxa_Device self, daxa_CommandListInfo const * info, daxa_CommandList * out_command_list) -> daxa_Result
{
    DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const lock{self->main_queue_command_pool_buffer_recycle_mtx});
    auto [pool, buffer] = self->buffer_pool_pool.get(self);
// TODO(capi): switch to create function when it's ready
#if 0
    return CommandList{
        ManagedPtr{
            new daxa_ImplCommandList{self, pool, buffer, info},
            [](daxa_ImplHandle * self_ptr)
            {
                delete reinterpret_cast<daxa_ImplCommandList *>(self_ptr);
            }}};
#endif
    return DAXA_RESULT_UNKNOWN;
}
auto daxa_dvc_create_binary_semaphore(daxa_Device self, daxa_BinarySemaphoreInfo const * info, daxa_BinarySemaphore * out_binary_semaphore) -> daxa_Result
{
// TODO(capi): switch to create function when it's ready
#if 0
    return BinarySemaphore(ManagedPtr(
        new daxa_ImplBinarySemaphore{self, info},
        [](daxa_ImplHandle * self_ptr)
        {
            delete reinterpret_cast<daxa_ImplBinarySemaphore *>(self_ptr);
        }));
#endif
    return DAXA_RESULT_UNKNOWN;
}
auto daxa_dvc_create_timeline_semaphore(daxa_Device self, daxa_TimelineSemaphoreInfo const * info, daxa_TimelineSemaphore * out_timeline_semaphore) -> daxa_Result
{
// TODO(capi): switch to create function when it's ready
#if 0
    return TimelineSemaphore(ManagedPtr(
        new daxa_ImplTimelineSemaphore(self, info),
        [](daxa_ImplHandle * self_ptr)
        {
            delete reinterpret_cast<daxa_ImplTimelineSemaphore *>(self_ptr);
        }));
#endif
    return DAXA_RESULT_UNKNOWN;
}
auto daxa_dvc_create_event(daxa_Device self, daxa_EventInfo const * info, daxa_Event * out_event) -> daxa_Result
{
    // return {this->make_weak(), info};
    return DAXA_RESULT_UNKNOWN;
}
auto daxa_dvc_create_timeline_query_pool(daxa_Device self, daxa_TimelineQueryPoolInfo const * info, daxa_TimelineQueryPool * out_timeline_query_pool) -> daxa_Result
{
// TODO(capi): switch to create function when it's ready
#if 0
    return TimelineQueryPool(ManagedPtr(
        new ImplTimelineQueryPool(self, info),
        [](daxa_ImplHandle * self_ptr)
        {
            delete reinterpret_cast<ImplTimelineQueryPool *>(self_ptr);
        }));
#endif
    return DAXA_RESULT_UNKNOWN;
}

auto daxa_dvc_buffer_device_address(daxa_Device self, daxa_BufferId id, daxa_BufferDeviceAddress * out_bda) -> daxa_Result
{
    // TODO(capi): This may error
    *out_bda = static_cast<daxa_BufferDeviceAddress>(self->slot(id).device_address);
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_buffer_host_address(daxa_Device self, daxa_BufferId id, void ** out_ptr) -> daxa_Result
{
    DAXA_DBG_ASSERT_TRUE_M(
        self->slot(id).host_address != nullptr,
        "host buffer address is only available if the buffer is created with either of the following memory flags: HOST_ACCESS_RANDOM, HOST_ACCESS_SEQUENTIAL_WRITE");
    // TODO(capi): This may error
    *out_ptr = self->slot(id).host_address;
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_create_buffer(daxa_Device self, daxa_BufferInfo const * info, daxa_BufferId * out_id) -> daxa_Result
{
    // --- Begin Parameter Validation ---

    bool parameters_valid = true;
    // Size must be larger then one.
    parameters_valid = parameters_valid && info->size > 0;
    if (!parameters_valid)
        return DAXA_RESULT_INVALID_BUFFER_INFO;

        // --- End Parameter Validation ---

        // TODO(capi): implement (NEEDS REF COUNT OF INFO->THING)
#if 0
    auto [id, ret] = self->gpu_shader_resource_table.buffer_slots.new_slot();
    ret.info = std::bit_cast<BufferInfo>(*info);
    ret.info_name = ret.info.name;
    ret.info.name = {ret.info_name.data(), ret.info_name.size()};

    VkBufferCreateInfo const vk_buffer_create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = {},
        .size = static_cast<VkDeviceSize>(buffer_info.size),
        .usage = BUFFER_USE_FLAGS,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &this->main_queue_family_index,
    };

    bool host_accessible = false;
    VmaAllocationInfo vma_allocation_info = {};
    if (AutoAllocInfo const * auto_info = std::get_if<AutoAllocInfo>(&buffer_info.manual_alloc_info))
    {
        auto vma_allocation_flags = static_cast<VmaAllocationCreateFlags>(auto_info->data);
        if (((vma_allocation_flags & VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT) != 0u) ||
            ((vma_allocation_flags & VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT) != 0u) ||
            ((vma_allocation_flags & VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT) != 0u))
        {
            vma_allocation_flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
            host_accessible = true;
        }

        VmaAllocationCreateInfo const vma_allocation_create_info{
            .flags = vma_allocation_flags,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            .requiredFlags = {},
            .preferredFlags = {},
            .memoryTypeBits = std::numeric_limits<u32>::max(),
            .pool = nullptr,
            .pUserData = nullptr,
            .priority = 0.5f,
        };

        [[maybe_unused]] VkResult const vk_create_buffer_result = vmaCreateBuffer(self->vma_allocator, &vk_buffer_create_info, &vma_allocation_create_info, &ret.vk_buffer, &ret.vma_allocation, &vma_allocation_info);
        DAXA_DBG_ASSERT_TRUE_M(vk_create_buffer_result == VK_SUCCESS, "failed to create buffer");
    }
    else
    {
        ManualAllocInfo const & manual_info = std::get<ManualAllocInfo>(buffer_info.allocate_info);
        ImplMemoryBlock const & mem_block = *manual_info.memory_block.as<ImplMemoryBlock const>();

        // TODO(pahrens): Add validation for memory type requirements.

        vkCreateBuffer(self->vk_device, &self->vk_buffer_create_info, nullptr, &ret.vk_buffer);

        vmaBindBufferMemory2(
            self->vma_allocator,
            mem_block.allocation,
            manual_info.offset,
            ret.vk_buffer,
            {});
    }

    VkBufferDeviceAddressInfo const vk_buffer_device_address_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .pNext = nullptr,
        .buffer = ret.vk_buffer,
    };

    ret.device_address = vkGetBufferDeviceAddress(self->vk_device, &vk_buffer_device_address_info);

    ret.host_address = host_accessible ? vma_allocation_info.pMappedData : nullptr;
    ret.zombie = false;

    this->buffer_device_address_buffer_host_ptr[id.index] = ret.device_address;

    if ((this->instance->info.flags & DAXA_INSTANCE_FLAG_DEBUG_UTIL) != 0 && !buffer_info.name.empty())
    {
        auto const buffer_name = buffer_info.name;
        VkDebugUtilsObjectNameInfoEXT const buffer_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_BUFFER,
            .objectHandle = reinterpret_cast<uint64_t>(ret.vk_buffer),
            .pObjectName = buffer_name.c_str(),
        };
        this->vkSetDebugUtilsObjectNameEXT(vk_device, &buffer_name_info);
    }

    write_descriptor_set_buffer(this->vk_device, this->gpu_shader_resource_table.vk_descriptor_set, ret.vk_buffer, 0, static_cast<VkDeviceSize>(buffer_info.size), id.index);

    return BufferId{id};
#endif
}
auto daxa_dvc_create_image(daxa_Device self, daxa_ImageInfo const * info, daxa_ImageId * out_id) -> daxa_Result
{
    // TODO(capi): implement (NEEDS REF COUNT OF INFO->THING)
    return DAXA_RESULT_UNKNOWN;
}
auto daxa_dvc_create_image_view(daxa_Device self, daxa_ImageViewInfo const * info, daxa_ImageViewId * out_id) -> daxa_Result
{
    // TODO(capi): implement
    return DAXA_RESULT_UNKNOWN;
}
auto daxa_dvc_create_sampler(daxa_Device self, daxa_SamplerInfo const * info, daxa_SamplerId * out_id) -> daxa_Result
{
    // TODO(capi): implement
    return DAXA_RESULT_UNKNOWN;
}

#define _DAXA_DECL_DVC_GPU_RES_FN(NAME, Name, name, slots_name)                                                       \
    auto daxa_dvc_info_##name(daxa_Device self, daxa_##Name##Id id, daxa_##Name##Info const ** out_info)->daxa_Result \
    {                                                                                                                 \
        if (!daxa_dvc_is_##name##_valid(self, id))                                                                    \
            return DAXA_RESULT_INVALID_##NAME##_ID;                                                                   \
        *out_info = reinterpret_cast<daxa_##Name##Info const *>(&self->slot(id).info);                                \
        return DAXA_RESULT_SUCCESS;                                                                                   \
    }                                                                                                                 \
    auto daxa_dvc_is_##name##_valid(daxa_Device self, daxa_##Name##Id id)->daxa_Bool8                                 \
    {                                                                                                                 \
        auto gid = std::bit_cast<daxa::GPUResourceId>(id);                                                            \
        return id.value != 0 && self->gpu_shader_resource_table.slots_name##_slots.is_id_valid(gid);                  \
    }                                                                                                                 \
    auto daxa_dvc_destroy_##name(daxa_Device self, daxa_##Name##Id id)->daxa_Result                                   \
    {                                                                                                                 \
        DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const lock{self->main_queue_zombies_mtx});                         \
        u64 const main_queue_cpu_timeline_value = DAXA_ATOMIC_FETCH(self->main_queue_cpu_timeline);                   \
        if (!daxa_dvc_is_##name##_valid(self, id))                                                                    \
        {                                                                                                             \
            return DAXA_RESULT_INVALID_##NAME##_ID;                                                                   \
        }                                                                                                             \
        if (self->slot(id).zombie)                                                                                    \
        {                                                                                                             \
            return DAXA_RESULT_##NAME##_DOUBLE_FREE;                                                                  \
        }                                                                                                             \
        self->slot(id).zombie = true;                                                                                 \
        auto gid = std::bit_cast<daxa::Name##Id>(id);                                                                 \
        self->main_queue_##name##_zombies.push_front({main_queue_cpu_timeline_value, gid});                           \
        return DAXA_RESULT_SUCCESS;                                                                                   \
    }

_DAXA_DECL_DVC_GPU_RES_FN(BUFFER, Buffer, buffer, buffer)
_DAXA_DECL_DVC_GPU_RES_FN(IMAGE, Image, image, image)
_DAXA_DECL_DVC_GPU_RES_FN(IMAGE_VIEW, ImageView, image_view, image)
_DAXA_DECL_DVC_GPU_RES_FN(SAMPLER, Sampler, sampler, sampler)

auto daxa_destroy_device(daxa_Device self) -> daxa_Result
{
    daxa_dvc_wait_idle(self);
    self->main_queue_collect_garbage();
    self->buffer_pool_pool.cleanup(self);
    vmaUnmapMemory(self->vma_allocator, self->buffer_device_address_buffer_allocation);
    vmaDestroyBuffer(self->vma_allocator, self->buffer_device_address_buffer, self->buffer_device_address_buffer_allocation);
    self->gpu_shader_resource_table.cleanup(self->vk_device);
    vmaDestroyImage(self->vma_allocator, self->vk_null_image, self->vk_null_image_vma_allocation);
    vmaDestroyBuffer(self->vma_allocator, self->vk_null_buffer, self->vk_null_buffer_vma_allocation);
    vmaDestroyAllocator(self->vma_allocator);
    vkDestroySampler(self->vk_device, self->vk_null_sampler, nullptr);
    vkDestroyImageView(self->vk_device, self->vk_null_image_view, nullptr);
    vkDestroySemaphore(self->vk_device, self->vk_main_queue_gpu_timeline_semaphore, nullptr);
    vkDestroyDevice(self->vk_device, nullptr);
    delete self;
    return DAXA_RESULT_UNKNOWN;
}

// --- End API Functions ---

// --- Begin Internal Functions ---

auto daxa_ImplDevice::create(daxa_Instance instance, daxa_DeviceInfo const & info, VkPhysicalDevice physical_device) -> std::pair<daxa_Result, daxa_Device>
{
    auto * self = new daxa_ImplDevice;
    self->vk_physical_device = physical_device;
    self->instance = instance;
    self->info = info;
    self->info_name = std::string{info.name.data, info.name.size};
    self->info.name = {self->info_name.c_str(), self->info_name.size()};
    using namespace daxa;

    self->vk_physical_device_properties2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        .pNext = {},
        .properties = {},
    };
    vkGetPhysicalDeviceProperties2(self->vk_physical_device, &self->vk_physical_device_properties2);

    // SELECT QUEUE

    u32 queue_family_props_count = 0;
    std::vector<VkQueueFamilyProperties> queue_props;
    vkGetPhysicalDeviceQueueFamilyProperties(self->vk_physical_device, &queue_family_props_count, nullptr);
    queue_props.resize(queue_family_props_count);
    vkGetPhysicalDeviceQueueFamilyProperties(self->vk_physical_device, &queue_family_props_count, queue_props.data());
    std::vector<VkBool32> supports_present;
    supports_present.resize(queue_family_props_count);

    // Maybe check if device supports present for self surface?
    // self requires that the surface was already created.. which
    // is really CRINGE
    // for (u32 i = 0; i < queue_family_props_count; i++)
    //     vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &supports_present[i]);

    for (u32 i = 0; i < queue_family_props_count; i++)
    {
        if ((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 && (queue_props[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0 && (queue_props[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0)
        {
            self->main_queue_family_index = i;
            break;
        }
    }
    DAXA_DBG_ASSERT_TRUE_M(self->main_queue_family_index != std::numeric_limits<u32>::max(), "found no suitable queue family");

    std::array<f32, 1> queue_priorities = {0.0};
    VkDeviceQueueCreateInfo const queue_ci{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = self->main_queue_family_index,
        .queueCount = static_cast<u32>(queue_priorities.size()),
        .pQueuePriorities = queue_priorities.data(),
    };

    PhysicalDeviceFeatureTable feature_table = {};
    feature_table.initialize(info);
    PhysicalDeviceExtensionList extension_list = {};
    extension_list.initialize(info);

    VkPhysicalDeviceFeatures2 physical_device_features_2{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = feature_table.chain,
        .features = feature_table.features,
    };

    VkDeviceCreateInfo const device_ci = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = reinterpret_cast<void const *>(&physical_device_features_2),
        .flags = {},
        .queueCreateInfoCount = static_cast<u32>(1),
        .pQueueCreateInfos = &queue_ci,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<u32>(extension_list.size),
        // TODO(capi): This is a list of string views... this should be a list of c pointers!!
        // .ppEnabledExtensionNames = extension_list.data,
        .pEnabledFeatures = nullptr,
    };
    vkCreateDevice(self->vk_physical_device, &device_ci, nullptr, &self->vk_device);

    if ((self->instance->info.flags & DAXA_INSTANCE_FLAG_DEBUG_UTIL) != 0)
    {
        self->vkSetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetDeviceProcAddr(self->vk_device, "vkSetDebugUtilsObjectNameEXT"));
        self->vkCmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCmdBeginDebugUtilsLabelEXT"));
        self->vkCmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCmdEndDebugUtilsLabelEXT"));
    }
    self->vkCmdPushDescriptorSetKHR = reinterpret_cast<PFN_vkCmdPushDescriptorSetKHR>(vkGetDeviceProcAddr(self->vk_device, "vkCmdPushDescriptorSetKHR"));

    if ((self->info.flags & DAXA_DEVICE_FLAG_MESH_SHADER_BIT) != 0)
    {
        self->vkCmdDrawMeshTasksEXT = reinterpret_cast<PFN_vkCmdDrawMeshTasksEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCmdDrawMeshTasksEXT"));
        self->vkCmdDrawMeshTasksIndirectEXT = reinterpret_cast<PFN_vkCmdDrawMeshTasksIndirectEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCmdDrawMeshTasksIndirectEXT"));
        self->vkCmdDrawMeshTasksIndirectCountEXT = reinterpret_cast<PFN_vkCmdDrawMeshTasksIndirectCountEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCmdDrawMeshTasksIndirectCountEXT"));
        auto * out_struct = reinterpret_cast<VkBaseOutStructure *>(&self->vk_physical_device_properties2);
        while (out_struct != nullptr)
        {
            if (out_struct->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT)
            {
                self->mesh_shader_properties = *reinterpret_cast<VkPhysicalDeviceMeshShaderPropertiesEXT *>(out_struct);
            }
            out_struct = out_struct->pNext;
        }
    }

    vkGetDeviceQueue(self->vk_device, self->main_queue_family_index, 0, &self->main_queue_vk_queue);

    VkCommandPool init_cmd_pool = {};
    VkCommandBuffer init_cmd_buffer = {};
    VkCommandPoolCreateInfo const vk_command_pool_create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = self->main_queue_family_index,
    };

    vkCreateCommandPool(self->vk_device, &vk_command_pool_create_info, nullptr, &init_cmd_pool);

    VkCommandBufferAllocateInfo const vk_command_buffer_allocate_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = init_cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    vkAllocateCommandBuffers(self->vk_device, &vk_command_buffer_allocate_info, &init_cmd_buffer);

    VkCommandBufferBeginInfo const vk_command_buffer_begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = {},
    };
    vkBeginCommandBuffer(init_cmd_buffer, &vk_command_buffer_begin_info);

    VkSemaphoreTypeCreateInfo timeline_ci{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .pNext = nullptr,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = 0,
    };

    VkSemaphoreCreateInfo const vk_semaphore_create_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = reinterpret_cast<void *>(&timeline_ci),
        .flags = {},
    };

    vkCreateSemaphore(self->vk_device, &vk_semaphore_create_info, nullptr, &self->vk_main_queue_gpu_timeline_semaphore);

    DAXA_DBG_ASSERT_TRUE_M(
        self->info.max_allowed_buffers <= self->vk_physical_device_properties2.properties.limits.maxDescriptorSetStorageBuffers,
        std::string("device does not support ") +
            std::to_string(self->info.max_allowed_buffers) +
            " buffers, the device supports up to " +
            std::to_string(self->vk_physical_device_properties2.properties.limits.maxDescriptorSetStorageBuffers) +
            "buffers.");
    [[maybe_unused]] auto const max_device_supported_images_in_set = std::min(self->vk_physical_device_properties2.properties.limits.maxDescriptorSetSampledImages, self->vk_physical_device_properties2.properties.limits.maxDescriptorSetStorageImages);
    DAXA_DBG_ASSERT_TRUE_M(
        self->info.max_allowed_images <= max_device_supported_images_in_set,
        std::string("device does not support ") +
            std::to_string(self->info.max_allowed_images) +
            " images, the device supports up to " +
            std::to_string(max_device_supported_images_in_set) +
            "images.");
    DAXA_DBG_ASSERT_TRUE_M(
        self->info.max_allowed_samplers <= self->vk_physical_device_properties2.properties.limits.maxDescriptorSetSamplers,
        std::string("device does not support ") +
            std::to_string(self->info.max_allowed_samplers) +
            " samplers, the device supports up to " +
            std::to_string(self->vk_physical_device_properties2.properties.limits.maxDescriptorSetSamplers) +
            "samplers.");

    VmaVulkanFunctions const vma_vulkan_functions
    {
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
        .vkGetPhysicalDeviceProperties = {},
        .vkGetPhysicalDeviceMemoryProperties = {},
        .vkAllocateMemory = {},
        .vkFreeMemory = {},
        .vkMapMemory = {},
        .vkUnmapMemory = {},
        .vkFlushMappedMemoryRanges = {},
        .vkInvalidateMappedMemoryRanges = {},
        .vkBindBufferMemory = {},
        .vkBindImageMemory = {},
        .vkGetBufferMemoryRequirements = {},
        .vkGetImageMemoryRequirements = {},
        .vkCreateBuffer = {},
        .vkDestroyBuffer = {},
        .vkCreateImage = {},
        .vkDestroyImage = {},
        .vkCmdCopyBuffer = {},
#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
        .vkGetBufferMemoryRequirements2KHR = {},
        .vkGetImageMemoryRequirements2KHR = {},
#endif
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
        .vkBindBufferMemory2KHR = {},
        .vkBindImageMemory2KHR = {},
#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
        .vkGetPhysicalDeviceMemoryProperties2KHR = {},
#endif
#if VMA_VULKAN_VERSION >= 1003000
        .vkGetDeviceBufferMemoryRequirements = {},
        .vkGetDeviceImageMemoryRequirements = {},
#endif
    };

    VmaAllocatorCreateInfo const vma_allocator_create_info{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = self->vk_physical_device,
        .device = self->vk_device,
        .preferredLargeHeapBlockSize = 0, // Sets it to lib internal default (256MiB).
        .pAllocationCallbacks = nullptr,
        .pDeviceMemoryCallbacks = nullptr,
        .pHeapSizeLimit = nullptr,
        .pVulkanFunctions = &vma_vulkan_functions,
        .instance = self->instance->vk_instance,
        .vulkanApiVersion = VK_API_VERSION_1_3,
        .pTypeExternalMemoryHandleTypes = {},
    };

    vmaCreateAllocator(&vma_allocator_create_info, &self->vma_allocator);

    {
        auto buffer_data = std::array<u8, 4>{0xff, 0x00, 0xff, 0xff};

        VkBufferCreateInfo const vk_buffer_create_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .size = sizeof(u8) * 4,
            .usage = BUFFER_USE_FLAGS,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &self->main_queue_family_index,
        };

        VmaAllocationInfo vma_allocation_info = {};

        auto vma_allocation_flags = static_cast<VmaAllocationCreateFlags>(
            VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT |
            VMA_ALLOCATION_CREATE_MAPPED_BIT |
            VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);

        VmaAllocationCreateInfo const vma_allocation_create_info{
            .flags = vma_allocation_flags,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            .requiredFlags = {},
            .preferredFlags = {},
            .memoryTypeBits = std::numeric_limits<u32>::max(),
            .pool = nullptr,
            .pUserData = nullptr,
            .priority = 0.5f,
        };

        [[maybe_unused]] VkResult const vk_create_buffer_result = vmaCreateBuffer(self->vma_allocator, &vk_buffer_create_info, &vma_allocation_create_info, &self->vk_null_buffer, &self->vk_null_buffer_vma_allocation, &vma_allocation_info);
        DAXA_DBG_ASSERT_TRUE_M(vk_create_buffer_result == VK_SUCCESS, "failed to create vk null buffer");

        *static_cast<decltype(buffer_data) *>(vma_allocation_info.pMappedData) = buffer_data;
    }

    {
        auto image_info = ImageInfo{
            .dimensions = 2,
            .format = Format::R8G8B8A8_UNORM,
            .size = {1, 1, 1},
            .mip_level_count = 1,
            .array_layer_count = 1,
            .sample_count = 1,
            .usage = ImageUsageFlagBits::SHADER_SAMPLED | ImageUsageFlagBits::SHADER_STORAGE | ImageUsageFlagBits::TRANSFER_DST,
            .allocate_info = MemoryFlagBits::DEDICATED_MEMORY,
        };
        VkImageCreateInfo const vk_image_create_info = initialize_image_create_info_from_image_info(
            *reinterpret_cast<daxa_ImageInfo const *>(&image_info), &self->main_queue_family_index);

        VmaAllocationCreateInfo const vma_allocation_create_info{
            .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            .requiredFlags = {},
            .preferredFlags = {},
            .memoryTypeBits = std::numeric_limits<u32>::max(),
            .pool = nullptr,
            .pUserData = nullptr,
            .priority = 0.5f,
        };

        [[maybe_unused]] VkResult const vk_create_image_result = vmaCreateImage(self->vma_allocator, &vk_image_create_info, &vma_allocation_create_info, &self->vk_null_image, &self->vk_null_image_vma_allocation, nullptr);
        DAXA_DBG_ASSERT_TRUE_M(vk_create_image_result == VK_SUCCESS, "failed to create vk null image");

        VkImageViewCreateInfo const vk_image_view_create_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .image = self->vk_null_image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = *reinterpret_cast<VkFormat const *>(&image_info.format),
            .components = VkComponentMapping{
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = image_info.mip_level_count,
                .baseArrayLayer = 0,
                .layerCount = image_info.array_layer_count,
            },
        };

        [[maybe_unused]] VkResult const vk_create_image_view_result = vkCreateImageView(self->vk_device, &vk_image_view_create_info, nullptr, &self->vk_null_image_view);
        DAXA_DBG_ASSERT_TRUE_M(vk_create_image_view_result == VK_SUCCESS, "failed to create vk null image view");

        VkImageMemoryBarrier vk_image_mem_barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = {},
            .srcAccessMask = VK_ACCESS_HOST_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .oldLayout = {},
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = self->vk_null_image,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };
        vkCmdPipelineBarrier(init_cmd_buffer, VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {}, {}, {}, {}, 1, &vk_image_mem_barrier);
        VkBufferImageCopy const vk_buffer_image_copy{
            .bufferOffset = 0u,
            .bufferRowLength = 0u,
            .bufferImageHeight = 0u,
            .imageSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .imageOffset = VkOffset3D{0, 0, 0},
            .imageExtent = VkExtent3D{1, 1, 1},
        };
        vkCmdCopyBufferToImage(init_cmd_buffer, self->vk_null_buffer, self->vk_null_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &vk_buffer_image_copy);
        vk_image_mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        vk_image_mem_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
        vk_image_mem_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        vk_image_mem_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL,
        vk_image_mem_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        vk_image_mem_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        vkCmdPipelineBarrier(init_cmd_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {}, {}, {}, {}, 1, &vk_image_mem_barrier);
    }

    {
        VkSamplerCreateInfo const vk_sampler_create_info{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .magFilter = VkFilter::VK_FILTER_LINEAR,
            .minFilter = VkFilter::VK_FILTER_LINEAR,
            .mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .mipLodBias = 0.0f,
            .anisotropyEnable = VK_FALSE,
            .maxAnisotropy = 0,
            .compareEnable = VK_FALSE,
            .compareOp = VkCompareOp::VK_COMPARE_OP_ALWAYS,
            .minLod = 0,
            .maxLod = 0,
            .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
        };
        vkCreateSampler(self->vk_device, &vk_sampler_create_info, nullptr, &self->vk_null_sampler);
    }

    {
        VkBufferUsageFlags const usage_flags =
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        VkBufferCreateInfo const vk_buffer_create_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .size = self->info.max_allowed_buffers * sizeof(u64),
            .usage = usage_flags,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &self->main_queue_family_index,
        };

        VmaAllocationCreateInfo const vma_allocation_create_info{
            .flags = static_cast<VmaAllocationCreateFlags>(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT),
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            .requiredFlags = {},
            .preferredFlags = {},
            .memoryTypeBits = std::numeric_limits<u32>::max(),
            .pool = nullptr,
            .pUserData = nullptr,
            .priority = 0.5f,
        };

        [[maybe_unused]] VkResult const result = vmaCreateBuffer(self->vma_allocator, &vk_buffer_create_info, &vma_allocation_create_info, &self->buffer_device_address_buffer, &self->buffer_device_address_buffer_allocation, nullptr);
        vmaMapMemory(self->vma_allocator, self->buffer_device_address_buffer_allocation, reinterpret_cast<void **>(&self->buffer_device_address_buffer_host_ptr));
        DAXA_DBG_ASSERT_TRUE_M(result == VK_SUCCESS, "failed to create buffer");
    }

    if ((self->instance->info.flags & DAXA_INSTANCE_FLAG_DEBUG_UTIL) != 0 && !self->info_name.empty())
    {
        auto const device_name = self->info_name;
        VkDebugUtilsObjectNameInfoEXT const device_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_DEVICE,
            .objectHandle = reinterpret_cast<uint64_t>(self->vk_device),
            .pObjectName = device_name.c_str(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &device_name_info);

        auto const queue_name = self->info_name;
        VkDebugUtilsObjectNameInfoEXT const device_main_queue_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_QUEUE,
            .objectHandle = reinterpret_cast<uint64_t>(self->main_queue_vk_queue),
            .pObjectName = queue_name.c_str(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &device_main_queue_name_info);

        auto const semaphore_name = self->info_name;
        VkDebugUtilsObjectNameInfoEXT const device_main_queue_timeline_semaphore_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_SEMAPHORE,
            .objectHandle = reinterpret_cast<uint64_t>(self->vk_main_queue_gpu_timeline_semaphore),
            .pObjectName = semaphore_name.c_str(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &device_main_queue_timeline_semaphore_name_info);

        auto const buffer_name = self->info_name;
        VkDebugUtilsObjectNameInfoEXT const device_main_queue_timeline_buffer_device_address_buffer_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_BUFFER,
            .objectHandle = reinterpret_cast<uint64_t>(self->buffer_device_address_buffer),
            .pObjectName = buffer_name.c_str(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &device_main_queue_timeline_buffer_device_address_buffer_name_info);
    }

    self->gpu_shader_resource_table.initialize(
        self->info.max_allowed_buffers,
        self->info.max_allowed_images,
        self->info.max_allowed_samplers,
        self->vk_device,
        self->buffer_device_address_buffer,
        self->vkSetDebugUtilsObjectNameEXT);

    vkEndCommandBuffer(init_cmd_buffer);
    // Submit initial commands to set up the daxa device.
    constexpr VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo init_submit{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = {},
        .waitSemaphoreCount = {},
        .pWaitSemaphores = {},
        .pWaitDstStageMask = &wait_stage_mask,
        .commandBufferCount = 1,
        .pCommandBuffers = &init_cmd_buffer,
        .signalSemaphoreCount = {},
        .pSignalSemaphores = {},
    };
    vkQueueSubmit(self->main_queue_vk_queue, 1, &init_submit, {});
    // Wait for commands in from the init cmd list to complete.
    vkDeviceWaitIdle(self->vk_device);
    vkDestroyCommandPool(self->vk_device, init_cmd_pool, {});

    return {daxa_Result::DAXA_RESULT_SUCCESS, self};
}

void daxa_ImplDevice::main_queue_collect_garbage()
{
    DAXA_ONLY_IF_THREADSAFETY(std::unique_lock lock{this->main_queue_zombies_mtx});

    u64 gpu_timeline_value = std::numeric_limits<u64>::max();
    [[maybe_unused]] auto vk_result = vkGetSemaphoreCounterValue(this->vk_device, this->vk_main_queue_gpu_timeline_semaphore, &gpu_timeline_value);
    DAXA_DBG_ASSERT_TRUE_M(vk_result != VK_ERROR_DEVICE_LOST, "device lost");

    auto check_and_cleanup_gpu_resources = [&](auto & zombies, auto const & cleanup_fn)
    {
        while (!zombies.empty())
        {
            auto & [timeline_value, object] = zombies.back();

            if (timeline_value > gpu_timeline_value)
            {
                break;
            }

            cleanup_fn(object);
            zombies.pop_back();
        }
    };
    // Need to unlock, as command list destructor locks this mutex too.
    lock.unlock();
    check_and_cleanup_gpu_resources(
        this->main_queue_submits_zombies,
        [](auto & /* command_lists */) {});
    lock.lock();
    {
        DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const l_lock{this->main_queue_command_pool_buffer_recycle_mtx});
        check_and_cleanup_gpu_resources(
            this->main_queue_command_list_zombies,
            [&](auto & command_list_zombie)
            {
                this->buffer_pool_pool.put_back({command_list_zombie.vk_cmd_pool, command_list_zombie.vk_cmd_buffer});
            });
    }
    check_and_cleanup_gpu_resources(
        this->main_queue_buffer_zombies,
        [&](auto id)
        {
            this->cleanup_buffer(std::bit_cast<daxa_BufferId>(id));
        });
    check_and_cleanup_gpu_resources(
        this->main_queue_image_view_zombies,
        [&](auto id)
        {
            this->cleanup_image_view(std::bit_cast<daxa_ImageViewId>(id));
        });
    check_and_cleanup_gpu_resources(
        this->main_queue_image_zombies,
        [&](auto id)
        {
            this->cleanup_image(std::bit_cast<daxa_ImageId>(id));
        });
    check_and_cleanup_gpu_resources(
        this->main_queue_sampler_zombies,
        [&](auto id)
        {
            this->cleanup_sampler(std::bit_cast<daxa_SamplerId>(id));
        });
    check_and_cleanup_gpu_resources(
        this->main_queue_pipeline_zombies,
        [&](auto & pipeline_zombie)
        {
            vkDestroyPipeline(this->vk_device, pipeline_zombie.vk_pipeline, nullptr);
        });
    check_and_cleanup_gpu_resources(
        this->main_queue_semaphore_zombies,
        [&](auto & semaphore_zombie)
        {
            vkDestroySemaphore(this->vk_device, semaphore_zombie.vk_semaphore, nullptr);
        });
    check_and_cleanup_gpu_resources(
        this->main_queue_split_barrier_zombies,
        [&](auto & split_barrier_zombie)
        {
            vkDestroyEvent(this->vk_device, split_barrier_zombie.vk_event, nullptr);
        });
    check_and_cleanup_gpu_resources(
        this->main_queue_timeline_query_pool_zombies,
        [&](auto & timeline_query_pool_zombie)
        {
            vkDestroyQueryPool(this->vk_device, timeline_query_pool_zombie.vk_timeline_query_pool, nullptr);
        });
}

void daxa_ImplDevice::wait_idle() const
{
    vkQueueWaitIdle(this->main_queue_vk_queue);
    vkDeviceWaitIdle(this->vk_device);
}

auto daxa_ImplDevice::validate_image_slice(ImageMipArraySlice const & slice, daxa_ImageId id) -> ImageMipArraySlice
{
    if (slice.level_count == std::numeric_limits<u32>::max() || slice.level_count == 0)
    {
        auto & image_info = this->slot(id).info;
        return ImageMipArraySlice{
            .base_mip_level = 0,
            .level_count = image_info.mip_level_count,
            .base_array_layer = 0,
            .layer_count = image_info.array_layer_count,
        };
    }
    else
    {
        return slice;
    }
}

auto daxa_ImplDevice::validate_image_slice(ImageMipArraySlice const & slice, daxa_ImageViewId id) -> ImageMipArraySlice
{
    if (slice.level_count == std::numeric_limits<u32>::max() || slice.level_count == 0)
    {
        return this->slot(id).info.slice;
    }
    else
    {
        return slice;
    }
}

auto daxa_ImplDevice::new_swapchain_image(VkImage swapchain_image, VkFormat format, u32 index, ImageUsageFlags usage, ImageInfo const & image_info) -> ImageId
{
    auto [id, image_slot] = gpu_shader_resource_table.image_slots.new_slot();

    ImplImageSlot ret;
    ret.vk_image = swapchain_image;
    ret.view_slot.info = ImageViewInfo{
        .type = static_cast<ImageViewType>(image_info.dimensions - 1),
        .format = image_info.format,
        .image = {id},
        .slice = ImageMipArraySlice{
            .base_mip_level = 0,
            .level_count = image_info.mip_level_count,
            .base_array_layer = 0,
            .layer_count = image_info.array_layer_count,
        },
        .name = image_info.name,
    };
    ret.aspect_flags = infer_aspect_from_format(image_info.format);

    VkImageViewCreateInfo const view_ci{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = swapchain_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components = {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY,
        },
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    ret.swapchain_image_index = static_cast<i32>(index);

// TODO(capi): Finish impl!
#if 0
    ret.info = image_info;
    vkCreateImageView(vk_device, &view_ci, nullptr, &ret.view_slot.vk_image_view);

    if ((this->instance->info.flags & DAXA_INSTANCE_FLAG_DEBUG_UTIL) != 0 && !image_info.name.empty())
    {
        auto swapchain_image_name = image_info.name;
        VkDebugUtilsObjectNameInfoEXT const swapchain_image_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_IMAGE,
            .objectHandle = reinterpret_cast<uint64_t>(ret.vk_image),
            .pObjectName = swapchain_image_name.c_str(),
        };
        this->vkSetDebugUtilsObjectNameEXT(this->vk_device, &swapchain_image_name_info);

        auto swapchain_image_view_name = image_info.name;
        VkDebugUtilsObjectNameInfoEXT const swapchain_image_view_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
            .objectHandle = reinterpret_cast<uint64_t>(ret.view_slot.vk_image_view),
            .pObjectName = swapchain_image_view_name.c_str(),
        };
        this->vkSetDebugUtilsObjectNameEXT(this->vk_device, &swapchain_image_view_name_info);
    }

    write_descriptor_set_image(this->vk_device, this->gpu_shader_resource_table.vk_descriptor_set, ret.view_slot.vk_image_view, usage, id.index);

    image_slot = ret;

    return ImageId{id};
#endif
    return {};
}

void daxa_ImplDevice::cleanup_buffer(daxa_BufferId id)
{
    auto gid = std::bit_cast<GPUResourceId>(id);
    ImplBufferSlot & buffer_slot = this->gpu_shader_resource_table.buffer_slots.dereference_id(gid);
    this->buffer_device_address_buffer_host_ptr[gid.index] = 0;
    write_descriptor_set_buffer(this->vk_device, this->gpu_shader_resource_table.vk_descriptor_set, this->vk_null_buffer, 0, VK_WHOLE_SIZE, gid.index);
// TODO(capi): Finish impl!
#if 0
    if (std::holds_alternative<AutoAllocInfo>(buffer_slot.info.allocate_info))
    {
        vmaDestroyBuffer(this->vma_allocator, buffer_slot.vk_buffer, buffer_slot.vma_allocation);
    }
    else
    {
        vkDestroyBuffer(this->vk_device, buffer_slot.vk_buffer, {});
    }
    buffer_slot = {};
    gpu_shader_resource_table.buffer_slots.return_slot(id);
#endif
}

void daxa_ImplDevice::cleanup_image(daxa_ImageId id)
{
    auto gid = std::bit_cast<GPUResourceId>(id);
    ImplImageSlot & image_slot = gpu_shader_resource_table.image_slots.dereference_id(gid);
    write_descriptor_set_image(this->vk_device, this->gpu_shader_resource_table.vk_descriptor_set, this->vk_null_image_view, image_slot.info.usage, gid.index);
    vkDestroyImageView(vk_device, image_slot.view_slot.vk_image_view, nullptr);
// TODO(capi): Finish impl!
#if 0
    if (image_slot.swapchain_image_index == NOT_OWNED_BY_SWAPCHAIN)
    {
        if (std::holds_alternative<AutoAllocInfo>(image_slot.info.allocate_info))
        {
            vmaDestroyImage(this->vma_allocator, image_slot.vk_image, image_slot.vma_allocation);
        }
        else
        {
            vkDestroyImage(this->vk_device, image_slot.vk_image, {});
        }
    }
    image_slot = {};
    gpu_shader_resource_table.image_slots.return_slot(id);
#endif
}

void daxa_ImplDevice::cleanup_image_view(daxa_ImageViewId id)
{
    // TODO(capi): Finish impl!
#if 0
    DAXA_DBG_ASSERT_TRUE_M(gpu_shader_resource_table.image_slots.dereference_id(std::bit_cast<GPUResourceId>(id)).vk_image == VK_NULL_HANDLE, "can not destroy default image view of image");
    ImplImageViewSlot & image_slot = gpu_shader_resource_table.image_slots.dereference_id(std::bit_cast<GPUResourceId>(id)).view_slot;
    write_descriptor_set_image(this->vk_device, this->gpu_shader_resource_table.vk_descriptor_set, this->vk_null_image_view, ImageUsageFlagBits::SHADER_STORAGE | ImageUsageFlagBits::SHADER_SAMPLED, id.index);
    vkDestroyImageView(vk_device, image_slot.vk_image_view, nullptr);
    image_slot = {};
    gpu_shader_resource_table.image_slots.return_slot(std::bit_cast<GPUResourceId>(id));
#endif
}

void daxa_ImplDevice::cleanup_sampler(daxa_SamplerId id)
{
    // TODO(capi): Finish impl!
#if 0
    ImplSamplerSlot & sampler_slot = this->gpu_shader_resource_table.sampler_slots.dereference_id(std::bit_cast<GPUResourceId>(id));
    write_descriptor_set_sampler(this->vk_device, this->gpu_shader_resource_table.vk_descriptor_set, this->vk_null_sampler, std::bit_cast<GPUResourceId>(id).index);
    vkDestroySampler(this->vk_device, sampler_slot.vk_sampler, nullptr);
    sampler_slot = {};
    gpu_shader_resource_table.sampler_slots.return_slot(std::bit_cast<GPUResourceId>(id));
#endif
}

void daxa_ImplDevice::zombify_buffer(daxa_BufferId id)
{
    // TODO(capi): Finish impl!
#if 0
    DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const lock{this->main_queue_zombies_mtx});
    u64 const main_queue_cpu_timeline_value = DAXA_ATOMIC_FETCH(this->main_queue_cpu_timeline);
    DAXA_DBG_ASSERT_TRUE_M(gpu_shader_resource_table.buffer_slots.dereference_id(std::bit_cast<GPUResourceId>(id)).zombie == false,
                           "detected free after free - buffer already is a zombie");
    gpu_shader_resource_table.buffer_slots.dereference_id(std::bit_cast<GPUResourceId>(id)).zombie = true;
    this->main_queue_buffer_zombies.push_front({main_queue_cpu_timeline_value, std::bit_cast<GPUResourceId>(id)});
#endif
}

void daxa_ImplDevice::zombify_image(daxa_ImageId id)
{
    // TODO(capi): Finish impl!
#if 0
    DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const lock{this->main_queue_zombies_mtx});
    u64 const main_queue_cpu_timeline_value = DAXA_ATOMIC_FETCH(this->main_queue_cpu_timeline);
    DAXA_DBG_ASSERT_TRUE_M(gpu_shader_resource_table.image_slots.dereference_id(std::bit_cast<GPUResourceId>(id)).zombie == false,
                           "detected free after free - image already is a zombie");
    gpu_shader_resource_table.image_slots.dereference_id(std::bit_cast<GPUResourceId>(id)).zombie = true;
    this->main_queue_image_zombies.push_front({main_queue_cpu_timeline_value, std::bit_cast<GPUResourceId>(id)});
#endif
}

void daxa_ImplDevice::zombify_image_view(daxa_ImageViewId id)
{
    // TODO(capi): Finish impl!
#if 0
    DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const lock{this->main_queue_zombies_mtx});
    u64 const main_queue_cpu_timeline_value = DAXA_ATOMIC_FETCH(this->main_queue_cpu_timeline);
    this->main_queue_image_view_zombies.push_front({main_queue_cpu_timeline_value, std::bit_cast<GPUResourceId>(id)});
#endif
}

void daxa_ImplDevice::zombify_sampler(daxa_SamplerId id)
{
    // TODO(capi): Finish impl!
#if 0
    DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const lock{this->main_queue_zombies_mtx});
    u64 const main_queue_cpu_timeline_value = DAXA_ATOMIC_FETCH(this->main_queue_cpu_timeline);
    DAXA_DBG_ASSERT_TRUE_M(gpu_shader_resource_table.sampler_slots.dereference_id(id).zombie == false,
                           "detected free after free - sampler already is a zombie");
    gpu_shader_resource_table.sampler_slots.dereference_id(std::bit_cast<GPUResourceId>(id)).zombie = true;
    this->main_queue_sampler_zombies.push_front({main_queue_cpu_timeline_value, std::bit_cast<GPUResourceId>(id)});
#endif
}

auto daxa_ImplDevice::slot(daxa_BufferId id) -> ImplBufferSlot &
{
    return gpu_shader_resource_table.buffer_slots.dereference_id(std::bit_cast<GPUResourceId>(id));
}

auto daxa_ImplDevice::slot(daxa_ImageId id) -> ImplImageSlot &
{
    return gpu_shader_resource_table.image_slots.dereference_id(std::bit_cast<GPUResourceId>(id));
}

auto daxa_ImplDevice::slot(daxa_ImageViewId id) -> ImplImageViewSlot &
{
    return gpu_shader_resource_table.image_slots.dereference_id(std::bit_cast<GPUResourceId>(id)).view_slot;
}

auto daxa_ImplDevice::slot(daxa_SamplerId id) -> ImplSamplerSlot &
{
    return gpu_shader_resource_table.sampler_slots.dereference_id(std::bit_cast<GPUResourceId>(id));
}

auto daxa_ImplDevice::slot(daxa_BufferId id) const -> ImplBufferSlot const &
{
    return gpu_shader_resource_table.buffer_slots.dereference_id(std::bit_cast<GPUResourceId>(id));
}

auto daxa_ImplDevice::slot(daxa_ImageId id) const -> ImplImageSlot const &
{
    return gpu_shader_resource_table.image_slots.dereference_id(std::bit_cast<GPUResourceId>(id));
}

auto daxa_ImplDevice::slot(daxa_ImageViewId id) const -> ImplImageViewSlot const &
{
    return gpu_shader_resource_table.image_slots.dereference_id(std::bit_cast<GPUResourceId>(id)).view_slot;
}

auto daxa_ImplDevice::slot(daxa_SamplerId id) const -> ImplSamplerSlot const &
{
    return gpu_shader_resource_table.sampler_slots.dereference_id(std::bit_cast<GPUResourceId>(id));
}

// --- End Internal Functions ---