#include "DearImGuiImpl.hpp"

namespace daxa {
    ImGuiRenderer::ImGuiRenderer(gpu::DeviceHandle device, gpu::QueueHandle queue) 
        : device{ device }
    {
        char const* const vertexGLSL = R"--(
            #version 450 core
            layout(location = 0) in vec2 aPos;
            layout(location = 1) in vec2 aUV;
            layout(location = 2) in vec4 aColor;
            layout(push_constant) uniform uPushConstant { vec2 uScale; vec2 uTranslate; } pc;
            out gl_PerVertex { vec4 gl_Position; };
            layout(location = 0) out struct { vec4 Color; vec2 UV; } Out;
            void main()
            {
                Out.Color = aColor;
                Out.UV = aUV;
                gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
            }
        )--";
        auto vertexShaderModule = device->tryCreateShderModuleFromGLSL(vertexGLSL, VK_SHADER_STAGE_VERTEX_BIT).value();

        char const* const fragmentGLSL = R"--(
            #version 450 core
            layout(location = 0) out vec4 fColor;
            layout(set=0, binding=0) uniform sampler2D sTexture;
            layout(location = 0) in struct { vec4 Color; vec2 UV; } In;
            vec4 srgb_to_linear(vec4 srgb) {
                vec3 color_srgb = srgb.rgb;
                vec3 selector = clamp(ceil(color_srgb - 0.04045), 0.0, 1.0); // 0 if under value, 1 if over
                vec3 under = color_srgb / 12.92;
                vec3 over = pow((color_srgb + 0.055) / 1.055, vec3(2.4));
                vec3 result = mix(under, over, selector);
                return vec4(result, srgb.a);
            }
            void main()
            {
                vec4 color = srgb_to_linear(In.Color);
                fColor = color * texture(sTexture, In.UV.st);
            }
        )--";
        auto fragmentShaderModule = device->tryCreateShderModuleFromGLSL(fragmentGLSL, VK_SHADER_STAGE_FRAGMENT_BIT).value();

        daxa::gpu::GraphicsPipelineBuilder pipelineBuilder;
        auto pipelineDescription = pipelineBuilder
            .addShaderStage(vertexShaderModule)
            .addShaderStage(fragmentShaderModule)
            .beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)
            .addVertexInputAttribute(VK_FORMAT_R32G32_SFLOAT)
            .addVertexInputAttribute(VK_FORMAT_R32G32_SFLOAT)
            .addVertexInputAttribute(VK_FORMAT_R8G8B8A8_UNORM)
            .addColorAttachment(VK_FORMAT_R8G8B8A8_SRGB, VkPipelineColorBlendAttachmentState{
                .blendEnable = VK_TRUE,
                .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                .alphaBlendOp = VK_BLEND_OP_ADD,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            });

        pipeline = device->createGraphicsPipeline(pipelineDescription);

        setAlloc = device->createBindingSetAllocator(pipeline->getSetDescription(0));

        recreatePerFrameData(4096, 4096);

        ImGuiIO& io = ImGui::GetIO();

        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        size_t upload_size = width * height * 4 * sizeof(char);

        fontSheet = device->createImage2d({
            .width = (u32)width,
            .height = (u32)height,
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .sampler = device->createSampler({})
        });

        auto cmdList = device->getEmptyCommandList();
        cmdList->begin();
        cmdList->copyHostToImageSynced({
            .src = pixels,
            .dst = fontSheet,
            .size = width * height * sizeof(u8) * 4,
            .dstFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        });
        cmdList->end();
        queue->submitBlocking({
            .commandLists = { cmdList }
        });

        fontSheetBinding = setAlloc->getSet();
        fontSheetBinding->bindImage(0, fontSheet, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        io.Fonts->SetTexID(fontSheet->getVkImage());
    }

    void ImGuiRenderer::recreatePerFrameData(size_t newMinSizeVertex, size_t newMinSizeIndices) {
        perFrameData.clear();
        for (int i = 0; i < 3; i++) {
            perFrameData.push_back(PerFrameData{
                .vertexBuffer = device->createBuffer({
                    .size = newMinSizeVertex,
                    .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    .memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU,
                }),
                .indexBuffer = device->createBuffer({
                    .size = newMinSizeIndices,
                    .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                    .memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU,
                }),
            });
        }
    }

    void ImGuiRenderer::recordCommands(ImDrawData* draw_data, daxa::gpu::CommandListHandle& cmdList, gpu::ImageHandle& target) {

        if (draw_data && draw_data->TotalIdxCount > 0) {
            //--        data upload        --//

            {
                auto frame = std::move(perFrameData.back());
                perFrameData.pop_back();
                perFrameData.push_front(std::move(frame));
            }

            auto& vertexBuffer  = perFrameData.front().vertexBuffer;
            auto& indexBuffer   = perFrameData.front().indexBuffer;

            if (draw_data->TotalVtxCount * sizeof(ImDrawVert) > vertexBuffer->getSize()) {
                auto newSize = draw_data->TotalVtxCount * sizeof(ImDrawVert) + 4096;
                vertexBuffer = device->createBuffer({
                    .size = newSize,
                    .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    .memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU,
                });
            }

            if (draw_data->TotalIdxCount * sizeof(ImDrawIdx) > indexBuffer->getSize()) {
                auto newSize = draw_data->TotalIdxCount * sizeof(ImDrawIdx) + 4096;
                indexBuffer = device->createBuffer({
                    .size = newSize,
                    .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                    .memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU,
                });
            }
            
            ImDrawVert* vtx_dst = (ImDrawVert*)vertexBuffer->mapMemory();
            ImDrawIdx* idx_dst = (ImDrawIdx*)indexBuffer->mapMemory();

            for (int n = 0; n < draw_data->CmdListsCount; n++)
            {
                const ImDrawList* cmd_list = draw_data->CmdLists[n];
                std::memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
                std::memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
                vtx_dst += cmd_list->VtxBuffer.Size;
                idx_dst += cmd_list->IdxBuffer.Size;
            }

            indexBuffer->unmapMemory();
            vertexBuffer->unmapMemory();

            //--  render command recording --//

            auto colorAttachments = std::array{
                gpu::RenderAttachmentInfo{
                    .image = target,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,
                }
            };
            cmdList->beginRendering({
                .colorAttachments = colorAttachments,
            });

            cmdList->bindPipeline(pipeline);

            cmdList->bindVertexBuffer(0, vertexBuffer);

            cmdList->bindIndexBuffer(indexBuffer, 0, VkIndexType::VK_INDEX_TYPE_UINT16);

            {
                float scale[2];
                scale[0] = 2.0f / draw_data->DisplaySize.x;
                scale[1] = 2.0f / draw_data->DisplaySize.y;
                float translate[2];
                translate[0] = -1.0f - draw_data->DisplayPos.x * scale[0];
                translate[1] = -1.0f - draw_data->DisplayPos.y * scale[1];
                cmdList->pushConstant(VK_SHADER_STAGE_VERTEX_BIT, scale);
                cmdList->pushConstant(VK_SHADER_STAGE_VERTEX_BIT, translate, sizeof(scale));
            }

            cmdList->bindSet(0, fontSheetBinding);

            ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
            ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

            int global_vtx_offset = 0;
            int global_idx_offset = 0;
            for (int n = 0; n < draw_data->CmdListsCount; n++) {
                const ImDrawList* cmd_list = draw_data->CmdLists[n];
                for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)  {
                    const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                    // Project scissor/clipping rectangles into framebuffer space
                    ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                    ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

                    // Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
                    if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
                    if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
                    if (clip_max.x > target->getVkExtent().width) { clip_max.x = (float)target->getVkExtent().width; }
                    if (clip_max.y > target->getVkExtent().height) { clip_max.y = (float)target->getVkExtent().height; }
                    if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                        continue;
                    
                    // Apply scissor/clipping rectangle
                    VkRect2D scissor;
                    scissor.offset.x = (int32_t)(clip_min.x);
                    scissor.offset.y = (int32_t)(clip_min.y);
                    scissor.extent.width = (uint32_t)(clip_max.x - clip_min.x);
                    scissor.extent.height = (uint32_t)(clip_max.y - clip_min.y);

                    cmdList->setScissor(scissor);

                    // Draw
                    cmdList->drawIndexed(pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
                }
                global_idx_offset += cmd_list->IdxBuffer.Size;
                global_vtx_offset += cmd_list->VtxBuffer.Size;
            }

            cmdList->endRendering();
        }
    }
}