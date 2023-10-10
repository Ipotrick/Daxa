#include "impl_device.hpp"
#include "impl_pipeline.hpp"
#include "impl_swapchain.hpp"

// --- Begin API Functions ---

auto daxa_dvc_create_raster_pipeline(daxa_Device device, daxa_RasterPipelineInfo const * info, daxa_RasterPipeline * out_pipeline) -> daxa_Result
{
    daxa_ImplRasterPipeline ret = {};
    ret.device = device;
    ret.info = *reinterpret_cast<RasterPipelineInfo const *>(info);
    ret.info_name = {ret.info.name.data(), ret.info.name.size()};
    ret.info.name = {ret.info_name.data(), ret.info_name.size()};
    std::vector<VkShaderModule> vk_shader_modules{};
    std::vector<VkPipelineShaderStageCreateInfo> vk_pipeline_shader_stage_create_infos{};

    auto create_shader_module = [&](ShaderInfo const & shader_info, VkShaderStageFlagBits shader_stage) -> VkResult
    {
        VkShaderModule vk_shader_module = nullptr;
        VkShaderModuleCreateInfo const vk_shader_module_create_info{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .codeSize = static_cast<u32>(shader_info.byte_code_size * sizeof(u32)),
            .pCode = shader_info.byte_code,
        };
        auto result = vkCreateShaderModule(ret.device->vk_device, &vk_shader_module_create_info, nullptr, &vk_shader_module);
        if (result != VK_SUCCESS)
        {
            return result;
        }
        vk_shader_modules.push_back(vk_shader_module);
        VkPipelineShaderStageCreateInfo const vk_pipeline_shader_stage_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .stage = shader_stage,
            .module = vk_shader_module,
            .pName = ret.info_name.c_str(),
            .pSpecializationInfo = nullptr,
        };
        vk_pipeline_shader_stage_create_infos.push_back(vk_pipeline_shader_stage_create_info);
        device->inc_weak_refcnt();
        return result;
    };

