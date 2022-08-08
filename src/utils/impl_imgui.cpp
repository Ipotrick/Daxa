#if DAXA_BUILT_WITH_UTILS

#include "impl_imgui.hpp"

#include <cstring>

void set_imgui_style()
{
    ImVec4 * colors = ImGui::GetStyle().Colors;
    ImGuiStyle & style = ImGui::GetStyle();
    // clang-format off
    colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
    colors[ImGuiCol_Border]                 = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
    colors[ImGuiCol_Button]                 = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
    colors[ImGuiCol_Separator]              = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

    style.WindowPadding                     = ImVec2(8.00f, 8.00f);
    style.FramePadding                      = ImVec2(5.00f, 2.00f);
    style.CellPadding                       = ImVec2(6.00f, 6.00f);
    style.ItemSpacing                       = ImVec2(6.00f, 6.00f);
    style.ItemInnerSpacing                  = ImVec2(6.00f, 6.00f);
    style.TouchExtraPadding                 = ImVec2(0.00f, 0.00f);
    style.IndentSpacing                     = 25;
    style.ScrollbarSize                     = 15;
    style.GrabMinSize                       = 10;
    style.WindowBorderSize                  = 1;
    style.ChildBorderSize                   = 1;
    style.PopupBorderSize                   = 1;
    style.FrameBorderSize                   = 1;
    style.TabBorderSize                     = 1;
    style.WindowRounding                    = 7;
    style.ChildRounding                     = 4;
    style.FrameRounding                     = 3;
    style.PopupRounding                     = 4;
    style.ScrollbarRounding                 = 9;
    style.GrabRounding                      = 3;
    style.LogSliderDeadzone                 = 4;
    style.TabRounding                       = 4;
    // clang-format on
}

struct Push
{
    daxa::f32vec2 scale;
    daxa::f32vec2 translate;
    daxa::u32 vbuffer_offset;
    daxa::u32 ibuffer_offset;
    daxa::BufferId vbuffer_id;
    daxa::BufferId ibuffer_id;
    daxa::ImageViewId texture0_id;
    daxa::SamplerId sampler0_id;
};

char const * shaders_hlsl = R"--(
    #include "daxa/daxa.hlsl"
    struct Push
    {
        daxa::f32vec2 scale;
        daxa::f32vec2 translate;
        daxa::u32 vbuffer_offset;
        daxa::u32 ibuffer_offset;
        daxa::BufferId vbuffer_id;
        daxa::BufferId ibuffer_id;
        daxa::ImageViewId texture0_id;
        daxa::SamplerId sampler0_id;
    };
    [[vk::push_constant]] const Push p;
    struct Vertex
    {
        float2 pos;
        float2 uv;
        uint col;
    };
    struct VertexOutput {
        float4 pos : SV_POSITION;
        float4 col : COLOR0;
        float2 uv  : TEXCOORD0;
    };
    VertexOutput vs_main(uint invocation_index : SV_VERTEXID) {
        ByteAddressBuffer vbuffer = daxa::get_ByteAddressBuffer(p.vbuffer_id);
        Vertex input = vbuffer.Load<Vertex>(invocation_index * sizeof(Vertex));
        VertexOutput output;
        output.pos = float4(input.pos * p.scale + p.translate, 0, 1);
        output.col.r = ((input.col >> 0x00) & 0xff) * 1.0 / 255.0;
        output.col.g = ((input.col >> 0x08) & 0xff) * 1.0 / 255.0;
        output.col.b = ((input.col >> 0x10) & 0xff) * 1.0 / 255.0;
        output.col.a = ((input.col >> 0x18) & 0xff) * 1.0 / 255.0;
        output.uv  = input.uv;
        return output;
    }
    float4 srgb_to_linear(float4 srgb) {
        float3 color_srgb = srgb.rgb;
        float3 selector = clamp(ceil(color_srgb - 0.04045), 0.0, 1.0); // 0 if under value, 1 if over
        float3 under = color_srgb / 12.92;
        float3 over = pow((color_srgb + 0.055) / 1.055, float3(2.4, 2.4, 2.4));
        float3 result = lerp(under, over, selector);
        return float4(result, srgb.a);
    }
    float4 fs_main(VertexOutput input) : SV_Target {
        Texture2D<float4> texture0 = daxa::get_Texture2D<float4>(p.texture0_id);
        SamplerState sampler0 = daxa::get_sampler(p.sampler0_id);
        float4 col = srgb_to_linear(input.col) * texture0.Sample(sampler0, input.uv);
        return col;
    }
)--";

