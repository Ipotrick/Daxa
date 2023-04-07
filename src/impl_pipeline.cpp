#include "impl_pipeline.hpp"
#include "impl_swapchain.hpp"
#include "impl_device.hpp"

namespace daxa
{
    RasterPipeline::RasterPipeline(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}

    auto RasterPipeline::info() const -> RasterPipelineInfo const &
    {
        auto const & impl = *as<ImplRasterPipeline>();
        return impl.info;
    }

    ComputePipeline::ComputePipeline(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}

    auto ComputePipeline::info() const -> ComputePipelineInfo const &
    {
        auto const & impl = *as<ImplComputePipeline>();
        return impl.info;
    }

    ImplRasterPipeline::ImplRasterPipeline(ManagedWeakPtr a_impl_device, RasterPipelineInfo a_info)
        : ImplPipeline(std::move(a_impl_device)), info{std::move(a_info)}
    {
        std::vector<VkShaderModule> vk_shader_modules{};
        std::vector<VkPipelineShaderStageCreateInfo> vk_pipeline_shader_stage_create_infos{};

        auto create_shader_module = [&](ShaderInfo const & shader_info, VkShaderStageFlagBits shader_stage)
        {
            VkShaderModule vk_shader_module = nullptr;
            VkShaderModuleCreateInfo const vk_shader_module_create_info{
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .codeSize = static_cast<u32>(shader_info.byte_code.size() * sizeof(u32)),
                .pCode = shader_info.byte_code.data(),
            };
            vkCreateShaderModule(this->impl_device.as<ImplDevice>()->vk_device, &vk_shader_module_create_info, nullptr, &vk_shader_module);
            vk_shader_modules.push_back(vk_shader_module);
            VkPipelineShaderStageCreateInfo const vk_pipeline_shader_stage_create_info{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .stage = shader_stage,
                .module = vk_shader_module,
                .pName = shader_info.entry_point.has_value() ? shader_info.entry_point.value().c_str() : "main",
                .pSpecializationInfo = nullptr,
            };
            vk_pipeline_shader_stage_create_infos.push_back(vk_pipeline_shader_stage_create_info);
        };

        create_shader_module(this->info.vertex_shader_info, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
        if (this->info.tesselation_control_shader_info.has_value())
        {
            create_shader_module(this->info.tesselation_control_shader_info.value(), VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
        }
        if (this->info.tesselation_evaluation_shader_info.has_value())
        {
            create_shader_module(this->info.tesselation_evaluation_shader_info.value(), VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
        }
        if (this->info.fragment_shader_info.has_value())
        {
            create_shader_module(this->info.fragment_shader_info.value(), VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
        }

        this->vk_pipeline_layout = this->impl_device.as<ImplDevice>()->gpu_shader_resource_table.pipeline_layouts.at((this->info.push_constant_size + 3) / 4);
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
            .topology = *reinterpret_cast<VkPrimitiveTopology const *>(&info.raster.primitive_topology),
            .primitiveRestartEnable = static_cast<VkBool32>(info.raster.primitive_restart_enable),
        };
        VkPipelineTessellationDomainOriginStateCreateInfo const vk_tesselation_domain_origin_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO,
            .pNext = nullptr,
            .domainOrigin = *reinterpret_cast<VkTessellationDomainOrigin const *>(&info.tesselation.origin),
        };
        VkPipelineTessellationStateCreateInfo const vk_tesselation_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
            .pNext = reinterpret_cast<void const *>(&vk_tesselation_domain_origin_state),
            .flags = {},
            .patchControlPoints = info.tesselation.control_points,
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
            .depthClampEnable = {},
            .rasterizerDiscardEnable = info.fragment_shader_info.has_value() ? VK_FALSE : VK_TRUE,
            .polygonMode = *reinterpret_cast<VkPolygonMode const *>(&info.raster.polygon_mode),
            .cullMode = *reinterpret_cast<VkCullModeFlags const *>(&info.raster.face_culling),
            .frontFace = *reinterpret_cast<VkFrontFace const *>(&info.raster.front_face_winding),
            .depthBiasEnable = static_cast<VkBool32>(info.raster.depth_bias_enable),
            .depthBiasConstantFactor = info.raster.depth_bias_constant_factor,
            .depthBiasClamp = info.raster.depth_bias_clamp,
            .depthBiasSlopeFactor = info.raster.depth_bias_slope_factor,
            .lineWidth = info.raster.line_width,
        };
        auto vk_conservative_raster_state = VkPipelineRasterizationConservativeStateCreateInfoEXT{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags = {},
            .conservativeRasterizationMode = VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT,
            .extraPrimitiveOverestimationSize = 0.0f,
        };
        if (this->info.raster.conservative_raster_info.has_value())
        {
            DAXA_DBG_ASSERT_TRUE_M(this->impl_device.as<ImplDevice>()->info.enable_conservative_rasterization, "You must enable conservative rasterization in the device to use this feature");
            // TODO(grundlett): Ask Patrick why this doesn't work
            // auto vk_instance = this->impl_device.as<ImplDevice>()->impl_ctx.as<ImplContext>()->vk_instance;
            // PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
            //     reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(vkGetInstanceProcAddr(vk_instance, "vkGetPhysicalDeviceProperties2KHR"));
            // DAXA_DBG_ASSERT_TRUE_M(vkGetPhysicalDeviceProperties2KHR != nullptr, "Failed to load this extension function function");
            // VkPhysicalDeviceProperties2KHR device_props2{};
            // VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservative_raster_props{};
            // conservative_raster_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT;
            // device_props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
            // device_props2.pNext = &conservative_raster_props;
            // vkGetPhysicalDeviceProperties2KHR(this->impl_device.as<ImplDevice>()->vk_physical_device, &device_props2);
            auto const & conservative_raster_info = this->info.raster.conservative_raster_info.value();
            vk_conservative_raster_state.conservativeRasterizationMode = static_cast<VkConservativeRasterizationModeEXT>(conservative_raster_info.mode);
            vk_conservative_raster_state.extraPrimitiveOverestimationSize = conservative_raster_info.size;
            vk_raster_state.pNext = &vk_conservative_raster_state;
        }
        VkPipelineDepthStencilStateCreateInfo const vk_depth_stencil_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .depthTestEnable = static_cast<VkBool32>(this->info.depth_test.enable_depth_test),
            .depthWriteEnable = static_cast<VkBool32>(this->info.depth_test.enable_depth_write),
            .depthCompareOp = static_cast<VkCompareOp>(this->info.depth_test.depth_test_compare_op),
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = {},
            .back = {},
            .minDepthBounds = this->info.depth_test.min_depth_bounds,
            .maxDepthBounds = this->info.depth_test.max_depth_bounds,
        };
        DAXA_DBG_ASSERT_TRUE_M(this->info.color_attachments.size() < pipeline_manager_MAX_ATTACHMENTS, "too many color attachments, make pull request to bump max");
        std::array<VkPipelineColorBlendAttachmentState, pipeline_manager_MAX_ATTACHMENTS> vk_pipeline_color_blend_attachment_blend_states = {};
        for (usize i = 0; i < this->info.color_attachments.size(); ++i)
        {
            vk_pipeline_color_blend_attachment_blend_states.at(i) = *reinterpret_cast<VkPipelineColorBlendAttachmentState const *>(&this->info.color_attachments.at(i).blend);
        }
        std::array<VkFormat, pipeline_manager_MAX_ATTACHMENTS> vk_pipeline_color_attachment_formats = {};
        for (usize i = 0; i < this->info.color_attachments.size(); ++i)
        {
            vk_pipeline_color_attachment_formats.at(i) = *reinterpret_cast<VkFormat const *>(&this->info.color_attachments.at(i).format);
        }
        VkPipelineColorBlendStateCreateInfo const vk_color_blend_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .logicOpEnable = VK_FALSE,
            .logicOp = {},
            .attachmentCount = static_cast<u32>(this->info.color_attachments.size()),
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
            .colorAttachmentCount = static_cast<u32>(this->info.color_attachments.size()),
            .pColorAttachmentFormats = vk_pipeline_color_attachment_formats.data(),
            .depthAttachmentFormat = static_cast<VkFormat>(this->info.depth_test.depth_attachment_format),
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
            .layout = this->vk_pipeline_layout,
            .renderPass = nullptr,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = 0,
        };
        [[maybe_unused]] auto pipeline_result = vkCreateGraphicsPipelines(
            this->impl_device.as<ImplDevice>()->vk_device,
            VK_NULL_HANDLE,
            1u,
            &vk_graphics_pipeline_create_info,
            nullptr,
            &this->vk_pipeline);
        DAXA_DBG_ASSERT_TRUE_M(pipeline_result == VK_SUCCESS, "failed to create graphics pipeline");

