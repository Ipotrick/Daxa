#include "impl_device.hpp"

#include <utility>

namespace daxa
{
    Device::Device(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}

    static VkBufferUsageFlags const BUFFER_USE_FLAGS =
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT |
        VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT |
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
        VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT |
        VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT |
        VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT |
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
        VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;

    auto initialize_image_create_info_from_image_info(ImageInfo const & image_info, u32 const * queue_family_index_ptr) -> VkImageCreateInfo
    {
        DAXA_DBG_ASSERT_TRUE_M(std::popcount(image_info.sample_count) == 1 && image_info.sample_count <= 64, "image samples must be power of two and between 1 and 64(inclusive)");
        DAXA_DBG_ASSERT_TRUE_M(
            image_info.size.x > 0 &&
                image_info.size.y > 0 &&
                image_info.size.z > 0,
            "image (x,y,z) dimensions must be greater then 0");
        DAXA_DBG_ASSERT_TRUE_M(image_info.array_layer_count > 0, "image array layer count must be greater then 0");
        DAXA_DBG_ASSERT_TRUE_M(image_info.mip_level_count > 0, "image mip level count must be greater then 0");

        auto const vk_image_type = static_cast<VkImageType>(image_info.dimensions - 1);

        VkImageCreateFlags vk_image_create_flags = static_cast<VkImageCreateFlags>(image_info.flags.data);

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
            .usage = image_info.usage.data,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = queue_family_index_ptr,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        return vk_image_create_info;
    }

