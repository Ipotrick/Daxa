#include "impl_core.hpp"
#include "impl_device.hpp"
#include "impl_pipeline.hpp"
#include "impl_command_list.hpp"
#include "impl_swapchain.hpp"
#include "impl_semaphore.hpp"

namespace daxa
{
    Device::Device(std::shared_ptr<void> impl) : Handle(impl) {}

    Device::~Device()
    {
        auto & impl = *reinterpret_cast<ImplDevice *>(this->impl.get());

        wait_idle();
        collect_garbage();

        impl.submits_pool.clear();
        impl.zombie_binary_semaphores.clear();
        impl.zombie_command_lists.clear();
    }

    auto Device::info() const -> DeviceInfo const &
    {
        auto & impl = *reinterpret_cast<ImplDevice *>(this->impl.get());
        return impl.info;
    }

    void Device::wait_idle()
    {
        auto & impl = *reinterpret_cast<ImplDevice *>(this->impl.get());

        vkQueueWaitIdle(impl.vk_main_queue_handle);
        vkDeviceWaitIdle(impl.vk_device_handle);
    }

    void Device::submit_commands(CommandSubmitInfo const & submit_info)
    {
        auto & impl = *reinterpret_cast<ImplDevice *>(this->impl.get());

        std::unique_lock lock{impl.submit_mtx};

        ImplDevice::Submit submit = {};
        if (impl.submits_pool.empty())
        {
            VkFenceCreateInfo vk_fence_create_info{
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .pNext = nullptr,
            };

            vkCreateFence(impl.vk_device_handle, &vk_fence_create_info, nullptr, &submit.vk_fence_handle);

            if (DAXA_LOCK_WEAK(impl.impl_ctx)->enable_debug_names)
            {
                VkDebugUtilsObjectNameInfoEXT fence_name_info{
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_FENCE,
                    .objectHandle = reinterpret_cast<uint64_t>(submit.vk_fence_handle),
                    .pObjectName = "[[DAXA INTERNAL]] submit fence",
                };
                vkSetDebugUtilsObjectNameEXT(impl.vk_device_handle, &fence_name_info);
            }
        }
        else
        {
            submit = std::move(impl.submits_pool.back());
            impl.submits_pool.pop_back();
        }

        std::vector<VkCommandBuffer> submit_vk_command_buffer_handles = {};
        for (auto & command_list : submit_info.command_lists)
        {
            auto & impl_cmd_list = *reinterpret_cast<ImplCommandList *>(command_list.impl.get());
            DAXA_DBG_ASSERT_TRUE_M(impl_cmd_list.recording_complete, "all submitted command lists must be completed before submission");
            submit.binary_semaphores.push_back(std::static_pointer_cast<ImplBinarySemaphore>(command_list.impl));
            submit_vk_command_buffer_handles.push_back(impl_cmd_list.vk_cmd_buffer_handle);
        }

        std::vector<VkSemaphore> submit_vk_semaphore_handles = {};
        for (auto & binary_semaphore : submit_info.signal_binary_semaphores_on_completion)
        {
            auto & impl_binary_semaphore = *reinterpret_cast<ImplBinarySemaphore *>(binary_semaphore.impl.get());
            submit_vk_semaphore_handles.push_back(impl_binary_semaphore.vk_semaphore_handle);
        }

        VkPipelineStageFlags pipe_stage_flags;
        pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo vk_submit_info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = nullptr,
            .pWaitDstStageMask = &pipe_stage_flags,
            .commandBufferCount = static_cast<u32>(submit_vk_command_buffer_handles.size()),
            .pCommandBuffers = submit_vk_command_buffer_handles.data(),
            .signalSemaphoreCount = static_cast<u32>(submit_vk_semaphore_handles.size()),
            .pSignalSemaphores = submit_vk_semaphore_handles.data(),
        };
        vkQueueSubmit(impl.vk_main_queue_handle, 1, &vk_submit_info, submit.vk_fence_handle);