#define _DAXA_DECL_TRY_CREATE_MODULE(name, NAME)                                                                                \
    if (ret.info.vertex_shader_info.has_value())                                                                                \
    {                                                                                                                           \
        auto result = create_shader_module(ret.info.name##_shader_info.value(), VkShaderStageFlagBits::VK_SHADER_STAGE_##NAME); \
        if (result != VK_SUCCESS)                                                                                               \
        {                                                                                                                       \
            for (auto module : vk_shader_modules)                                                                               \
            {                                                                                                                   \
                vkDestroyShaderModule(ret.device->vk_device, module, nullptr);                                                  \
            }                                                                                                                   \
            return std::bit_cast<daxa_Result>(result);                                                                          \
        }                                                                                                                       \
    }
    _DAXA_DECL_TRY_CREATE_MODULE(vertex, VERTEX_BIT)
    _DAXA_DECL_TRY_CREATE_MODULE(tesselation_control, TESSELLATION_CONTROL_BIT)
    _DAXA_DECL_TRY_CREATE_MODULE(tesselation_evaluation, TESSELLATION_EVALUATION_BIT)
    _DAXA_DECL_TRY_CREATE_MODULE(fragment, FRAGMENT_BIT)
    if ((ret.device->info.flags & DeviceFlagBits::MESH_SHADER_BIT) != DeviceFlagBits::NONE)
    {
        _DAXA_DECL_TRY_CREATE_MODULE(task, TASK_BIT_EXT)
        _DAXA_DECL_TRY_CREATE_MODULE(mesh, MESH_BIT_EXT)
    }

    ret.vk_pipeline_layout = ret.device->gpu_shader_resource_table.pipeline_layouts.at((ret.info.push_constant_size + 3) / 4);
    constexpr VkPipelineVertexInputStateCreateInfo vk_vertex_input_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = {},
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr,
    };
    VkPipelineInputAssemblyStateCreateInfo const vk_input_assembly_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = {},
        .topology = *reinterpret_cast<VkPrimitiveTopology const *>(&ret.info.raster.primitive_topology),
        .primitiveRestartEnable = static_cast<VkBool32>(ret.info.raster.primitive_restart_enable),
    };
    auto no_tess = TesselationInfo{};
    VkPipelineTessellationDomainOriginStateCreateInfo const vk_tesselation_domain_origin_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO,
        .pNext = nullptr,
        .domainOrigin = *reinterpret_cast<VkTessellationDomainOrigin const *>(&ret.info.tesselation.value_or(no_tess).origin),
    };
    VkPipelineTessellationStateCreateInfo const vk_tesselation_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
        .pNext = reinterpret_cast<void const *>(&vk_tesselation_domain_origin_state),
        .flags = {},
        .patchControlPoints = ret.info.tesselation.value_or(no_tess).control_points,
    };
    constexpr VkPipelineMultisampleStateCreateInfo vk_multisample_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = {},
        .rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = {},
        .alphaToCoverageEnable = {},
        .alphaToOneEnable = {},
    };
    VkPipelineRasterizationStateCreateInfo vk_raster_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = {},
        .depthClampEnable = ret.info.raster.depth_clamp_enable,
        .rasterizerDiscardEnable = static_cast<VkBool32>(ret.info.raster.rasterizer_discard_enable),
        .polygonMode = *reinterpret_cast<VkPolygonMode const *>(&ret.info.raster.polygon_mode),
        .cullMode = *reinterpret_cast<VkCullModeFlags const *>(&ret.info.raster.face_culling),
        .frontFace = *reinterpret_cast<VkFrontFace const *>(&ret.info.raster.front_face_winding),
        .depthBiasEnable = static_cast<VkBool32>(ret.info.raster.depth_bias_enable),
        .depthBiasConstantFactor = ret.info.raster.depth_bias_constant_factor,
        .depthBiasClamp = ret.info.raster.depth_bias_clamp,
        .depthBiasSlopeFactor = ret.info.raster.depth_bias_slope_factor,
        .lineWidth = ret.info.raster.line_width,
    };
    auto vk_conservative_raster_state = VkPipelineRasterizationConservativeStateCreateInfoEXT{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT,
        .pNext = nullptr,
        .flags = {},
        .conservativeRasterizationMode = VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT,
        .extraPrimitiveOverestimationSize = 0.0f,
    };
    if (
        ret.info.raster.conservative_raster_info.has_value() &&
        ((ret.device->info.flags & DeviceFlagBits::CONSERVATIVE_RASTERIZATION) != DeviceFlagBits::NONE))
    {
        // TODO(grundlett): Ask Patrick why this doesn't work
        // auto vk_instance = ret.device->instance->vk_instance;
        // PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        //     reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(vkGetInstanceProcAddr(vk_instance, "vkGetPhysicalDeviceProperties2KHR"));
        // DAXA_DBG_ASSERT_TRUE_M(vkGetPhysicalDeviceProperties2KHR != nullptr, "Failed to load this extension function function");
        // VkPhysicalDeviceProperties2KHR device_props2{};
        // VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservative_raster_props{};
        // conservative_raster_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT;
        // device_props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
        // device_props2.pNext = &conservative_raster_props;
        // vkGetPhysicalDeviceProperties2KHR(ret.device->vk_physical_device, &device_props2);
        auto const & conservative_raster_info = ret.info.raster.conservative_raster_info.value();
        vk_conservative_raster_state.conservativeRasterizationMode = static_cast<VkConservativeRasterizationModeEXT>(conservative_raster_info.mode);
        vk_conservative_raster_state.extraPrimitiveOverestimationSize = conservative_raster_info.size;
        vk_raster_state.pNext = &vk_conservative_raster_state;
    }
    DepthTestInfo no_depth = {};
    VkPipelineDepthStencilStateCreateInfo const vk_depth_stencil_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = {},
        .depthTestEnable = static_cast<VkBool32>(ret.info.depth_test.has_value()),
        .depthWriteEnable = static_cast<VkBool32>(ret.info.depth_test.value_or(no_depth).enable_depth_write),
        .depthCompareOp = static_cast<VkCompareOp>(ret.info.depth_test.value_or(no_depth).depth_test_compare_op),
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .front = {},
        .back = {},
        .minDepthBounds = ret.info.depth_test.value_or(no_depth).min_depth_bounds,
        .maxDepthBounds = ret.info.depth_test.value_or(no_depth).max_depth_bounds,
    };
    DAXA_DBG_ASSERT_TRUE_M(ret.info.color_attachments.size() < pipeline_manager_MAX_ATTACHMENTS, "too many color attachments, make pull request to bump max");
    std::array<VkPipelineColorBlendAttachmentState, pipeline_manager_MAX_ATTACHMENTS> vk_pipeline_color_blend_attachment_blend_states = {};
    auto no_blend = BlendInfo{};
    for (FixedListSizeT i = 0; i < ret.info.color_attachments.size(); ++i)
    {
        vk_pipeline_color_blend_attachment_blend_states.at(i) = VkPipelineColorBlendAttachmentState{
            .blendEnable = static_cast<VkBool32>(ret.info.color_attachments.at(i).blend.has_value()),
            .srcColorBlendFactor = static_cast<VkBlendFactor>(ret.info.color_attachments.at(i).blend.value_or(no_blend).src_color_blend_factor),
            .dstColorBlendFactor = static_cast<VkBlendFactor>(ret.info.color_attachments.at(i).blend.value_or(no_blend).dst_color_blend_factor),
            .colorBlendOp = static_cast<VkBlendOp>(ret.info.color_attachments.at(i).blend.value_or(no_blend).color_blend_op),
            .srcAlphaBlendFactor = static_cast<VkBlendFactor>(ret.info.color_attachments.at(i).blend.value_or(no_blend).src_alpha_blend_factor),
            .dstAlphaBlendFactor = static_cast<VkBlendFactor>(ret.info.color_attachments.at(i).blend.value_or(no_blend).dst_alpha_blend_factor),
            .alphaBlendOp = static_cast<VkBlendOp>(ret.info.color_attachments.at(i).blend.value_or(no_blend).alpha_blend_op),
            .colorWriteMask = std::bit_cast<VkColorComponentFlags>(ret.info.color_attachments.at(i).blend.value_or(no_blend).color_write_mask),
        };
    }
    std::array<VkFormat, pipeline_manager_MAX_ATTACHMENTS> vk_pipeline_color_attachment_formats = {};
    for (FixedListSizeT i = 0; i < ret.info.color_attachments.size(); ++i)
    {
        vk_pipeline_color_attachment_formats.at(i) = std::bit_cast<VkFormat>(ret.info.color_attachments.at(i).format);
    }
    VkPipelineColorBlendStateCreateInfo const vk_color_blend_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = {},
        .logicOpEnable = VK_FALSE,
        .logicOp = {},
        .attachmentCount = static_cast<u32>(ret.info.color_attachments.size()),
        .pAttachments = vk_pipeline_color_blend_attachment_blend_states.data(),
        .blendConstants = {1.0f, 1.0f, 1.0f, 1.0f},
    };
    constexpr VkViewport DEFAULT_VIEWPORT{.x = 0, .y = 0, .width = 1, .height = 1, .minDepth = 0, .maxDepth = 0};
    constexpr VkRect2D DEFAULT_SCISSOR{.offset = {0, 0}, .extent = {1, 1}};
    VkPipelineViewportStateCreateInfo const vk_viewport_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = {},
        .viewportCount = 1,
        .pViewports = &DEFAULT_VIEWPORT,
        .scissorCount = 1,
        .pScissors = &DEFAULT_SCISSOR,
    };
    auto dynamic_state = std::array{
        VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT,
        VkDynamicState::VK_DYNAMIC_STATE_SCISSOR,
        VkDynamicState::VK_DYNAMIC_STATE_DEPTH_BIAS,
    };
    VkPipelineDynamicStateCreateInfo const vk_dynamic_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = {},
        .dynamicStateCount = static_cast<u32>(dynamic_state.size()),
        .pDynamicStates = dynamic_state.data(),
    };
    VkPipelineRenderingCreateInfo vk_pipeline_rendering{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .pNext = nullptr,
        .viewMask = {},
        .colorAttachmentCount = static_cast<u32>(ret.info.color_attachments.size()),
        .pColorAttachmentFormats = vk_pipeline_color_attachment_formats.data(),
        .depthAttachmentFormat = static_cast<VkFormat>(ret.info.depth_test.value_or(no_depth).depth_attachment_format),
        .stencilAttachmentFormat = {},
    };
    VkGraphicsPipelineCreateInfo const vk_graphics_pipeline_create_info{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &vk_pipeline_rendering,
        .flags = {},
        .stageCount = static_cast<u32>(vk_pipeline_shader_stage_create_infos.size()),
        .pStages = vk_pipeline_shader_stage_create_infos.data(),
        .pVertexInputState = &vk_vertex_input_state,
        .pInputAssemblyState = &vk_input_assembly_state,
        .pTessellationState = &vk_tesselation_state,
        .pViewportState = &vk_viewport_state,
        .pRasterizationState = &vk_raster_state,
        .pMultisampleState = &vk_multisample_state,
        .pDepthStencilState = &vk_depth_stencil_state,
        .pColorBlendState = &vk_color_blend_state,
        .pDynamicState = &vk_dynamic_state,
        .layout = ret.vk_pipeline_layout,
        .renderPass = nullptr,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0,
    };
    auto result = vkCreateGraphicsPipelines(
        ret.device->vk_device,
        VK_NULL_HANDLE,
        1u,
        &vk_graphics_pipeline_create_info,
        nullptr,
        &ret.vk_pipeline);
    for (auto & vk_shader_module : vk_shader_modules)
    {
        vkDestroyShaderModule(ret.device->vk_device, vk_shader_module, nullptr);
    }
    if (result != VK_SUCCESS)
    {
        return std::bit_cast<daxa_Result>(result);
    }
    if ((ret.device->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE && !ret.info.name.empty())
    {
        VkDebugUtilsObjectNameInfoEXT const name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_PIPELINE,
            .objectHandle = std::bit_cast<u64>(ret.vk_pipeline),
            .pObjectName = ret.info_name.c_str(),
        };
        ret.device->vkSetDebugUtilsObjectNameEXT(ret.device->vk_device, &name_info);
    }
    *out_pipeline = new daxa_ImplRasterPipeline{};
    **out_pipeline = std::move(ret);
    daxa_raster_pipeline_inc_refcnt(*out_pipeline);
    device->inc_weak_refcnt();
    return DAXA_RESULT_SUCCESS;
}