    auto Device::create_memory(MemoryBlockInfo const & info) -> MemoryBlock
    {
        auto const & impl = *as<ImplDevice>();

        DAXA_DBG_ASSERT_TRUE_M(info.requirements.memory_type_bits != 0, "memory_type_bits must be non zero");

        VkMemoryRequirements requirements = std::bit_cast<VkMemoryRequirements>(info.requirements);
        VmaAllocationCreateFlags const flags = std::bit_cast<VmaAllocationCreateFlags>(info.flags);
        VmaAllocationCreateInfo create_info{
            .flags = flags,
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
        vmaAllocateMemory(impl.vma_allocator, &requirements, &create_info, &allocation, &allocation_info);

        return MemoryBlock{ManagedPtr{new ImplMemoryBlock(this->make_weak(), info, allocation, allocation_info)}};
    }

    auto Device::get_memory_requirements(BufferInfo const & info) -> MemoryRequirements
    {
        auto const & impl = *as<ImplDevice>();
        VkBufferCreateInfo const vk_buffer_create_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .size = static_cast<VkDeviceSize>(info.size),
            .usage = BUFFER_USE_FLAGS,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &impl.main_queue_family_index,
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
        vkGetDeviceBufferMemoryRequirements(impl.vk_device, &buffer_requirement_info, &mem_requirements);
        MemoryRequirements ret = std::bit_cast<MemoryRequirements>(mem_requirements.memoryRequirements);
        return ret;
    }

    auto Device::get_memory_requirements(ImageInfo const & info) -> MemoryRequirements
    {
        auto const & impl = *as<ImplDevice>();
        VkImageCreateInfo vk_image_create_info = initialize_image_create_info_from_image_info(info, &impl.main_queue_family_index);
        VkDeviceImageMemoryRequirements image_requirement_info{
            .sType = VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS,
            .pNext = {},
            .pCreateInfo = &vk_image_create_info,
            .planeAspect = static_cast<VkImageAspectFlagBits>(infer_aspect_from_format(info.format)),
        };
        VkMemoryRequirements2 mem_requirements{
            .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
            .pNext = {},
            .memoryRequirements = {},
        };
        vkGetDeviceImageMemoryRequirements(impl.vk_device, &image_requirement_info, &mem_requirements);
        MemoryRequirements ret = std::bit_cast<MemoryRequirements>(mem_requirements.memoryRequirements);
        return ret;
    }

    auto Device::info() const -> DeviceInfo const &
    {
        auto const & impl = *as<ImplDevice>();
        return impl.info;
    }

    auto Device::properties() const -> DeviceProperties const &
    {
        auto const & impl = *as<ImplDevice>();
        return impl.vk_info;
    }

    auto Device::mesh_shader_properties() const -> MeshShaderDeviceProperties const &
    {
        auto const & impl = *as<ImplDevice>();
        return impl.mesh_shader_properties;
    }

    void Device::wait_idle()
    {
        auto & impl = *as<ImplDevice>();
        impl.wait_idle();
    }

    void Device::submit_commands(CommandSubmitInfo const & submit_info)
    {
        auto & impl = *as<ImplDevice>();

        impl.main_queue_collect_garbage();

        u64 const current_main_queue_cpu_timeline_value = DAXA_ATOMIC_FETCH_INC(impl.main_queue_cpu_timeline) + 1;

        std::pair<u64, std::vector<ManagedPtr>> submit = {current_main_queue_cpu_timeline_value, {}};

        for (auto const & command_list : submit_info.command_lists)
        {
            auto const * cmd_list = command_list.as<ImplCommandList>();
            for (auto [id, index] : cmd_list->deferred_destructions)
            {
                switch (index)
                {
                case DEFERRED_DESTRUCTION_BUFFER_INDEX: impl.main_queue_buffer_zombies.push_front({current_main_queue_cpu_timeline_value, BufferId{id}}); break;
                case DEFERRED_DESTRUCTION_IMAGE_INDEX: impl.main_queue_image_zombies.push_front({current_main_queue_cpu_timeline_value, ImageId{id}}); break;
                case DEFERRED_DESTRUCTION_IMAGE_VIEW_INDEX: impl.main_queue_image_view_zombies.push_front({current_main_queue_cpu_timeline_value, ImageViewId{id}}); break;
                case DEFERRED_DESTRUCTION_SAMPLER_INDEX: impl.main_queue_sampler_zombies.push_front({current_main_queue_cpu_timeline_value, SamplerId{id}}); break;
                default: DAXA_DBG_ASSERT_TRUE_M(false, "unreachable");
                }
            }
        }

        std::vector<VkCommandBuffer> submit_vk_command_buffers = {};
        for (auto const & command_list : submit_info.command_lists)
        {
            auto const & impl_cmd_list = *command_list.as<ImplCommandList>();
            DAXA_DBG_ASSERT_TRUE_M(impl_cmd_list.recording_complete, "all submitted command lists must be completed before submission");
            submit.second.push_back(command_list);
            submit_vk_command_buffers.push_back(impl_cmd_list.vk_cmd_buffer);
        }

        std::vector<VkSemaphore> submit_semaphore_signals = {}; // All timeline semaphores come first, then binary semaphores follow.
        std::vector<u64> submit_semaphore_signal_values = {};   // Used for timeline semaphores. Ignored (push dummy value) for binary semaphores.

        // Add main queue timeline signaling as first timeline semaphore signaling:
        submit_semaphore_signals.push_back(impl.vk_main_queue_gpu_timeline_semaphore);
        submit_semaphore_signal_values.push_back(current_main_queue_cpu_timeline_value);

        for (auto const & [timeline_semaphore, signal_value] : submit_info.signal_timeline_semaphores)
        {
            auto const & impl_timeline_semaphore = *timeline_semaphore.as<ImplTimelineSemaphore>();
            submit_semaphore_signals.push_back(impl_timeline_semaphore.vk_semaphore);
            submit_semaphore_signal_values.push_back(signal_value);
        }

        for (auto const & binary_semaphore : submit_info.signal_binary_semaphores)
        {
            auto const & impl_binary_semaphore = *binary_semaphore.as<ImplBinarySemaphore>();
            submit_semaphore_signals.push_back(impl_binary_semaphore.vk_semaphore);
            submit_semaphore_signal_values.push_back(0); // The vulkan spec requires to have dummy values for binary semaphores.
        }

        // used to synchronize with previous submits:
        std::vector<VkSemaphore> submit_semaphore_waits = {}; // All timeline semaphores come first, then binary semaphores follow.
        std::vector<VkPipelineStageFlags> submit_semaphore_wait_stage_masks = {};
        std::vector<u64> submit_semaphore_wait_values = {}; // Used for timeline semaphores. Ignored (push dummy value) for binary semaphores.

        for (auto const & [timeline_semaphore, wait_value] : submit_info.wait_timeline_semaphores)
        {
            auto const & impl_timeline_semaphore = *timeline_semaphore.as<ImplTimelineSemaphore>();
            submit_semaphore_waits.push_back(impl_timeline_semaphore.vk_semaphore);
            submit_semaphore_wait_stage_masks.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
            submit_semaphore_wait_values.push_back(wait_value);
        }

        for (auto const & binary_semaphore : submit_info.wait_binary_semaphores)
        {
            auto const & impl_binary_semaphore = *binary_semaphore.as<ImplBinarySemaphore>();
            submit_semaphore_waits.push_back(impl_binary_semaphore.vk_semaphore);
            submit_semaphore_wait_stage_masks.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
            submit_semaphore_wait_values.push_back(0);
        }

        VkTimelineSemaphoreSubmitInfo timeline_info{
            .sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreValueCount = static_cast<u32>(submit_semaphore_wait_values.size()),
            .pWaitSemaphoreValues = submit_semaphore_wait_values.data(),
            .signalSemaphoreValueCount = static_cast<u32>(submit_semaphore_signal_values.size()),
            .pSignalSemaphoreValues = submit_semaphore_signal_values.data(),
        };

        VkSubmitInfo const vk_submit_info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = reinterpret_cast<void *>(&timeline_info),
            .waitSemaphoreCount = static_cast<u32>(submit_semaphore_waits.size()),
            .pWaitSemaphores = submit_semaphore_waits.data(),
            .pWaitDstStageMask = submit_semaphore_wait_stage_masks.data(),
            .commandBufferCount = static_cast<u32>(submit_vk_command_buffers.size()),
            .pCommandBuffers = submit_vk_command_buffers.data(),
            .signalSemaphoreCount = static_cast<u32>(submit_semaphore_signals.size()),
            .pSignalSemaphores = submit_semaphore_signals.data(),
        };
        vkQueueSubmit(impl.main_queue_vk_queue, 1, &vk_submit_info, VK_NULL_HANDLE);

        DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const lock{impl.main_queue_zombies_mtx});
        impl.main_queue_submits_zombies.push_front(std::move(submit));
    }

    void Device::present_frame(PresentInfo const & info)
    {
        auto & impl = *as<ImplDevice>();
        auto const & swapchain_impl = *info.swapchain.as<ImplSwapchain>();

        // used to synchronize with previous submits:
        std::vector<VkSemaphore> submit_semaphore_waits = {};

        for (auto const & binary_semaphore : info.wait_binary_semaphores)
        {
            auto const & impl_binary_semaphore = *binary_semaphore.as<ImplBinarySemaphore>();
            submit_semaphore_waits.push_back(impl_binary_semaphore.vk_semaphore);
        }

        VkPresentInfoKHR const present_info{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = static_cast<u32>(submit_semaphore_waits.size()),
            .pWaitSemaphores = submit_semaphore_waits.data(),
            .swapchainCount = static_cast<u32>(1),
            .pSwapchains = &swapchain_impl.vk_swapchain,
            .pImageIndices = &swapchain_impl.current_image_index,
            .pResults = {},
        };

        [[maybe_unused]] VkResult const err = vkQueuePresentKHR(impl.main_queue_vk_queue, &present_info);
        // We currently ignore VK_ERROR_OUT_OF_DATE_KHR, VK_ERROR_SURFACE_LOST_KHR and VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT
        // because supposedly these kinds of things are not specified within the spec. This is also handled in Swapchain::acquire_next_image()
        DAXA_DBG_ASSERT_TRUE_M(err == VK_SUCCESS || err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_ERROR_SURFACE_LOST_KHR || err == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT, "Daxa should never be in a situation where Present fails");

        collect_garbage();
    }

    void Device::collect_garbage()
    {
        auto & impl = *as<ImplDevice>();
        impl.main_queue_collect_garbage();
    }

    auto Device::create_swapchain(SwapchainInfo const & info) -> Swapchain
    {
        return Swapchain{ManagedPtr{new ImplSwapchain(this->make_weak(), info)}};
    }

    auto Device::create_raster_pipeline(RasterPipelineInfo const & info) -> RasterPipeline
    {
        return RasterPipeline{ManagedPtr{new ImplRasterPipeline(this->make_weak(), info)}};
    }

    auto Device::create_compute_pipeline(ComputePipelineInfo const & info) -> ComputePipeline
    {
        return ComputePipeline{ManagedPtr{new ImplComputePipeline(this->make_weak(), info)}};
    }

    auto Device::create_command_list(CommandListInfo const & info) -> CommandList
    {
        auto & impl = *as<ImplDevice>();
        DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const lock{impl.main_queue_command_pool_buffer_recycle_mtx});
        auto [pool, buffer] = impl.buffer_pool_pool.get(&impl);
        return CommandList{ManagedPtr{new ImplCommandList{this->make_weak(), pool, buffer, info}}};
    }

    auto Device::create_binary_semaphore(BinarySemaphoreInfo const & info) -> BinarySemaphore
    {
        return BinarySemaphore(ManagedPtr(new ImplBinarySemaphore{this->make_weak(), info}));
    }

    auto Device::create_timeline_semaphore(TimelineSemaphoreInfo const & info) -> TimelineSemaphore
    {
        return TimelineSemaphore(ManagedPtr(new ImplTimelineSemaphore(this->make_weak(), info)));
    }

    auto Device::create_split_barrier(SplitBarrierInfo const & info) -> SplitBarrierState
    {
        return {this->make_weak(), info};
    }

    auto Device::create_timeline_query_pool(TimelineQueryPoolInfo const & info) -> TimelineQueryPool
    {
        return TimelineQueryPool(ManagedPtr(new ImplTimelineQueryPool(this->make_weak(), info)));
    }

    auto Device::create_buffer(BufferInfo const & info) -> BufferId
    {
        auto & impl = *as<ImplDevice>();
        return impl.new_buffer(info);
    }

    auto Device::create_image(ImageInfo const & info) -> ImageId
    {
        auto & impl = *as<ImplDevice>();
        return impl.new_image(info);
    }

    auto Device::create_image_view(ImageViewInfo const & info) -> ImageViewId
    {
        auto & impl = *as<ImplDevice>();
        return impl.new_image_view(info);
    }

    auto Device::create_sampler(SamplerInfo const & info) -> SamplerId
    {
        auto & impl = *as<ImplDevice>();
        return impl.new_sampler(info);
    }

    void Device::destroy_buffer(BufferId id)
    {
        auto & impl = *as<ImplDevice>();
        impl.zombify_buffer(id);
    }

    void Device::destroy_image(ImageId id)
    {
        auto & impl = *as<ImplDevice>();
        impl.zombify_image(id);
    }

    void Device::destroy_image_view(ImageViewId id)
    {
        auto & impl = *as<ImplDevice>();
        impl.zombify_image_view(id);
    }

    void Device::destroy_sampler(SamplerId id)
    {
        auto & impl = *as<ImplDevice>();
        impl.zombify_sampler(id);
    }

    auto Device::info_buffer(BufferId id) const -> BufferInfo
    {
        auto const & impl = *as<ImplDevice>();
        DAXA_DBG_ASSERT_TRUE_M(is_id_valid(id), "detected invalid buffer id");
        return impl.slot(id).info;
    }

    auto Device::get_device_address(BufferId id) const -> BufferDeviceAddress
    {
        auto const & impl = *as<ImplDevice>();
        return BufferDeviceAddress{static_cast<u64>(impl.slot(id).device_address)};
    }

    auto Device::get_host_address(BufferId id) const -> void *
    {
        auto const & impl = *as<ImplDevice>();
        DAXA_DBG_ASSERT_TRUE_M(
            impl.slot(id).host_address != nullptr,
            "host buffer address is only available if the buffer is created with either of the following memory flags: HOST_ACCESS_RANDOM, HOST_ACCESS_SEQUENTIAL_WRITE");
        return impl.slot(id).host_address;
    }

    auto Device::info_image(ImageId id) const -> ImageInfo
    {
        auto const & impl = *as<ImplDevice>();
        DAXA_DBG_ASSERT_TRUE_M(is_id_valid(id), "detected invalid image id");
        return impl.slot(id).info;
    }

    auto Device::info_image_view(ImageViewId id) const -> ImageViewInfo
    {
        auto const & impl = *as<ImplDevice>();
        DAXA_DBG_ASSERT_TRUE_M(is_id_valid(id), "detected invalid image view id");
        return impl.slot(id).info;
    }

    auto Device::info_sampler(SamplerId id) const -> SamplerInfo
    {
        auto const & impl = *as<ImplDevice>();
        DAXA_DBG_ASSERT_TRUE_M(is_id_valid(id), "detected invalid sampler id");
        return impl.slot(id).info;
    }

    auto Device::is_id_valid(ImageId id) const -> bool
    {
        auto const & impl = *as<ImplDevice>();
        return !id.is_empty() && impl.gpu_shader_resource_table.image_slots.is_id_valid(id);
    }

    auto Device::is_id_valid(ImageViewId id) const -> bool
    {
        auto const & impl = *as<ImplDevice>();
        return !id.is_empty() && impl.gpu_shader_resource_table.image_slots.is_id_valid(id);
    }

    auto Device::is_id_valid(BufferId id) const -> bool
    {
        auto const & impl = *as<ImplDevice>();
        return !id.is_empty() && impl.gpu_shader_resource_table.buffer_slots.is_id_valid(id);
    }
    auto Device::is_id_valid(SamplerId id) const -> bool
    {
        auto const & impl = *as<ImplDevice>();
        return !id.is_empty() && impl.gpu_shader_resource_table.sampler_slots.is_id_valid(id);
    }

    ImplDevice::ImplDevice(DeviceInfo a_info, ManagedWeakPtr a_impl_ctx, VkPhysicalDevice a_physical_device)
        : impl_ctx{std::move(a_impl_ctx)},
          vk_physical_device{a_physical_device},
          info{std::move(a_info)},
          main_queue_family_index(std::numeric_limits<u32>::max())
    {
        VkPhysicalDeviceProperties2 vk_physical_device_properties2 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
            .pNext = {},
            .properties = {},
        };
        vkGetPhysicalDeviceProperties2(vk_physical_device, &vk_physical_device_properties2);
        vk_info = *reinterpret_cast<DeviceProperties *>(&vk_physical_device_properties2.properties);

        // SELECT QUEUE

        u32 queue_family_props_count = 0;
        std::vector<VkQueueFamilyProperties> queue_props;
        vkGetPhysicalDeviceQueueFamilyProperties(a_physical_device, &queue_family_props_count, nullptr);
        queue_props.resize(queue_family_props_count);
        vkGetPhysicalDeviceQueueFamilyProperties(a_physical_device, &queue_family_props_count, queue_props.data());
        std::vector<VkBool32> supports_present;
        supports_present.resize(queue_family_props_count);

        // Maybe check if device supports present for this surface?
        // this requires that the surface was already created.. which
        // is really CRINGE
        // for (u32 i = 0; i < queue_family_props_count; i++)
        //     vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &supports_present[i]);

        for (u32 i = 0; i < queue_family_props_count; i++)
        {
            if ((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 && (queue_props[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0 && (queue_props[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0)
            {
                this->main_queue_family_index = i;
                break;
            }
        }
        DAXA_DBG_ASSERT_TRUE_M(this->main_queue_family_index != std::numeric_limits<u32>::max(), "found no suitable queue family");

        std::array<f32, 1> queue_priorities = {0.0};
        VkDeviceQueueCreateInfo const queue_ci{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = this->main_queue_family_index,
            .queueCount = static_cast<u32>(queue_priorities.size()),
            .pQueuePriorities = queue_priorities.data(),
        };

        VkPhysicalDeviceFeatures const REQUIRED_PHYSICAL_DEVICE_FEATURES{
            .robustBufferAccess = VK_FALSE,
            .fullDrawIndexUint32 = VK_FALSE,
            .imageCubeArray = VK_TRUE,
            .independentBlend = VK_TRUE,
            .geometryShader = VK_FALSE,
            .tessellationShader = VK_TRUE,
            .sampleRateShading = VK_FALSE,
            .dualSrcBlend = VK_FALSE,
            .logicOp = VK_FALSE,
            .multiDrawIndirect = VK_TRUE, // Very useful for gpu driven rendering
            .drawIndirectFirstInstance = VK_FALSE,
            .depthClamp = VK_TRUE, // NOTE(msakmary) need this for bikeshed if breaks ping me
            .depthBiasClamp = VK_FALSE,
            .fillModeNonSolid = VK_TRUE,
            .depthBounds = VK_FALSE,
            .wideLines = VK_TRUE,
            .largePoints = VK_FALSE,
            .alphaToOne = VK_FALSE,
            .multiViewport = VK_FALSE,
            .samplerAnisotropy = VK_TRUE, // Allows for anisotropic filtering.
            .textureCompressionETC2 = VK_FALSE,
            .textureCompressionASTC_LDR = VK_FALSE,
            .textureCompressionBC = VK_FALSE,
            .occlusionQueryPrecise = VK_FALSE,
            .pipelineStatisticsQuery = VK_FALSE,
            .vertexPipelineStoresAndAtomics = VK_FALSE,
            .fragmentStoresAndAtomics = VK_TRUE,
            .shaderTessellationAndGeometryPointSize = VK_FALSE,
            .shaderImageGatherExtended = VK_FALSE,
            .shaderStorageImageExtendedFormats = VK_FALSE,
            .shaderStorageImageMultisample = VK_TRUE,            // Useful for software vrs.
            .shaderStorageImageReadWithoutFormat = VK_TRUE,      // This allows daxa shaders to not specify image layout for image binding tables and read ops.
            .shaderStorageImageWriteWithoutFormat = VK_TRUE,     // This allows daxa shaders to not specify image layout for image binding tables and write ops.
            .shaderUniformBufferArrayDynamicIndexing = VK_FALSE, // This is superseded by descriptor indexing.
            .shaderSampledImageArrayDynamicIndexing = VK_FALSE,  // This is superseded by descriptor indexing.
            .shaderStorageBufferArrayDynamicIndexing = VK_FALSE, // This is superseded by descriptor indexing.
            .shaderStorageImageArrayDynamicIndexing = VK_FALSE,  // This is superseded by descriptor indexing.
            .shaderClipDistance = VK_FALSE,
            .shaderCullDistance = VK_FALSE,
            .shaderFloat64 = VK_FALSE,
            .shaderInt64 = VK_TRUE, // Used for buffer device address math.
            .shaderInt16 = VK_FALSE,
            .shaderResourceResidency = VK_FALSE,
            .shaderResourceMinLod = VK_FALSE,
            .sparseBinding = VK_FALSE,
            .sparseResidencyBuffer = VK_FALSE,
            .sparseResidencyImage2D = VK_FALSE,
            .sparseResidencyImage3D = VK_FALSE,
            .sparseResidency2Samples = VK_FALSE,
            .sparseResidency4Samples = VK_FALSE,
            .sparseResidency8Samples = VK_FALSE,
            .sparseResidency16Samples = VK_FALSE,
            .sparseResidencyAliased = VK_FALSE,
            .variableMultisampleRate = VK_FALSE,
            .inheritedQueries = VK_FALSE,
        };

        void * REQUIRED_DEVICE_FEATURE_P_CHAIN = nullptr;

        VkPhysicalDeviceVulkanMemoryModelFeatures REQUIRED_PHYSICAL_DEVICE_FEATURES_VULKAN_MEMORY_MODEL{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES,
            .pNext = REQUIRED_DEVICE_FEATURE_P_CHAIN,
            .vulkanMemoryModel = VK_TRUE,
            .vulkanMemoryModelDeviceScope = VK_TRUE,
            .vulkanMemoryModelAvailabilityVisibilityChains = VK_FALSE, // Low support.
        };
        REQUIRED_DEVICE_FEATURE_P_CHAIN = reinterpret_cast<void *>(&REQUIRED_PHYSICAL_DEVICE_FEATURES_VULKAN_MEMORY_MODEL);

        VkPhysicalDeviceBufferDeviceAddressFeatures REQUIRED_PHYSICAL_DEVICE_FEATURES_BUFFER_DEVICE_ADDRESS{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
            .pNext = REQUIRED_DEVICE_FEATURE_P_CHAIN,
            .bufferDeviceAddress = VK_TRUE,
            .bufferDeviceAddressCaptureReplay = static_cast<VkBool32>(this->info.enable_buffer_device_address_capture_replay),
            .bufferDeviceAddressMultiDevice = VK_FALSE,
        };
        REQUIRED_DEVICE_FEATURE_P_CHAIN = reinterpret_cast<void *>(&REQUIRED_PHYSICAL_DEVICE_FEATURES_BUFFER_DEVICE_ADDRESS);

        VkPhysicalDeviceDescriptorIndexingFeatures REQUIRED_PHYSICAL_DEVICE_FEATURES_DESCRIPTOR_INDEXING{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
            .pNext = REQUIRED_DEVICE_FEATURE_P_CHAIN,
            .shaderInputAttachmentArrayDynamicIndexing = VK_FALSE,
            .shaderUniformTexelBufferArrayDynamicIndexing = VK_FALSE,
            .shaderStorageTexelBufferArrayDynamicIndexing = VK_FALSE,
            .shaderUniformBufferArrayNonUniformIndexing = VK_FALSE,
            .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,  // Needed for bindless sampled images.
            .shaderStorageBufferArrayNonUniformIndexing = VK_TRUE, // Needed for bindless buffers.
            .shaderStorageImageArrayNonUniformIndexing = VK_TRUE,  // Needed for bindless storage images.
            .shaderInputAttachmentArrayNonUniformIndexing = VK_FALSE,
            .shaderUniformTexelBufferArrayNonUniformIndexing = VK_FALSE,
            .shaderStorageTexelBufferArrayNonUniformIndexing = VK_FALSE,
            .descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE,
            .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,  // Needed for bindless sampled images.
            .descriptorBindingStorageImageUpdateAfterBind = VK_TRUE,  // Needed for bindless storage images.
            .descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE, // Needed for bindless buffers.
            .descriptorBindingUniformTexelBufferUpdateAfterBind = VK_FALSE,
            .descriptorBindingStorageTexelBufferUpdateAfterBind = VK_FALSE,
            .descriptorBindingUpdateUnusedWhilePending = VK_TRUE, // Needed for bindless table updates.
            .descriptorBindingPartiallyBound = VK_TRUE,           // Needed for sparse binding in bindless table.
            .descriptorBindingVariableDescriptorCount = VK_FALSE,
            .runtimeDescriptorArray = VK_TRUE, // Allows shaders to not have a hardcoded descriptor maximum per table.
        };
        REQUIRED_DEVICE_FEATURE_P_CHAIN = reinterpret_cast<void *>(&REQUIRED_PHYSICAL_DEVICE_FEATURES_DESCRIPTOR_INDEXING);

        VkPhysicalDeviceHostQueryResetFeatures REQUIRED_PHYSICAL_DEVICE_FEATURES_HOST_QUERY_RESET{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES,
            .pNext = REQUIRED_DEVICE_FEATURE_P_CHAIN,
            .hostQueryReset = VK_TRUE,
        };
        REQUIRED_DEVICE_FEATURE_P_CHAIN = reinterpret_cast<void *>(&REQUIRED_PHYSICAL_DEVICE_FEATURES_HOST_QUERY_RESET);

        VkPhysicalDeviceShaderAtomicInt64Features REQUIRED_PHYSICAL_DEVICE_FEATURES_SHADER_ATOMIC_INT64{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES,
            .pNext = REQUIRED_DEVICE_FEATURE_P_CHAIN,
            .shaderBufferInt64Atomics = VK_TRUE,
            .shaderSharedInt64Atomics = VK_TRUE,
        };
        REQUIRED_DEVICE_FEATURE_P_CHAIN = reinterpret_cast<void *>(&REQUIRED_PHYSICAL_DEVICE_FEATURES_SHADER_ATOMIC_INT64);

        VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT REQUIRED_PHYSICAL_DEVICE_FEATURES_SHADER_IMAGE_ATOMIC_INT64{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT,
            .pNext = REQUIRED_DEVICE_FEATURE_P_CHAIN,
            .shaderImageInt64Atomics = VK_TRUE,
            .sparseImageInt64Atomics = VK_FALSE, // I do not care about sparse images.
        };
        REQUIRED_DEVICE_FEATURE_P_CHAIN = reinterpret_cast<void *>(&REQUIRED_PHYSICAL_DEVICE_FEATURES_SHADER_IMAGE_ATOMIC_INT64);

        VkPhysicalDeviceDynamicRenderingFeatures REQUIRED_PHYSICAL_DEVICE_FEATURES_DYNAMIC_RENDERING{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
            .pNext = REQUIRED_DEVICE_FEATURE_P_CHAIN,
            .dynamicRendering = VK_TRUE,
        };
        REQUIRED_DEVICE_FEATURE_P_CHAIN = reinterpret_cast<void *>(&REQUIRED_PHYSICAL_DEVICE_FEATURES_DYNAMIC_RENDERING);

        VkPhysicalDeviceTimelineSemaphoreFeatures REQUIRED_PHYSICAL_DEVICE_FEATURES_TIMELINE_SEMAPHORE{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES,
            .pNext = REQUIRED_DEVICE_FEATURE_P_CHAIN,
            .timelineSemaphore = VK_TRUE,
        };
        REQUIRED_DEVICE_FEATURE_P_CHAIN = reinterpret_cast<void *>(&REQUIRED_PHYSICAL_DEVICE_FEATURES_TIMELINE_SEMAPHORE);

        VkPhysicalDeviceSynchronization2Features REQUIRED_PHYSICAL_DEVICE_FEATURES_SYNCHRONIZATION_2{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR,
            .pNext = REQUIRED_DEVICE_FEATURE_P_CHAIN,
            .synchronization2 = VK_TRUE,
        };
        
        REQUIRED_DEVICE_FEATURE_P_CHAIN = reinterpret_cast<void *>(&REQUIRED_PHYSICAL_DEVICE_FEATURES_SYNCHRONIZATION_2);

        VkPhysicalDeviceScalarBlockLayoutFeatures REQUIRED_PHYSICAL_DEVICE_FEATURES_SCALAR_LAYOUT{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES,
            .pNext = REQUIRED_DEVICE_FEATURE_P_CHAIN,
            .scalarBlockLayout = VK_TRUE,
        };
        REQUIRED_DEVICE_FEATURE_P_CHAIN = reinterpret_cast<void *>(&REQUIRED_PHYSICAL_DEVICE_FEATURES_SCALAR_LAYOUT);

        std::vector<char const *> extension_names;
        std::vector<char const *> enabled_layers;

        extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        extension_names.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
        extension_names.push_back(VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME); // might be a problem, intel does not support it at all.
        extension_names.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);

        if (this->info.enable_conservative_rasterization)
        {
            extension_names.push_back(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
        }

        // Mesh shading
        VkPhysicalDeviceMeshShaderFeaturesEXT REQUIRED_PHYSICAL_DEVICE_FEATURES_MESH_SHADER{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
            .pNext = REQUIRED_DEVICE_FEATURE_P_CHAIN,
            .taskShader = VK_TRUE,
            .meshShader = VK_TRUE,
            .multiviewMeshShader = VK_FALSE,
            .primitiveFragmentShadingRateMeshShader = VK_FALSE,
            .meshShaderQueries = VK_FALSE,
        };
        if (this->info.enable_mesh_shader)
        {
            extension_names.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
            REQUIRED_DEVICE_FEATURE_P_CHAIN = reinterpret_cast<void *>(&REQUIRED_PHYSICAL_DEVICE_FEATURES_MESH_SHADER);
        }

        VkPhysicalDeviceFeatures2 physical_device_features_2{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = REQUIRED_DEVICE_FEATURE_P_CHAIN,
            .features = REQUIRED_PHYSICAL_DEVICE_FEATURES,
        };
        REQUIRED_DEVICE_FEATURE_P_CHAIN = reinterpret_cast<void *>(&physical_device_features_2);

        VkDeviceCreateInfo const device_ci = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = REQUIRED_DEVICE_FEATURE_P_CHAIN,
            .flags = {},
            .queueCreateInfoCount = static_cast<u32>(1),
            .pQueueCreateInfos = &queue_ci,
            .enabledLayerCount = static_cast<u32>(enabled_layers.size()),
            .ppEnabledLayerNames = enabled_layers.data(),
            .enabledExtensionCount = static_cast<u32>(extension_names.size()),
            .ppEnabledExtensionNames = extension_names.data(),
            .pEnabledFeatures = nullptr,
        };
        vkCreateDevice(a_physical_device, &device_ci, nullptr, &this->vk_device);

        this->vkCmdPushDescriptorSetKHR = reinterpret_cast<PFN_vkCmdPushDescriptorSetKHR>(vkGetDeviceProcAddr(this->vk_device, "vkCmdPushDescriptorSetKHR"));

        if (this->info.enable_mesh_shader)
        {
            this->vkCmdDrawMeshTasksEXT = reinterpret_cast<PFN_vkCmdDrawMeshTasksEXT>(vkGetDeviceProcAddr(this->vk_device, "vkCmdDrawMeshTasksEXT"));
            this->vkCmdDrawMeshTasksIndirectEXT = reinterpret_cast<PFN_vkCmdDrawMeshTasksIndirectEXT>(vkGetDeviceProcAddr(this->vk_device, "vkCmdDrawMeshTasksIndirectEXT"));
            this->vkCmdDrawMeshTasksIndirectCountEXT = reinterpret_cast<PFN_vkCmdDrawMeshTasksIndirectCountEXT>(vkGetDeviceProcAddr(this->vk_device, "vkCmdDrawMeshTasksIndirectCountEXT"));
            auto pnext_ptr = vk_physical_device_properties2.pNext;
            while (pnext_ptr != 0)
            {
                if (*reinterpret_cast<VkStructureType *>(pnext_ptr) == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT)
                {
                    VkPhysicalDeviceMeshShaderPropertiesEXT * prop_ptr = reinterpret_cast<VkPhysicalDeviceMeshShaderPropertiesEXT *>(pnext_ptr);
                    this->mesh_shader_properties = *reinterpret_cast<MeshShaderDeviceProperties *>(reinterpret_cast<u64 *>(prop_ptr) + 2 /* skip sType and pNext ptrs*/);
                }
                pnext_ptr = reinterpret_cast<void *>(reinterpret_cast<u64 *>(pnext_ptr)[1]);
            }
        }

        vkGetDeviceQueue(this->vk_device, this->main_queue_family_index, 0, &this->main_queue_vk_queue);

        VkCommandPool init_cmd_pool = {};
        VkCommandBuffer init_cmd_buffer = {};
        VkCommandPoolCreateInfo const vk_command_pool_create_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = this->main_queue_family_index,
        };

        vkCreateCommandPool(this->vk_device, &vk_command_pool_create_info, nullptr, &init_cmd_pool);

        VkCommandBufferAllocateInfo const vk_command_buffer_allocate_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = init_cmd_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        vkAllocateCommandBuffers(this->vk_device, &vk_command_buffer_allocate_info, &init_cmd_buffer);

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

        vkCreateSemaphore(this->vk_device, &vk_semaphore_create_info, nullptr, &this->vk_main_queue_gpu_timeline_semaphore);

        DAXA_DBG_ASSERT_TRUE_M(
            this->info.max_allowed_buffers <= this->vk_info.limits.max_descriptor_set_storage_buffers,
            std::string("device does not support ") +
                std::to_string(this->info.max_allowed_buffers) +
                " buffers, the device supports up to " +
                std::to_string(this->vk_info.limits.max_descriptor_set_storage_buffers) +
                "buffers.");
        [[maybe_unused]] auto const max_device_supported_images_in_set = std::min(this->vk_info.limits.max_descriptor_set_sampled_images, this->vk_info.limits.max_descriptor_set_storage_images);
        DAXA_DBG_ASSERT_TRUE_M(
            this->info.max_allowed_images <= max_device_supported_images_in_set,
            std::string("device does not support ") +
                std::to_string(this->info.max_allowed_images) +
                " images, the device supports up to " +
                std::to_string(max_device_supported_images_in_set) +
                "images.");
        DAXA_DBG_ASSERT_TRUE_M(
            this->info.max_allowed_samplers <= this->vk_info.limits.max_descriptor_set_samplers,
            std::string("device does not support ") +
                std::to_string(this->info.max_allowed_samplers) +
                " samplers, the device supports up to " +
                std::to_string(this->vk_info.limits.max_descriptor_set_samplers) +
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
            .physicalDevice = this->vk_physical_device,
            .device = this->vk_device,
            .preferredLargeHeapBlockSize = 0, // Sets it to lib internal default (256MiB).
            .pAllocationCallbacks = nullptr,
            .pDeviceMemoryCallbacks = nullptr,
            .pHeapSizeLimit = nullptr,
            .pVulkanFunctions = &vma_vulkan_functions,
            .instance = this->impl_ctx.as<ImplInstance>()->vk_instance,
            .vulkanApiVersion = VK_API_VERSION_1_3,
            .pTypeExternalMemoryHandleTypes = {},
        };

        vmaCreateAllocator(&vma_allocator_create_info, &this->vma_allocator);

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
                .pQueueFamilyIndices = &this->main_queue_family_index,
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

            [[maybe_unused]] VkResult const vk_create_buffer_result = vmaCreateBuffer(this->vma_allocator, &vk_buffer_create_info, &vma_allocation_create_info, &this->vk_null_buffer, &this->vk_null_buffer_vma_allocation, &vma_allocation_info);
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
            VkImageCreateInfo const vk_image_create_info = initialize_image_create_info_from_image_info(image_info, &this->main_queue_family_index);

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

            [[maybe_unused]] VkResult const vk_create_image_result = vmaCreateImage(this->vma_allocator, &vk_image_create_info, &vma_allocation_create_info, &this->vk_null_image, &this->vk_null_image_vma_allocation, nullptr);
            DAXA_DBG_ASSERT_TRUE_M(vk_create_image_result == VK_SUCCESS, "failed to create vk null image");

            VkImageViewCreateInfo const vk_image_view_create_info{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .image = this->vk_null_image,
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

            [[maybe_unused]] VkResult const vk_create_image_view_result = vkCreateImageView(vk_device, &vk_image_view_create_info, nullptr, &this->vk_null_image_view);
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
                .image = vk_null_image,
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
            vkCmdCopyBufferToImage(init_cmd_buffer, vk_null_buffer, vk_null_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &vk_buffer_image_copy);
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
            vkCreateSampler(vk_device, &vk_sampler_create_info, nullptr, &this->vk_null_sampler);
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
                .size = this->info.max_allowed_buffers * sizeof(u64),
                .usage = usage_flags,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .queueFamilyIndexCount = 1,
                .pQueueFamilyIndices = &this->main_queue_family_index,
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

            [[maybe_unused]] VkResult const result = vmaCreateBuffer(this->vma_allocator, &vk_buffer_create_info, &vma_allocation_create_info, &buffer_device_address_buffer, &buffer_device_address_buffer_allocation, nullptr);
            vmaMapMemory(this->vma_allocator, this->buffer_device_address_buffer_allocation, reinterpret_cast<void **>(&this->buffer_device_address_buffer_host_ptr));
            DAXA_DBG_ASSERT_TRUE_M(result == VK_SUCCESS, "failed to create buffer");
        }

        if (this->impl_ctx.as<ImplInstance>()->enable_debug_names && !this->info.name.empty())
        {
            auto const device_name = this->info.name;
            VkDebugUtilsObjectNameInfoEXT const device_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_DEVICE,
                .objectHandle = reinterpret_cast<uint64_t>(this->vk_device),
                .pObjectName = device_name.c_str(),
            };
            this->impl_ctx.as<ImplInstance>()->vkSetDebugUtilsObjectNameEXT(vk_device, &device_name_info);

            auto const queue_name = this->info.name;
            VkDebugUtilsObjectNameInfoEXT const device_main_queue_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_QUEUE,
                .objectHandle = reinterpret_cast<uint64_t>(this->main_queue_vk_queue),
                .pObjectName = queue_name.c_str(),
            };
            this->impl_ctx.as<ImplInstance>()->vkSetDebugUtilsObjectNameEXT(vk_device, &device_main_queue_name_info);

            auto const semaphore_name = this->info.name;
            VkDebugUtilsObjectNameInfoEXT const device_main_queue_timeline_semaphore_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_SEMAPHORE,
                .objectHandle = reinterpret_cast<uint64_t>(this->vk_main_queue_gpu_timeline_semaphore),
                .pObjectName = semaphore_name.c_str(),
            };
            this->impl_ctx.as<ImplInstance>()->vkSetDebugUtilsObjectNameEXT(vk_device, &device_main_queue_timeline_semaphore_name_info);

            auto const buffer_name = this->info.name;
            VkDebugUtilsObjectNameInfoEXT const device_main_queue_timeline_buffer_device_address_buffer_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_BUFFER,
                .objectHandle = reinterpret_cast<uint64_t>(this->buffer_device_address_buffer),
                .pObjectName = buffer_name.c_str(),
            };
            this->impl_ctx.as<ImplInstance>()->vkSetDebugUtilsObjectNameEXT(vk_device, &device_main_queue_timeline_buffer_device_address_buffer_name_info);
        }

        gpu_shader_resource_table.initialize(
            this->info.max_allowed_buffers,
            this->info.max_allowed_images,
            this->info.max_allowed_samplers,
            vk_device,
            buffer_device_address_buffer,
            this->impl_ctx.as<ImplInstance>()->vkSetDebugUtilsObjectNameEXT);

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
        vkQueueSubmit(this->main_queue_vk_queue, 1, &init_submit, {});
        // Wait for commands in from the init cmd list to complete.
        vkDeviceWaitIdle(this->vk_device);
        vkDestroyCommandPool(this->vk_device, init_cmd_pool, {});
    }

    void ImplDevice::main_queue_collect_garbage()
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
                this->cleanup_buffer(id);
            });
        check_and_cleanup_gpu_resources(
            this->main_queue_image_view_zombies, [&](auto id)
            { this->cleanup_image_view(id); });
        check_and_cleanup_gpu_resources(
            this->main_queue_image_zombies,
            [&](auto id)
            {
                this->cleanup_image(id);
            });
        check_and_cleanup_gpu_resources(
            this->main_queue_sampler_zombies,
            [&](auto id)
            {
                this->cleanup_sampler(id);
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

    void ImplDevice::wait_idle() const
    {
        vkQueueWaitIdle(this->main_queue_vk_queue);
        vkDeviceWaitIdle(this->vk_device);
    }

    auto ImplDevice::new_buffer(BufferInfo const & buffer_info) -> BufferId
    {
        auto [id, ret] = gpu_shader_resource_table.buffer_slots.new_slot();

        DAXA_DBG_ASSERT_TRUE_M(buffer_info.size > 0, "can not create buffers with size zero");

        ret.info = buffer_info;

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
        if (AutoAllocInfo const * auto_info = std::get_if<AutoAllocInfo>(&buffer_info.allocate_info))
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

            [[maybe_unused]] VkResult const vk_create_buffer_result = vmaCreateBuffer(this->vma_allocator, &vk_buffer_create_info, &vma_allocation_create_info, &ret.vk_buffer, &ret.vma_allocation, &vma_allocation_info);
            DAXA_DBG_ASSERT_TRUE_M(vk_create_buffer_result == VK_SUCCESS, "failed to create buffer");
        }
        else
        {
            ManualAllocInfo const & manual_info = std::get<ManualAllocInfo>(buffer_info.allocate_info);
            ImplMemoryBlock const & mem_block = *manual_info.memory_block.as<ImplMemoryBlock const>();

            // TODO(pahrens): Add validation for memory type requirements.

            vkCreateBuffer(this->vk_device, &vk_buffer_create_info, nullptr, &ret.vk_buffer);

            vmaBindBufferMemory2(
                this->vma_allocator,
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

        ret.device_address = vkGetBufferDeviceAddress(vk_device, &vk_buffer_device_address_info);

        ret.host_address = host_accessible ? vma_allocation_info.pMappedData : nullptr;
        ret.zombie = false;

        this->buffer_device_address_buffer_host_ptr[id.index] = ret.device_address;

        if (this->impl_ctx.as<ImplInstance>()->enable_debug_names && !buffer_info.name.empty())
        {
            auto const buffer_name = buffer_info.name;
            VkDebugUtilsObjectNameInfoEXT const buffer_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_BUFFER,
                .objectHandle = reinterpret_cast<uint64_t>(ret.vk_buffer),
                .pObjectName = buffer_name.c_str(),
            };
            this->impl_ctx.as<ImplInstance>()->vkSetDebugUtilsObjectNameEXT(vk_device, &buffer_name_info);
        }

        write_descriptor_set_buffer(this->vk_device, this->gpu_shader_resource_table.vk_descriptor_set, ret.vk_buffer, 0, static_cast<VkDeviceSize>(buffer_info.size), id.index);

        return BufferId{id};
    }

    auto ImplDevice::validate_image_slice(ImageMipArraySlice const & slice, ImageId id) -> ImageMipArraySlice
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

    auto ImplDevice::validate_image_slice(ImageMipArraySlice const & slice, ImageViewId id) -> ImageMipArraySlice
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

    auto ImplDevice::new_swapchain_image(VkImage swapchain_image, VkFormat format, u32 index, ImageUsageFlags usage, ImageInfo const & image_info) -> ImageId
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
        ret.info = image_info;
        vkCreateImageView(vk_device, &view_ci, nullptr, &ret.view_slot.vk_image_view);

        if (this->impl_ctx.as<ImplInstance>()->enable_debug_names && !image_info.name.empty())
        {
            auto swapchain_image_name = image_info.name;
            VkDebugUtilsObjectNameInfoEXT const swapchain_image_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_IMAGE,
                .objectHandle = reinterpret_cast<uint64_t>(ret.vk_image),
                .pObjectName = swapchain_image_name.c_str(),
            };
            this->impl_ctx.as<ImplInstance>()->vkSetDebugUtilsObjectNameEXT(this->vk_device, &swapchain_image_name_info);

            auto swapchain_image_view_name = image_info.name;
            VkDebugUtilsObjectNameInfoEXT const swapchain_image_view_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
                .objectHandle = reinterpret_cast<uint64_t>(ret.view_slot.vk_image_view),
                .pObjectName = swapchain_image_view_name.c_str(),
            };
            this->impl_ctx.as<ImplInstance>()->vkSetDebugUtilsObjectNameEXT(this->vk_device, &swapchain_image_view_name_info);
        }

        write_descriptor_set_image(this->vk_device, this->gpu_shader_resource_table.vk_descriptor_set, ret.view_slot.vk_image_view, usage, id.index);

        image_slot = ret;

        return ImageId{id};
    }

    auto ImplDevice::new_image(ImageInfo const & image_info) -> ImageId
    {
        auto [id, image_slot_variant] = gpu_shader_resource_table.image_slots.new_slot();
        DAXA_DBG_ASSERT_TRUE_M(image_info.dimensions >= 1 && image_info.dimensions <= 3, "image dimensions must be a value between 1 to 3(inclusive)");
        ImplImageSlot ret = {};
        ret.zombie = false;
        ret.info = image_info;
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
        VkImageCreateInfo const vk_image_create_info = initialize_image_create_info_from_image_info(image_info, &this->main_queue_family_index);
        if (AutoAllocInfo const * auto_info = std::get_if<AutoAllocInfo>(&image_info.allocate_info))
        {
            VmaAllocationCreateInfo const vma_allocation_create_info{
                .flags = static_cast<VmaAllocationCreateFlags>(auto_info->data),
                .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                .requiredFlags = {},
                .preferredFlags = {},
                .memoryTypeBits = std::numeric_limits<u32>::max(),
                .pool = nullptr,
                .pUserData = nullptr,
                .priority = 0.5f,
            };

            [[maybe_unused]] VkResult const vk_create_image_result = vmaCreateImage(this->vma_allocator, &vk_image_create_info, &vma_allocation_create_info, &ret.vk_image, &ret.vma_allocation, nullptr);
            DAXA_DBG_ASSERT_TRUE_M(vk_create_image_result == VK_SUCCESS, "failed to create image");
        }
        else
        {
            ManualAllocInfo const & manual_info = std::get<ManualAllocInfo>(image_info.allocate_info);
            ImplMemoryBlock const & mem_block = *manual_info.memory_block.as<ImplMemoryBlock const>();
            // TODO(pahrens): Add validation for memory requirements.
            [[maybe_unused]] VkResult const vk_create_image_result = vkCreateImage(this->vk_device, &vk_image_create_info, nullptr, &ret.vk_image);
            DAXA_DBG_ASSERT_TRUE_M(vk_create_image_result == VK_SUCCESS, "failed to create image");
            vmaBindImageMemory2(
                this->vma_allocator,
                mem_block.allocation,
                manual_info.offset,
                ret.vk_image,
                {});
        }

        VkImageViewType vk_image_view_type = {};
        if (image_info.array_layer_count > 1)
        {
            DAXA_DBG_ASSERT_TRUE_M(image_info.dimensions >= 1 && image_info.dimensions <= 2, "image dimensions must be 1 or 2 if making an image array");
            vk_image_view_type = static_cast<VkImageViewType>(image_info.dimensions + 3);
        }
        else
        {
            vk_image_view_type = static_cast<VkImageViewType>(image_info.dimensions - 1);
        }

        VkImageViewCreateInfo const vk_image_view_create_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .image = ret.vk_image,
            .viewType = vk_image_view_type,
            .format = *reinterpret_cast<VkFormat const *>(&image_info.format),
            .components = VkComponentMapping{
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = {
                .aspectMask = ret.aspect_flags,
                .baseMipLevel = 0,
                .levelCount = image_info.mip_level_count,
                .baseArrayLayer = 0,
                .layerCount = image_info.array_layer_count,
            },
        };

        [[maybe_unused]] VkResult const vk_create_image_view_result = vkCreateImageView(vk_device, &vk_image_view_create_info, nullptr, &ret.view_slot.vk_image_view);
        DAXA_DBG_ASSERT_TRUE_M(vk_create_image_view_result == VK_SUCCESS, "failed to create image view");

        if (this->impl_ctx.as<ImplInstance>()->enable_debug_names && !info.name.empty())
        {
            auto image_name = image_info.name;
            VkDebugUtilsObjectNameInfoEXT const swapchain_image_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_IMAGE,
                .objectHandle = reinterpret_cast<uint64_t>(ret.vk_image),
                .pObjectName = image_name.c_str(),
            };
            this->impl_ctx.as<ImplInstance>()->vkSetDebugUtilsObjectNameEXT(this->vk_device, &swapchain_image_name_info);

            auto image_view_name = image_info.name;
            VkDebugUtilsObjectNameInfoEXT const swapchain_image_view_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
                .objectHandle = reinterpret_cast<uint64_t>(ret.view_slot.vk_image_view),
                .pObjectName = image_view_name.c_str(),
            };
            this->impl_ctx.as<ImplInstance>()->vkSetDebugUtilsObjectNameEXT(this->vk_device, &swapchain_image_view_name_info);
        }

        write_descriptor_set_image(this->vk_device, this->gpu_shader_resource_table.vk_descriptor_set, ret.view_slot.vk_image_view, image_info.usage, id.index);

        image_slot_variant = ret;

        return ImageId{id};
    }

    auto ImplDevice::new_image_view(ImageViewInfo const & image_view_info) -> ImageViewId
    {
        auto [id, image_slot] = gpu_shader_resource_table.image_slots.new_slot();
        image_slot = {};
        ImplImageSlot const & parent_image_slot = slot(image_view_info.image);
        ImplImageViewSlot ret = {};
        ret.info = image_view_info;
        ImageMipArraySlice slice = this->validate_image_slice(image_view_info.slice, image_view_info.image);
        ret.info.slice = slice;
        VkImageViewCreateInfo const vk_image_view_create_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .image = parent_image_slot.vk_image,
            .viewType = static_cast<VkImageViewType>(image_view_info.type),
            .format = *reinterpret_cast<VkFormat const *>(&image_view_info.format),
            .components = VkComponentMapping{
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = make_subressource_range(slice, parent_image_slot.aspect_flags),
        };
        [[maybe_unused]] VkResult const result = vkCreateImageView(vk_device, &vk_image_view_create_info, nullptr, &ret.vk_image_view);
        DAXA_DBG_ASSERT_TRUE_M(result == VK_SUCCESS, "failed to create image view");
        if (this->impl_ctx.as<ImplInstance>()->enable_debug_names && !image_view_info.name.empty())
        {
            auto image_view_name = image_view_info.name;
            VkDebugUtilsObjectNameInfoEXT const name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
                .objectHandle = reinterpret_cast<uint64_t>(ret.vk_image_view),
                .pObjectName = image_view_name.c_str(),
            };
            this->impl_ctx.as<ImplInstance>()->vkSetDebugUtilsObjectNameEXT(this->vk_device, &name_info);
        }
        write_descriptor_set_image(this->vk_device, this->gpu_shader_resource_table.vk_descriptor_set, ret.vk_image_view, parent_image_slot.info.usage, id.index);
        image_slot.view_slot = ret;
        return ImageViewId{id};
    }

    auto ImplDevice::new_sampler(SamplerInfo const & sampler_info) -> SamplerId
    {
        auto [id, ret] = gpu_shader_resource_table.sampler_slots.new_slot();

        ret.info = sampler_info;
        ret.zombie = false;

        VkSamplerReductionModeCreateInfo vk_sampler_reduction_mode_create_info{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO,
            .pNext = nullptr,
            .reductionMode = static_cast<VkSamplerReductionMode>(sampler_info.reduction_mode),
        };

        DAXA_DBG_ASSERT_TRUE_M(sampler_info.mipmap_filter != Filter::CUBIC_IMG, "can not use cube addressing for mipmap filtering");

        VkSamplerCreateInfo const vk_sampler_create_info{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = reinterpret_cast<void *>(&vk_sampler_reduction_mode_create_info),
            .flags = {},
            .magFilter = static_cast<VkFilter>(sampler_info.magnification_filter),
            .minFilter = static_cast<VkFilter>(sampler_info.minification_filter),
            .mipmapMode = static_cast<VkSamplerMipmapMode>(sampler_info.mipmap_filter),
            .addressModeU = static_cast<VkSamplerAddressMode>(sampler_info.address_mode_u),
            .addressModeV = static_cast<VkSamplerAddressMode>(sampler_info.address_mode_v),
            .addressModeW = static_cast<VkSamplerAddressMode>(sampler_info.address_mode_w),
            .mipLodBias = sampler_info.mip_lod_bias,
            .anisotropyEnable = static_cast<VkBool32>(sampler_info.enable_anisotropy),
            .maxAnisotropy = sampler_info.max_anisotropy,
            .compareEnable = static_cast<VkBool32>(sampler_info.enable_compare),
            .compareOp = static_cast<VkCompareOp>(sampler_info.compare_op),
            .minLod = sampler_info.min_lod,
            .maxLod = sampler_info.max_lod,
            .borderColor = static_cast<VkBorderColor>(sampler_info.border_color),
            .unnormalizedCoordinates = static_cast<VkBool32>(sampler_info.enable_unnormalized_coordinates),
        };

        [[maybe_unused]] VkResult const result = vkCreateSampler(this->vk_device, &vk_sampler_create_info, nullptr, &ret.vk_sampler);
        DAXA_DBG_ASSERT_TRUE_M(result == VK_SUCCESS, "failed to create sampler");

        if (this->impl_ctx.as<ImplInstance>()->enable_debug_names && !info.name.empty())
        {
            auto sampler_name = info.name;
            VkDebugUtilsObjectNameInfoEXT const sampler_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_SAMPLER,
                .objectHandle = reinterpret_cast<uint64_t>(ret.vk_sampler),
                .pObjectName = sampler_name.c_str(),
            };
            this->impl_ctx.as<ImplInstance>()->vkSetDebugUtilsObjectNameEXT(this->vk_device, &sampler_name_info);
        }

        write_descriptor_set_sampler(this->vk_device, this->gpu_shader_resource_table.vk_descriptor_set, ret.vk_sampler, id.index);

        return SamplerId{id};
    }

    void ImplDevice::cleanup_buffer(BufferId id)
    {
        ImplBufferSlot & buffer_slot = this->gpu_shader_resource_table.buffer_slots.dereference_id(id);
        this->buffer_device_address_buffer_host_ptr[id.index] = 0;
        write_descriptor_set_buffer(this->vk_device, this->gpu_shader_resource_table.vk_descriptor_set, this->vk_null_buffer, 0, VK_WHOLE_SIZE, id.index);
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
    }

    void ImplDevice::cleanup_image(ImageId id)
    {
        ImplImageSlot & image_slot = gpu_shader_resource_table.image_slots.dereference_id(id);
        write_descriptor_set_image(this->vk_device, this->gpu_shader_resource_table.vk_descriptor_set, this->vk_null_image_view, image_slot.info.usage, id.index);
        vkDestroyImageView(vk_device, image_slot.view_slot.vk_image_view, nullptr);
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
    }

    void ImplDevice::cleanup_image_view(ImageViewId id)
    {
        DAXA_DBG_ASSERT_TRUE_M(gpu_shader_resource_table.image_slots.dereference_id(id).vk_image == VK_NULL_HANDLE, "can not destroy default image view of image");
        ImplImageViewSlot & image_slot = gpu_shader_resource_table.image_slots.dereference_id(id).view_slot;
        write_descriptor_set_image(this->vk_device, this->gpu_shader_resource_table.vk_descriptor_set, this->vk_null_image_view, ImageUsageFlagBits::SHADER_STORAGE | ImageUsageFlagBits::SHADER_SAMPLED, id.index);
        vkDestroyImageView(vk_device, image_slot.vk_image_view, nullptr);
        image_slot = {};
        gpu_shader_resource_table.image_slots.return_slot(id);
    }

    void ImplDevice::cleanup_sampler(SamplerId id)
    {
        ImplSamplerSlot & sampler_slot = this->gpu_shader_resource_table.sampler_slots.dereference_id(id);
        write_descriptor_set_sampler(this->vk_device, this->gpu_shader_resource_table.vk_descriptor_set, this->vk_null_sampler, id.index);
        vkDestroySampler(this->vk_device, sampler_slot.vk_sampler, nullptr);
        sampler_slot = {};
        gpu_shader_resource_table.sampler_slots.return_slot(id);
    }

    ImplDevice::~ImplDevice() // NOLINT(bugprone-exception-escape)
    {
        wait_idle();
        main_queue_collect_garbage();
        buffer_pool_pool.cleanup(this);
        vmaUnmapMemory(this->vma_allocator, this->buffer_device_address_buffer_allocation);
        vmaDestroyBuffer(this->vma_allocator, this->buffer_device_address_buffer, this->buffer_device_address_buffer_allocation);
        this->gpu_shader_resource_table.cleanup(this->vk_device);
        vmaDestroyImage(this->vma_allocator, this->vk_null_image, this->vk_null_image_vma_allocation);
        vmaDestroyBuffer(this->vma_allocator, this->vk_null_buffer, this->vk_null_buffer_vma_allocation);
        vmaDestroyAllocator(this->vma_allocator);
        vkDestroySampler(vk_device, this->vk_null_sampler, nullptr);
        vkDestroyImageView(this->vk_device, this->vk_null_image_view, nullptr);
        vkDestroySemaphore(this->vk_device, this->vk_main_queue_gpu_timeline_semaphore, nullptr);
        vkDestroyDevice(this->vk_device, nullptr);
    }

    void ImplDevice::zombify_buffer(BufferId id)
    {
        DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const lock{this->main_queue_zombies_mtx});
        u64 const main_queue_cpu_timeline_value = DAXA_ATOMIC_FETCH(this->main_queue_cpu_timeline);
        DAXA_DBG_ASSERT_TRUE_M(gpu_shader_resource_table.buffer_slots.dereference_id(id).zombie == false,
                               "detected free after free - buffer already is a zombie");
        gpu_shader_resource_table.buffer_slots.dereference_id(id).zombie = true;
        this->main_queue_buffer_zombies.push_front({main_queue_cpu_timeline_value, id});
    }

    void ImplDevice::zombify_image(ImageId id)
    {
        DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const lock{this->main_queue_zombies_mtx});
        u64 const main_queue_cpu_timeline_value = DAXA_ATOMIC_FETCH(this->main_queue_cpu_timeline);
        DAXA_DBG_ASSERT_TRUE_M(gpu_shader_resource_table.image_slots.dereference_id(id).zombie == false,
                               "detected free after free - image already is a zombie");
        gpu_shader_resource_table.image_slots.dereference_id(id).zombie = true;
        this->main_queue_image_zombies.push_front({main_queue_cpu_timeline_value, id});
    }

    void ImplDevice::zombify_image_view(ImageViewId id)
    {
        DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const lock{this->main_queue_zombies_mtx});
        u64 const main_queue_cpu_timeline_value = DAXA_ATOMIC_FETCH(this->main_queue_cpu_timeline);
        this->main_queue_image_view_zombies.push_front({main_queue_cpu_timeline_value, id});
    }

    void ImplDevice::zombify_sampler(SamplerId id)
    {
        DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const lock{this->main_queue_zombies_mtx});
        u64 const main_queue_cpu_timeline_value = DAXA_ATOMIC_FETCH(this->main_queue_cpu_timeline);
        DAXA_DBG_ASSERT_TRUE_M(gpu_shader_resource_table.sampler_slots.dereference_id(id).zombie == false,
                               "detected free after free - sampler already is a zombie");
        gpu_shader_resource_table.sampler_slots.dereference_id(id).zombie = true;
        this->main_queue_sampler_zombies.push_front({main_queue_cpu_timeline_value, id});
    }

    auto ImplDevice::slot(BufferId id) -> ImplBufferSlot &
    {
        return gpu_shader_resource_table.buffer_slots.dereference_id(id);
    }

    auto ImplDevice::slot(ImageId id) -> ImplImageSlot &
    {
        return gpu_shader_resource_table.image_slots.dereference_id(id);
    }

    auto ImplDevice::slot(ImageViewId id) -> ImplImageViewSlot &
    {
        return gpu_shader_resource_table.image_slots.dereference_id(id).view_slot;
    }

    auto ImplDevice::slot(SamplerId id) -> ImplSamplerSlot &
    {
        return gpu_shader_resource_table.sampler_slots.dereference_id(id);
    }

    auto ImplDevice::slot(BufferId id) const -> ImplBufferSlot const &
    {
        return gpu_shader_resource_table.buffer_slots.dereference_id(id);
    }

    auto ImplDevice::slot(ImageId id) const -> ImplImageSlot const &
    {
        return gpu_shader_resource_table.image_slots.dereference_id(id);
    }

    auto ImplDevice::slot(ImageViewId id) const -> ImplImageViewSlot const &
    {
        return gpu_shader_resource_table.image_slots.dereference_id(id).view_slot;
    }

    auto ImplDevice::slot(SamplerId id) const -> ImplSamplerSlot const &
    {
        return gpu_shader_resource_table.sampler_slots.dereference_id(id);
    }
} // namespace daxa