        impl.command_list_submits.push_back(std::move(submit));
    }

    void Device::present_frame(PresentInfo const & info)
    {
        auto & impl = *reinterpret_cast<ImplDevice *>(this->impl.get());
        auto & swapchain_impl = *reinterpret_cast<ImplSwapchain *>(info.swapchain.impl.get());
        auto & binary_semaphore_impl = *reinterpret_cast<ImplBinarySemaphore *>(info.wait_on_binary.impl.get());

        u32 image_index = 0;
        VkPresentInfoKHR present_info{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = static_cast<u32>(1),
            .pWaitSemaphores = &binary_semaphore_impl.vk_semaphore_handle,
            .swapchainCount = static_cast<u32>(1),
            .pSwapchains = &swapchain_impl.vk_swapchain_handle,
            .pImageIndices = &swapchain_impl.current_image_index,
        };

        vkQueuePresentKHR(impl.vk_main_queue_handle, &present_info);
    }

    void Device::collect_garbage()
    {
        auto & impl = *reinterpret_cast<ImplDevice *>(this->impl.get());

        std::unique_lock lock{impl.submit_mtx};

        for (auto iter = impl.command_list_submits.begin(); iter != impl.command_list_submits.end();)
        {
            ImplDevice::Submit & submit = *iter;
            VkResult result = vkGetFenceStatus(impl.vk_device_handle, submit.vk_fence_handle);

            DAXA_DBG_ASSERT_TRUE_M(result != VK_ERROR_DEVICE_LOST, "device lost");

            if (result == VK_SUCCESS)
            {
                // Return the fence into a pool, so they can be reused in future submits.
                impl.submits_pool.push_back(std::move(submit));

                iter = impl.command_list_submits.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }

    auto Device::create_swapchain(SwapchainInfo const & info) -> Swapchain
    {
        return Swapchain{std::make_shared<ImplSwapchain>(std::static_pointer_cast<ImplDevice>(this->impl), info)};
    }

    auto Device::create_pipeline_compiler(PipelineCompilerInfo const & info) -> PipelineCompiler
    {
        return PipelineCompiler{std::make_shared<ImplPipelineCompiler>(std::static_pointer_cast<ImplDevice>(this->impl), info)};
    }

    auto Device::create_command_list(CommandListInfo const & info) -> CommandList
    {
        auto & impl = *reinterpret_cast<ImplDevice *>(this->impl.get());

        std::shared_ptr<ImplCommandList> ret = {};

        {
            std::unique_lock lock{impl.zombie_command_lists_mtx};
            if (!impl.zombie_command_lists.empty())
            {
                ret = impl.zombie_command_lists.back();
                impl.zombie_command_lists.pop_back();
            }
        }

        if (!ret)
        {
            ret = std::make_shared<ImplCommandList>(std::static_pointer_cast<ImplDevice>(this->impl));
        }

        ret->initialize(info);

        return CommandList{ret};
    }

    auto Device::create_binary_semaphore(BinarySemaphoreInfo const & info) -> BinarySemaphore
    {
        auto & impl = *reinterpret_cast<ImplDevice *>(this->impl.get());

        std::shared_ptr<ImplBinarySemaphore> ret = {};

        {
            std::unique_lock lock{impl.zombie_binary_semaphores_mtx};
            if (!impl.zombie_binary_semaphores.empty())
            {
                ret = impl.zombie_binary_semaphores.back();
                impl.zombie_binary_semaphores.pop_back();
            }
        }

        if (!ret)
        {
            ret = std::make_shared<ImplBinarySemaphore>(std::static_pointer_cast<ImplDevice>(this->impl));
        }

        ret->initialize(info);

        return BinarySemaphore{ret};
    }

    static const VkPhysicalDeviceFeatures REQUIRED_PHYSICAL_DEVICE_FEATURES{
        .robustBufferAccess = VK_FALSE,
        .fullDrawIndexUint32 = VK_FALSE,
        .imageCubeArray = VK_FALSE,
        .independentBlend = VK_FALSE,
        .geometryShader = VK_FALSE,
        .tessellationShader = VK_FALSE,
        .sampleRateShading = VK_FALSE,
        .dualSrcBlend = VK_FALSE,
        .logicOp = VK_FALSE,
        .multiDrawIndirect = VK_FALSE,
        .drawIndirectFirstInstance = VK_FALSE,
        .depthClamp = VK_FALSE,
        .depthBiasClamp = VK_FALSE,
        .fillModeNonSolid = VK_FALSE,
        .depthBounds = VK_FALSE,
        .wideLines = VK_FALSE,
        .largePoints = VK_FALSE,
        .alphaToOne = VK_FALSE,
        .multiViewport = VK_FALSE,
        .samplerAnisotropy = VK_FALSE,
        .textureCompressionETC2 = VK_FALSE,
        .textureCompressionASTC_LDR = VK_FALSE,
        .textureCompressionBC = VK_FALSE,
        .occlusionQueryPrecise = VK_FALSE,
        .pipelineStatisticsQuery = VK_FALSE,
        .vertexPipelineStoresAndAtomics = VK_FALSE,
        .fragmentStoresAndAtomics = VK_FALSE,
        .shaderTessellationAndGeometryPointSize = VK_FALSE,
        .shaderImageGatherExtended = VK_FALSE,
        .shaderStorageImageExtendedFormats = VK_FALSE,
        .shaderStorageImageMultisample = VK_FALSE,
        .shaderStorageImageReadWithoutFormat = VK_FALSE,
        .shaderStorageImageWriteWithoutFormat = VK_FALSE,
        .shaderUniformBufferArrayDynamicIndexing = VK_FALSE,
        .shaderSampledImageArrayDynamicIndexing = VK_FALSE,
        .shaderStorageBufferArrayDynamicIndexing = VK_FALSE,
        .shaderStorageImageArrayDynamicIndexing = VK_FALSE,
        .shaderClipDistance = VK_FALSE,
        .shaderCullDistance = VK_FALSE,
        .shaderFloat64 = VK_FALSE,
        .shaderInt64 = VK_FALSE,
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

    static const VkPhysicalDeviceDescriptorIndexingFeatures REQUIRED_PHYSICAL_DEVICE_FEATURES_DESCRIPTOR_INDEXING{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
        .pNext = nullptr,
        .shaderInputAttachmentArrayDynamicIndexing = VK_FALSE, // no render passes
        .shaderUniformTexelBufferArrayDynamicIndexing = VK_TRUE,
        .shaderStorageTexelBufferArrayDynamicIndexing = VK_TRUE,
        .shaderUniformBufferArrayNonUniformIndexing = VK_TRUE,
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
        .shaderStorageBufferArrayNonUniformIndexing = VK_TRUE,
        .shaderStorageImageArrayNonUniformIndexing = VK_TRUE,
        .shaderInputAttachmentArrayNonUniformIndexing = VK_FALSE, // no render passes
        .shaderUniformTexelBufferArrayNonUniformIndexing = VK_TRUE,
        .shaderStorageTexelBufferArrayNonUniformIndexing = VK_TRUE,
        .descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE, // no uniform buffers
        .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
        .descriptorBindingStorageImageUpdateAfterBind = VK_TRUE,
        .descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE,
        .descriptorBindingUniformTexelBufferUpdateAfterBind = VK_FALSE, // no uniform buffers
        .descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE,
        .descriptorBindingUpdateUnusedWhilePending = VK_TRUE,
        .descriptorBindingPartiallyBound = VK_TRUE,
        .descriptorBindingVariableDescriptorCount = VK_TRUE,
        .runtimeDescriptorArray = VK_TRUE,
    };

    static const VkPhysicalDeviceDynamicRenderingFeatures REQUIRED_PHYSICAL_DEVICE_FEATURES_DYNAMIC_RENDERING{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
        .pNext = (void *)(&REQUIRED_PHYSICAL_DEVICE_FEATURES_DESCRIPTOR_INDEXING),
        .dynamicRendering = VK_TRUE,
    };

    static const VkPhysicalDeviceTimelineSemaphoreFeatures REQUIRED_PHYSICAL_DEVICE_FEATURES_TIMELINE_SEMAPHORE{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES,
        .pNext = (void *)(&REQUIRED_PHYSICAL_DEVICE_FEATURES_DYNAMIC_RENDERING),
        .timelineSemaphore = VK_TRUE,
    };

    static const VkPhysicalDeviceSynchronization2Features REQUIRED_PHYSICAL_DEVICE_FEATURES_SYNCHRONIZATION_2{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR,
        .pNext = (void *)(&REQUIRED_PHYSICAL_DEVICE_FEATURES_TIMELINE_SEMAPHORE),
        .synchronization2 = VK_TRUE,
    };

    static void * REQUIRED_DEVICE_FEATURE_P_CHAIN = (void *)(&REQUIRED_PHYSICAL_DEVICE_FEATURES_SYNCHRONIZATION_2);

    ImplDevice::ImplDevice(DeviceInfo const & a_info, DeviceVulkanInfo const & a_vk_info, std::shared_ptr<ImplContext> a_impl_ctx, VkPhysicalDevice a_physical_device)
        : info{a_info}, vk_info{a_vk_info}, impl_ctx{a_impl_ctx}, vk_physical_device{a_physical_device}
    {
        // SELECT QUEUE
        this->main_queue_family_index = std::numeric_limits<u32>::max();
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
            if ((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
            {
                this->main_queue_family_index = i;
                break;
            }
        }
        DAXA_DBG_ASSERT_TRUE_M(this->main_queue_family_index != std::numeric_limits<u32>::max(), "found no suitable queue family");

        f32 queue_priorities[1] = {0.0};
        VkDeviceQueueCreateInfo queue_ci{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = this->main_queue_family_index,
            .queueCount = 1,
            .pQueuePriorities = queue_priorities,
        };

        VkPhysicalDeviceFeatures2 physical_device_features_2{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = REQUIRED_DEVICE_FEATURE_P_CHAIN,
            .features = REQUIRED_PHYSICAL_DEVICE_FEATURES,
        };

        std::vector<char const *> extension_names;
        std::vector<char const *> enabled_layers;

        if (DAXA_LOCK_WEAK(impl_ctx)->info.enable_validation)
        {
            enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
        }
        extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        VkDeviceCreateInfo device_ci = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &physical_device_features_2,
            .queueCreateInfoCount = static_cast<u32>(1),
            .pQueueCreateInfos = &queue_ci,
            .enabledLayerCount = static_cast<u32>(enabled_layers.size()),
            .ppEnabledLayerNames = enabled_layers.data(),
            .enabledExtensionCount = static_cast<u32>(extension_names.size()),
            .ppEnabledExtensionNames = extension_names.data(),
            .pEnabledFeatures = nullptr,
        };
        vkCreateDevice(a_physical_device, &device_ci, nullptr, &this->vk_device_handle);
        volkLoadDevice(this->vk_device_handle);
        gpu_table.init(
            this->vk_info.limits.max_descriptor_set_storage_buffers,
            std::min(this->vk_info.limits.max_descriptor_set_sampled_images, this->vk_info.limits.max_descriptor_set_storage_images),
            this->vk_info.limits.max_descriptor_set_samplers,
            vk_device_handle);

        vkGetDeviceQueue(this->vk_device_handle, this->main_queue_family_index, 0, &this->vk_main_queue_handle);

        if (DAXA_LOCK_WEAK(this->impl_ctx)->enable_debug_names && this->info.debug_name.size() > 0)
        {
            VkDebugUtilsObjectNameInfoEXT device_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_DEVICE,
                .objectHandle = reinterpret_cast<uint64_t>(this->vk_device_handle),
                .pObjectName = this->info.debug_name.c_str(),
            };
            vkSetDebugUtilsObjectNameEXT(vk_device_handle, &device_name_info);

            VkDebugUtilsObjectNameInfoEXT device_main_queue_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_QUEUE,
                .objectHandle = reinterpret_cast<uint64_t>(this->vk_main_queue_handle),
                .pObjectName = this->info.debug_name.c_str(),
            };
            vkSetDebugUtilsObjectNameEXT(vk_device_handle, &device_main_queue_name_info);
        }
    }

    ImplDevice::~ImplDevice()
    {
        vkDestroyDevice(this->vk_device_handle, nullptr);
    }

    auto ImplDevice::new_buffer() -> BufferId
    {
        return BufferId{};
    }

    void ImplDevice::cleanup_buffer(BufferId id)
    {
    }

    auto ImplDevice::new_swapchain_image(VkImage swapchain_image, VkFormat format, u32 index, const std::string & debug_name) -> ImageId
    {
        auto [id, image_slot] = gpu_table.image_slots.new_slot();

        ImplImageSlot ret;
        ret.vk_image_handle = swapchain_image;
        VkImageViewCreateInfo view_ci{
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
        ret.info = ImageInfo{};
        vkCreateImageView(vk_device_handle, &view_ci, nullptr, &ret.vk_image_view_handle);
        image_slot = ret;

        if (DAXA_LOCK_WEAK(this->impl_ctx)->enable_debug_names && debug_name.size() > 0)
        {
            VkDebugUtilsObjectNameInfoEXT swapchain_image_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_IMAGE,
                .objectHandle = reinterpret_cast<uint64_t>(ret.vk_image_handle),
                .pObjectName = debug_name.c_str(),
            };
            vkSetDebugUtilsObjectNameEXT(vk_device_handle, &swapchain_image_name_info);

            VkDebugUtilsObjectNameInfoEXT swapchain_image_view_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
                .objectHandle = reinterpret_cast<uint64_t>(ret.vk_image_view_handle),
                .pObjectName = debug_name.c_str(),
            };
            vkSetDebugUtilsObjectNameEXT(vk_device_handle, &swapchain_image_view_name_info);
        }

        return ImageId{id};
    }

    auto ImplDevice::new_image() -> ImageId
    {
        return ImageId{};
    }

    void ImplDevice::cleanup_image(ImageId id)
    {
        ImplImageSlot & image_slot = std::get<ImplImageSlot>(gpu_table.image_slots.dereference_id(id));

        vkDestroyImageView(vk_device_handle, image_slot.vk_image_view_handle, nullptr);

        if (image_slot.swapchain_image_index == NOT_OWNED_BY_SWAPCHAIN)
        {
            vkDestroyImage(vk_device_handle, image_slot.vk_image_handle, nullptr);
        }

        gpu_table.image_slots.return_slot(id);
    }

    auto ImplDevice::new_image_view() -> ImageViewId
    {
        return ImageViewId{};
    }

    void ImplDevice::cleanup_image_view(ImageViewId id)
    {
    }

    auto ImplDevice::new_sampler() -> SamplerId
    {
        return SamplerId{};
    }

    void ImplDevice::cleanup_sampler(SamplerId id)
    {
    }

    auto ImplDevice::slot(BufferId id) -> ImplBufferSlot const &
    {
        return gpu_table.buffer_slots.dereference_id(id);
    }

    auto ImplDevice::slot(ImageId id) -> ImplImageSlot const &
    {
        return std::get<ImplImageSlot>(gpu_table.image_slots.dereference_id(id));
    }

    auto ImplDevice::slot(ImageViewId id) -> ImplImageViewSlot const &
    {
        return std::get<ImplImageViewSlot>(gpu_table.image_slots.dereference_id(id));
    }

    auto ImplDevice::slot(SamplerId id) -> ImplSamplerSlot const &
    {
        return gpu_table.sampler_slots.dereference_id(id);
    }

} // namespace daxa