auto daxa_raster_pipeline_info(daxa_RasterPipeline self) -> daxa_RasterPipelineInfo const *
{
    return reinterpret_cast<daxa_RasterPipelineInfo const *>(&self->info);
}

auto daxa_raster_pipeline_inc_refcnt(daxa_RasterPipeline self) -> u64
{
    return self->inc_refcnt();
}

auto daxa_raster_pipeline_dec_refcnt(daxa_RasterPipeline self) -> u64
{
    return self->dec_refcnt(
        &ImplPipeline::zero_ref_callback,
        self->device->instance);
}

auto daxa_dvc_create_compute_pipeline(daxa_Device device, daxa_ComputePipelineInfo const * info, daxa_ComputePipeline * out_pipeline) -> daxa_Result
{
    daxa_ImplComputePipeline ret = {};
    ret.device = device;
    ret.info = *reinterpret_cast<ComputePipelineInfo const *>(info);
    ret.info_name = {ret.info.name.data(), ret.info.name.size()};
    VkShaderModule vk_shader_module = {};
    VkShaderModuleCreateInfo const shader_module_ci{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = {},
        .codeSize = ret.info.shader_info.byte_code_size * static_cast<u32>(sizeof(u32)),
        .pCode = ret.info.shader_info.byte_code,
    };
    auto module_result = vkCreateShaderModule(ret.device->vk_device, &shader_module_ci, nullptr, &vk_shader_module);
    if (module_result != VK_SUCCESS)
    {
        return std::bit_cast<daxa_Result>(module_result);
    }
    ret.vk_pipeline_layout = ret.device->gpu_shader_resource_table.pipeline_layouts.at((ret.info.push_constant_size + 3) / 4);
    VkComputePipelineCreateInfo const vk_compute_pipeline_create_info{
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = {},
        .stage = VkPipelineShaderStageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .stage = VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT,
            .module = vk_shader_module,
            .pName = ret.info.shader_info.entry_point.data(),
            .pSpecializationInfo = nullptr,
        },
        .layout = ret.vk_pipeline_layout,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0,
    };
    auto pipeline_result = vkCreateComputePipelines(
        ret.device->vk_device,
        VK_NULL_HANDLE,
        1u,
        &vk_compute_pipeline_create_info,
        nullptr,
        &ret.vk_pipeline);
    vkDestroyShaderModule(ret.device->vk_device, vk_shader_module, nullptr);
    if (pipeline_result != VK_SUCCESS)
    {
        return std::bit_cast<daxa_Result>(pipeline_result);
    }
    if ((ret.device->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE && !ret.info_name.empty())
    {
        VkDebugUtilsObjectNameInfoEXT const name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_PIPELINE,
            .objectHandle = reinterpret_cast<uint64_t>(ret.vk_pipeline),
            .pObjectName = ret.info_name.c_str(),
        };
        ret.device->vkSetDebugUtilsObjectNameEXT(ret.device->vk_device, &name_info);
    }
    *out_pipeline = new daxa_ImplComputePipeline{};
    **out_pipeline = std::move(ret);
    daxa_compute_pipeline_inc_refcnt(*out_pipeline);
    device->inc_weak_refcnt();
    return DAXA_RESULT_SUCCESS;
}