namespace daxa
{
    ImGuiRenderer::ImGuiRenderer(ImGuiRendererInfo const & info)
        : ManagedPtr{new ImplImGuiRenderer(info)}
    {
    }

    ImGuiRenderer::~ImGuiRenderer() {}

    void ImGuiRenderer::record_commands(ImDrawData * draw_data, CommandList & cmd_list, ImageId target_image, u32 size_x, u32 size_y)
    {
        auto & impl = *as<ImplImGuiRenderer>();
        impl.record_commands(draw_data, cmd_list, target_image, size_x, size_y);
    }

    void ImplImGuiRenderer::recreate_vbuffer(usize vbuffer_new_size)
    {
        vbuffer = info.device.create_buffer({
            .size = static_cast<u32>(vbuffer_new_size),
            .debug_name = "dear ImGui vertex buffer",
        });
        staging_vbuffer = info.device.create_buffer({
            .memory_flags = MemoryFlagBits::HOST_ACCESS_RANDOM,
            .size = static_cast<u32>(vbuffer_new_size),
            .debug_name = "dear ImGui staging vertex buffer",
        });
    }
    void ImplImGuiRenderer::recreate_ibuffer(usize ibuffer_new_size)
    {
        ibuffer = info.device.create_buffer({
            .size = static_cast<u32>(ibuffer_new_size),
            .debug_name = "dear ImGui index buffer",
        });
        staging_ibuffer = info.device.create_buffer({
            .memory_flags = MemoryFlagBits::HOST_ACCESS_RANDOM,
            .size = static_cast<u32>(ibuffer_new_size),
            .debug_name = "dear ImGui staging index buffer",
        });
    }