        for (auto & vk_shader_module : vk_shader_modules)
        {
            vkDestroyShaderModule(this->impl_device.as<ImplDevice>()->vk_device, vk_shader_module, nullptr);
        }
        if (this->impl_device.as<ImplDevice>()->impl_ctx.as<ImplContext>()->enable_debug_names && !this->info.name.empty())
        {
            auto raster_pipeline_name = this->info.name;
            VkDebugUtilsObjectNameInfoEXT const name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_PIPELINE,
                .objectHandle = reinterpret_cast<uint64_t>(this->vk_pipeline),
                .pObjectName = raster_pipeline_name.c_str(),
            };
            this->impl_device.as<ImplDevice>()->vkSetDebugUtilsObjectNameEXT(this->impl_device.as<ImplDevice>()->vk_device, &name_info);
        }
    }

    ImplComputePipeline::ImplComputePipeline(ManagedWeakPtr a_impl_device, ComputePipelineInfo a_info)
        : ImplPipeline(std::move(a_impl_device)), info{std::move(a_info)}
    {
        VkShaderModule vk_shader_module = {};
        VkShaderModuleCreateInfo const shader_module_ci{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .codeSize = static_cast<u32>(this->info.shader_info.byte_code.size() * sizeof(u32)),
            .pCode = this->info.shader_info.byte_code.data(),
        };
        vkCreateShaderModule(this->impl_device.as<ImplDevice>()->vk_device, &shader_module_ci, nullptr, &vk_shader_module);
        this->vk_pipeline_layout = this->impl_device.as<ImplDevice>()->gpu_shader_resource_table.pipeline_layouts.at((this->info.push_constant_size + 3) / 4);
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
                .pName = this->info.shader_info.entry_point.has_value() ? this->info.shader_info.entry_point.value().c_str() : "main",
                .pSpecializationInfo = nullptr,
            },
            .layout = this->vk_pipeline_layout,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = 0,
        };
        [[maybe_unused]] auto pipeline_result = vkCreateComputePipelines(
            this->impl_device.as<ImplDevice>()->vk_device,
            VK_NULL_HANDLE,
            1u,
            &vk_compute_pipeline_create_info,
            nullptr,
            &this->vk_pipeline);
        DAXA_DBG_ASSERT_TRUE_M(pipeline_result == VK_SUCCESS, "failed to create compute pipeline");
        vkDestroyShaderModule(this->impl_device.as<ImplDevice>()->vk_device, vk_shader_module, nullptr);
        if (this->impl_device.as<ImplDevice>()->impl_ctx.as<ImplContext>()->enable_debug_names && !info.name.empty())
        {
            auto raster_pipeline_name = this->info.name;
            VkDebugUtilsObjectNameInfoEXT const name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_PIPELINE,
                .objectHandle = reinterpret_cast<uint64_t>(this->vk_pipeline),
                .pObjectName = raster_pipeline_name.c_str(),
            };
            this->impl_device.as<ImplDevice>()->vkSetDebugUtilsObjectNameEXT(this->impl_device.as<ImplDevice>()->vk_device, &name_info);
        }
    }

    ImplPipeline::ImplPipeline(ManagedWeakPtr a_impl_device)
        : impl_device{std::move(a_impl_device)}
    {
    }

    ImplPipeline::~ImplPipeline() // NOLINT(bugprone-exception-escape)
    {
        auto * device = this->impl_device.as<ImplDevice>();
        DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const lock{device->main_queue_zombies_mtx});
        u64 const main_queue_cpu_timeline_value = DAXA_ATOMIC_FETCH(device->main_queue_cpu_timeline);
        device->main_queue_pipeline_zombies.push_front({
            main_queue_cpu_timeline_value,
            PipelineZombie{
                .vk_pipeline = vk_pipeline,
            },
        });
    }
} // namespace daxa