auto daxa_compute_pipeline_info(daxa_ComputePipeline self) -> daxa_ComputePipelineInfo const *
{
    return reinterpret_cast<daxa_ComputePipelineInfo const *>(&self->info);
}

auto daxa_compute_pipeline_inc_refcnt(daxa_ComputePipeline self) -> u64
{
    return self->inc_refcnt();
}

auto daxa_compute_pipeline_dec_refcnt(daxa_ComputePipeline self) -> u64
{
    return self->dec_refcnt(
        &ImplPipeline::zero_ref_callback,
        self->device->instance);
}

// --- End API Functions ---

// --- Begin Internals ---

void ImplPipeline::zero_ref_callback(daxa_ImplHandle * handle)
{
    auto self = r_cast<ImplPipeline*>(handle);
    std::unique_lock const lock{self->device->main_queue_zombies_mtx};
    u64 const main_queue_cpu_timeline_value = self->device->main_queue_cpu_timeline.load(std::memory_order::relaxed);
    self->device->main_queue_pipeline_zombies.push_front({
        main_queue_cpu_timeline_value,
        PipelineZombie{
            .vk_pipeline = self->vk_pipeline,
        },
    });
    self->device->dec_weak_refcnt(
        daxa_ImplDevice::zero_ref_callback,
        self->device->instance
    );
    delete self;
}

// --- End Internals ---