    void ImplImGuiRenderer::record_commands(ImDrawData * draw_data, CommandList & cmd_list, ImageId target_image, u32 size_x, u32 size_y)
    {
        if (draw_data && draw_data->TotalIdxCount > 0)
        {
            auto vbuffer_current_size = info.device.info_buffer(vbuffer).size;
            auto vbuffer_needed_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
            auto ibuffer_current_size = info.device.info_buffer(ibuffer).size;
            auto ibuffer_needed_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);

            if (vbuffer_needed_size > vbuffer_current_size)
            {
                auto vbuffer_new_size = vbuffer_needed_size + 4096;
                info.device.destroy_buffer(vbuffer);
                info.device.destroy_buffer(staging_vbuffer);
                recreate_vbuffer(vbuffer_new_size);
            }
            if (ibuffer_needed_size > ibuffer_current_size)
            {
                auto ibuffer_new_size = ibuffer_needed_size + 4096;
                info.device.destroy_buffer(ibuffer);
                info.device.destroy_buffer(staging_ibuffer);
                recreate_ibuffer(ibuffer_new_size);
            }

            {
                auto vtx_dst = info.device.map_memory_as<ImDrawVert>(staging_vbuffer);
                for (int n = 0; n < draw_data->CmdListsCount; n++)
                {
                    const ImDrawList * draws = draw_data->CmdLists[n];
                    std::memcpy(vtx_dst, draws->VtxBuffer.Data, draws->VtxBuffer.Size * sizeof(ImDrawVert));
                    vtx_dst += draws->VtxBuffer.Size;
                }
                info.device.unmap_memory(staging_vbuffer);
            }
            {
                auto idx_dst = info.device.map_memory_as<ImDrawIdx>(staging_ibuffer);
                for (int n = 0; n < draw_data->CmdListsCount; n++)
                {
                    const ImDrawList * draws = draw_data->CmdLists[n];
                    std::memcpy(idx_dst, draws->IdxBuffer.Data, draws->IdxBuffer.Size * sizeof(ImDrawIdx));
                    idx_dst += draws->IdxBuffer.Size;
                }
                info.device.unmap_memory(staging_ibuffer);
            }

            cmd_list.pipeline_barrier({
                .awaited_pipeline_access = daxa::AccessConsts::HOST_WRITE,
                .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
            });
            cmd_list.copy_buffer_to_buffer({
                .src_buffer = staging_vbuffer,
                .dst_buffer = vbuffer,
                .size = vbuffer_needed_size,
            });
            cmd_list.copy_buffer_to_buffer({
                .src_buffer = staging_ibuffer,
                .dst_buffer = ibuffer,
                .size = ibuffer_needed_size,
            });
            cmd_list.pipeline_barrier({
                .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
                .waiting_pipeline_access = daxa::AccessConsts::VERTEX_SHADER_READ,
            });

            cmd_list.begin_renderpass({
                .color_attachments = {{.image_view = target_image.default_view(), .load_op = AttachmentLoadOp::LOAD}},
                .render_area = {.x = 0, .y = 0, .width = size_x, .height = size_y},
            });
            cmd_list.set_pipeline(raster_pipeline);

            cmd_list.set_index_buffer(ibuffer, 0, sizeof(ImDrawIdx));

            auto push = Push{};
            push.scale = {2.0f / draw_data->DisplaySize.x, 2.0f / draw_data->DisplaySize.y};
            push.translate = {-1.0f - draw_data->DisplayPos.x * push.scale.x, -1.0f - draw_data->DisplayPos.y * push.scale.y};
            ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
            ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)
            int global_vtx_offset = 0, global_idx_offset = 0;
            push.vbuffer_id = vbuffer;
            push.ibuffer_id = ibuffer;
            push.sampler0_id = sampler;

            for (int n = 0; n < draw_data->CmdListsCount; n++)
            {
                const ImDrawList * draws = draw_data->CmdLists[n];
                usize lastTexId = 0;
                for (int cmd_i = 0; cmd_i < draws->CmdBuffer.Size; cmd_i++)
                {
                    const ImDrawCmd * pcmd = &draws->CmdBuffer[cmd_i];

                    // Project scissor/clipping rectangles into framebuffer space
                    ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                    ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

                    // Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
                    clip_min.x = std::clamp(clip_min.x, 0.0f, static_cast<f32>(size_x));
                    clip_min.y = std::clamp(clip_min.y, 0.0f, static_cast<f32>(size_y));
                    if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    {
                        continue;
                    }

                    // Apply scissor/clipping rectangle
                    Rect2D scissor;
                    scissor.x = static_cast<i32>(clip_min.x);
                    scissor.y = static_cast<i32>(clip_min.y);
                    scissor.width = static_cast<u32>(clip_max.x - clip_min.x);
                    scissor.height = static_cast<u32>(clip_max.y - clip_min.y);
                    cmd_list.set_scissor(scissor);

                    // Draw
                    push.texture0_id = *reinterpret_cast<daxa::ImageViewId const*>(&pcmd->TextureId);

                    push.vbuffer_offset = pcmd->VtxOffset + global_vtx_offset;
                    push.ibuffer_offset = pcmd->IdxOffset + global_idx_offset;

                    cmd_list.push_constant(push);
                    cmd_list.draw_indexed({
                        .index_count = pcmd->ElemCount,
                        .first_index = pcmd->IdxOffset + global_idx_offset,
                        .vertex_offset = pcmd->VtxOffset + global_vtx_offset,
                    });
                }
                global_idx_offset += draws->IdxBuffer.Size;
                global_vtx_offset += draws->VtxBuffer.Size;
            }

            cmd_list.end_renderpass();
        }
    }

    auto ImplImGuiRenderer::managed_cleanup() -> bool
    {
        return true;
    }

    ImplImGuiRenderer::ImplImGuiRenderer(ImGuiRendererInfo const & info)
        : info{info},
          // clang-format off
        raster_pipeline{this->info.pipeline_compiler.create_raster_pipeline({
            .vertex_shader_info = {.source = daxa::ShaderCode{.string = shaders_hlsl}, .entry_point = "vs_main"},
            .fragment_shader_info = {.source = daxa::ShaderCode{.string = shaders_hlsl}, .entry_point = "fs_main"},
            .color_attachments = {
                {
                    .format = info.format,
                    .blend = {
                        .blend_enable = true,
                        .src_color_blend_factor = BlendFactor::SRC_ALPHA,
                        .dst_color_blend_factor = BlendFactor::ONE_MINUS_SRC_ALPHA,
                        .src_alpha_blend_factor = BlendFactor::ONE,
                        .dst_alpha_blend_factor = BlendFactor::ONE_MINUS_SRC_ALPHA,
                    },
                },
            },
            .raster = {},
            .push_constant_size = sizeof(Push),
            .debug_name = "ImGui Draw Pipeline",
        }).value()}
    // clang-format on
    {
        set_imgui_style();
        recreate_vbuffer(4096);
        recreate_ibuffer(4096);
        sampler = this->info.device.create_sampler({.debug_name = "ImGui Texture Sampler"});

        ImGuiIO & io = ImGui::GetIO();
        u8 * pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        usize upload_size = width * height * 4 * sizeof(u8);
        font_sheet = this->info.device.create_image({
            .size = {static_cast<u32>(width), static_cast<u32>(height), 1},
            .usage = ImageUsageFlagBits::TRANSFER_DST | ImageUsageFlagBits::SHADER_READ_ONLY,
            .debug_name = "ImGui Font Sheet Image",
        });

        auto texture_staging_buffer = this->info.device.create_buffer({
            .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
            .size = static_cast<u32>(upload_size),
        });

        u8 * staging_buffer_data = this->info.device.map_memory_as<u8>(texture_staging_buffer);
        std::memcpy(staging_buffer_data, pixels, upload_size);
        this->info.device.unmap_memory(texture_staging_buffer);

        auto cmd_list = this->info.device.create_command_list({});

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::HOST_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
        });

        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::HOST_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::UNDEFINED,
            .after_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .image_id = font_sheet,
            .image_slice = {
                .base_mip_level = 0,
                .level_count = 1,
                .base_array_layer = 0,
                .layer_count = 1,
            },
        });

        cmd_list.copy_buffer_to_image({
            .buffer = texture_staging_buffer,
            .image = font_sheet,
            .image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .image_slice = {
                .mip_level = 0,
                .base_array_layer = 0,
                .layer_count = 1,
            },
            .image_offset = {0, 0, 0},
            .image_extent = {static_cast<u32>(width), static_cast<u32>(height), 1},
        });

        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
            .waiting_pipeline_access = daxa::AccessConsts::ALL_GRAPHICS_READ,
            .before_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .after_layout = daxa::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
            .image_id = font_sheet,
            .image_slice = {
                .base_mip_level = 0,
                .level_count = 1,
                .base_array_layer = 0,
                .layer_count = 1,
            },
        });
        cmd_list.complete();

        this->info.device.submit_commands({
            .command_lists = {cmd_list},
        });
        this->info.device.destroy_buffer(texture_staging_buffer);
        auto image_view = font_sheet.default_view();
        io.Fonts->SetTexID(*reinterpret_cast<ImTextureID*>(&image_view));
    }

    ImplImGuiRenderer::~ImplImGuiRenderer()
    {
        this->info.device.destroy_buffer(vbuffer);
        this->info.device.destroy_buffer(staging_vbuffer);
        this->info.device.destroy_buffer(ibuffer);
        this->info.device.destroy_buffer(staging_ibuffer);
        this->info.device.destroy_sampler(sampler);
        this->info.device.destroy_image(font_sheet);
    }
} // namespace daxa

#endif
