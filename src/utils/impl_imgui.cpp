#if DAXA_BUILT_WITH_UTILS

#include "impl_imgui.hpp"

#include <cstring>
#include <utility>

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

static constexpr auto imgui_vert_spv = std::array{
    // clang-format off
    0x07230203u, 0x00010300u, 0x000e0000u, 0x00000070u, 0x00000000u, 0x00020011u, 0x00000001u, 0x00020011u,
    0x000014b6u, 0x00020011u, 0x00000031u, 0x0008000au, 0x5f565053u, 0x5f545845u, 0x63736564u, 0x74706972u,
    0x695f726fu, 0x7865646eu, 0x00676e69u, 0x0003000eu, 0x00000000u, 0x00000001u, 0x0009000fu, 0x00000000u,
    0x00000001u, 0x6d5f7376u, 0x006e6961u, 0x00000002u, 0x00000003u, 0x00000004u, 0x00000005u, 0x00030003u,
    0x00000005u, 0x00000294u, 0x00080005u, 0x00000006u, 0x65707974u, 0x7479422eu, 0x64644165u, 0x73736572u,
    0x66667542u, 0x00007265u, 0x00080005u, 0x00000007u, 0x65747942u, 0x72646441u, 0x42737365u, 0x65666675u,
    0x65695672u, 0x00000077u, 0x00080005u, 0x00000008u, 0x65707974u, 0x7375502eu, 0x6e6f4368u, 0x6e617473u,
    0x75502e74u, 0x00006873u, 0x00050006u, 0x00000008u, 0x00000000u, 0x6c616373u, 0x00000065u, 0x00060006u,
    0x00000008u, 0x00000001u, 0x6e617274u, 0x74616c73u, 0x00000065u, 0x00070006u, 0x00000008u, 0x00000002u,
    0x66756276u, 0x5f726566u, 0x7366666fu, 0x00007465u, 0x00070006u, 0x00000008u, 0x00000003u, 0x66756269u,
    0x5f726566u, 0x7366666fu, 0x00007465u, 0x00060006u, 0x00000008u, 0x00000004u, 0x66756276u, 0x5f726566u,
    0x00006469u, 0x00060006u, 0x00000008u, 0x00000005u, 0x66756269u, 0x5f726566u, 0x00006469u, 0x00060006u,
    0x00000008u, 0x00000006u, 0x74786574u, 0x30657275u, 0x0064695fu, 0x00060006u, 0x00000008u, 0x00000007u,
    0x706d6173u, 0x3072656cu, 0x0064695fu, 0x00050005u, 0x00000009u, 0x66667542u, 0x64497265u, 0x00000000u,
    0x00070006u, 0x00000009u, 0x00000000u, 0x66667562u, 0x695f7265u, 0x61765f64u, 0x0065756cu, 0x00050005u,
    0x0000000au, 0x67616d49u, 0x65695665u, 0x00644977u, 0x00080006u, 0x0000000au, 0x00000000u, 0x67616d69u,
    0x69765f65u, 0x695f7765u, 0x61765f64u, 0x0065756cu, 0x00050005u, 0x0000000bu, 0x706d6153u, 0x4972656cu,
    0x00000064u, 0x00080006u, 0x0000000bu, 0x00000000u, 0x706d6173u, 0x5f72656cu, 0x765f6469u, 0x65756c61u,
    0x00000000u, 0x00030005u, 0x0000000cu, 0x00000070u, 0x00060005u, 0x00000004u, 0x2e74756fu, 0x2e726176u,
    0x4f4c4f43u, 0x00003052u, 0x00070005u, 0x00000005u, 0x2e74756fu, 0x2e726176u, 0x43584554u, 0x44524f4fu,
    0x00000030u, 0x00040005u, 0x00000001u, 0x6d5f7376u, 0x006e6961u, 0x00040047u, 0x00000002u, 0x0000000bu,
    0x0000002au, 0x00040047u, 0x00000003u, 0x0000000bu, 0x00000000u, 0x00040047u, 0x00000004u, 0x0000001eu,
    0x00000000u, 0x00040047u, 0x00000005u, 0x0000001eu, 0x00000001u, 0x00040047u, 0x00000007u, 0x00000022u,
    0x00000000u, 0x00040047u, 0x00000007u, 0x00000021u, 0x00000000u, 0x00040047u, 0x0000000du, 0x00000006u,
    0x00000004u, 0x00050048u, 0x00000006u, 0x00000000u, 0x00000023u, 0x00000000u, 0x00040048u, 0x00000006u,
    0x00000000u, 0x00000018u, 0x00030047u, 0x00000006u, 0x00000003u, 0x00050048u, 0x00000009u, 0x00000000u,
    0x00000023u, 0x00000000u, 0x00050048u, 0x0000000au, 0x00000000u, 0x00000023u, 0x00000000u, 0x00050048u,
    0x0000000bu, 0x00000000u, 0x00000023u, 0x00000000u, 0x00050048u, 0x00000008u, 0x00000000u, 0x00000023u,
    0x00000000u, 0x00050048u, 0x00000008u, 0x00000001u, 0x00000023u, 0x00000008u, 0x00050048u, 0x00000008u,
    0x00000002u, 0x00000023u, 0x00000010u, 0x00050048u, 0x00000008u, 0x00000003u, 0x00000023u, 0x00000014u,
    0x00050048u, 0x00000008u, 0x00000004u, 0x00000023u, 0x00000018u, 0x00050048u, 0x00000008u, 0x00000005u,
    0x00000023u, 0x0000001cu, 0x00050048u, 0x00000008u, 0x00000006u, 0x00000023u, 0x00000020u, 0x00050048u,
    0x00000008u, 0x00000007u, 0x00000023u, 0x00000024u, 0x00030047u, 0x00000008u, 0x00000002u, 0x00040015u,
    0x0000000eu, 0x00000020u, 0x00000001u, 0x0004002bu, 0x0000000eu, 0x0000000fu, 0x00000004u, 0x00040015u,
    0x00000010u, 0x00000020u, 0x00000000u, 0x0004002bu, 0x00000010u, 0x00000011u, 0x00000014u, 0x0004002bu,
    0x00000010u, 0x00000012u, 0x00000002u, 0x0004002bu, 0x00000010u, 0x00000013u, 0x00000000u, 0x0004002bu,
    0x00000010u, 0x00000014u, 0x00000001u, 0x0004002bu, 0x00000010u, 0x00000015u, 0x00000004u, 0x0004002bu,
    0x0000000eu, 0x00000016u, 0x00000000u, 0x0004002bu, 0x0000000eu, 0x00000017u, 0x00000001u, 0x00030016u,
    0x00000018u, 0x00000020u, 0x0004002bu, 0x00000018u, 0x00000019u, 0x00000000u, 0x0004002bu, 0x00000018u,
    0x0000001au, 0x3f800000u, 0x0004002bu, 0x0000000eu, 0x0000001bu, 0x00000002u, 0x0004002bu, 0x00000010u,
    0x0000001cu, 0x000000ffu, 0x0004002bu, 0x00000010u, 0x0000001du, 0x00000008u, 0x0004002bu, 0x00000010u,
    0x0000001eu, 0x00000010u, 0x0004002bu, 0x00000010u, 0x0000001fu, 0x00000018u, 0x0004002bu, 0x0000000eu,
    0x00000020u, 0x00000003u, 0x0004002bu, 0x00000010u, 0x00000021u, 0x00ffffffu, 0x0003001du, 0x0000000du,
    0x00000010u, 0x0003001eu, 0x00000006u, 0x0000000du, 0x0003001du, 0x00000022u, 0x00000006u, 0x00040020u,
    0x00000023u, 0x00000002u, 0x00000022u, 0x00040017u, 0x00000024u, 0x00000018u, 0x00000002u, 0x0003001eu,
    0x00000009u, 0x00000010u, 0x0003001eu, 0x0000000au, 0x00000010u, 0x0003001eu, 0x0000000bu, 0x00000010u,
    0x000a001eu, 0x00000008u, 0x00000024u, 0x00000024u, 0x00000010u, 0x00000010u, 0x00000009u, 0x00000009u,
    0x0000000au, 0x0000000bu, 0x00040020u, 0x00000025u, 0x00000009u, 0x00000008u, 0x00040020u, 0x00000026u,
    0x00000001u, 0x00000010u, 0x00040017u, 0x00000027u, 0x00000018u, 0x00000004u, 0x00040020u, 0x00000028u,
    0x00000003u, 0x00000027u, 0x00040020u, 0x00000029u, 0x00000003u, 0x00000024u, 0x00020013u, 0x0000002au,
    0x00030021u, 0x0000002bu, 0x0000002au, 0x00040020u, 0x0000002cu, 0x00000002u, 0x00000006u, 0x00040020u,
    0x0000002du, 0x00000009u, 0x00000009u, 0x00040020u, 0x0000002eu, 0x00000002u, 0x00000010u, 0x00040020u,
    0x0000002fu, 0x00000009u, 0x00000024u, 0x00040020u, 0x00000030u, 0x00000007u, 0x00000027u, 0x00040020u,
    0x00000031u, 0x00000007u, 0x00000018u, 0x0004003bu, 0x00000023u, 0x00000007u, 0x00000002u, 0x0004003bu,
    0x00000025u, 0x0000000cu, 0x00000009u, 0x0004003bu, 0x00000026u, 0x00000002u, 0x00000001u, 0x0004003bu,
    0x00000028u, 0x00000003u, 0x00000003u, 0x0004003bu, 0x00000028u, 0x00000004u, 0x00000003u, 0x0004003bu,
    0x00000029u, 0x00000005u, 0x00000003u, 0x0004002bu, 0x00000010u, 0x00000032u, 0x00000003u, 0x0004002bu,
    0x00000018u, 0x00000033u, 0x3b808081u, 0x00050036u, 0x0000002au, 0x00000001u, 0x00000000u, 0x0000002bu,
    0x000200f8u, 0x00000034u, 0x0004003bu, 0x00000030u, 0x00000035u, 0x00000007u, 0x0004003du, 0x00000010u,
    0x00000036u, 0x00000002u, 0x00050041u, 0x0000002du, 0x00000037u, 0x0000000cu, 0x0000000fu, 0x0004003du,
    0x00000009u, 0x00000038u, 0x00000037u, 0x00050051u, 0x00000010u, 0x00000039u, 0x00000038u, 0x00000000u,
    0x000500c7u, 0x00000010u, 0x0000003au, 0x00000021u, 0x00000039u, 0x00050041u, 0x0000002cu, 0x0000003bu,
    0x00000007u, 0x0000003au, 0x00050084u, 0x00000010u, 0x0000003cu, 0x00000036u, 0x00000011u, 0x000500c2u,
    0x00000010u, 0x0000003du, 0x0000003cu, 0x00000012u, 0x00060041u, 0x0000002eu, 0x0000003eu, 0x0000003bu,
    0x00000013u, 0x0000003du, 0x0004003du, 0x00000010u, 0x0000003fu, 0x0000003eu, 0x0004007cu, 0x00000018u,
    0x00000040u, 0x0000003fu, 0x00050080u, 0x00000010u, 0x00000041u, 0x0000003du, 0x00000014u, 0x00060041u,
    0x0000002eu, 0x00000042u, 0x0000003bu, 0x00000013u, 0x00000041u, 0x0004003du, 0x00000010u, 0x00000043u,
    0x00000042u, 0x0004007cu, 0x00000018u, 0x00000044u, 0x00000043u, 0x00050050u, 0x00000024u, 0x00000045u,
    0x00000040u, 0x00000044u, 0x00050080u, 0x00000010u, 0x00000046u, 0x0000003du, 0x00000012u, 0x00060041u,
    0x0000002eu, 0x00000047u, 0x0000003bu, 0x00000013u, 0x00000046u, 0x0004003du, 0x00000010u, 0x00000048u,
    0x00000047u, 0x0004007cu, 0x00000018u, 0x00000049u, 0x00000048u, 0x00050080u, 0x00000010u, 0x0000004au,
    0x0000003du, 0x00000032u, 0x00060041u, 0x0000002eu, 0x0000004bu, 0x0000003bu, 0x00000013u, 0x0000004au,
    0x0004003du, 0x00000010u, 0x0000004cu, 0x0000004bu, 0x0004007cu, 0x00000018u, 0x0000004du, 0x0000004cu,
    0x00050050u, 0x00000024u, 0x0000004eu, 0x00000049u, 0x0000004du, 0x00050080u, 0x00000010u, 0x0000004fu,
    0x0000003du, 0x00000015u, 0x00060041u, 0x0000002eu, 0x00000050u, 0x0000003bu, 0x00000013u, 0x0000004fu,
    0x0004003du, 0x00000010u, 0x00000051u, 0x00000050u, 0x00050041u, 0x0000002fu, 0x00000052u, 0x0000000cu,
    0x00000016u, 0x0004003du, 0x00000024u, 0x00000053u, 0x00000052u, 0x00050085u, 0x00000024u, 0x00000054u,
    0x00000045u, 0x00000053u, 0x00050041u, 0x0000002fu, 0x00000055u, 0x0000000cu, 0x00000017u, 0x0004003du,
    0x00000024u, 0x00000056u, 0x00000055u, 0x00050081u, 0x00000024u, 0x00000057u, 0x00000054u, 0x00000056u,
    0x00050051u, 0x00000018u, 0x00000058u, 0x00000057u, 0x00000000u, 0x00050051u, 0x00000018u, 0x00000059u,
    0x00000057u, 0x00000001u, 0x00070050u, 0x00000027u, 0x0000005au, 0x00000058u, 0x00000059u, 0x00000019u,
    0x0000001au, 0x000500c2u, 0x00000010u, 0x0000005bu, 0x00000051u, 0x00000013u, 0x000500c7u, 0x00000010u,
    0x0000005cu, 0x0000005bu, 0x0000001cu, 0x00040070u, 0x00000018u, 0x0000005du, 0x0000005cu, 0x00050085u,
    0x00000018u, 0x0000005eu, 0x0000005du, 0x00000033u, 0x00050041u, 0x00000031u, 0x0000005fu, 0x00000035u,
    0x00000016u, 0x0003003eu, 0x0000005fu, 0x0000005eu, 0x000500c2u, 0x00000010u, 0x00000060u, 0x00000051u,
    0x0000001du, 0x000500c7u, 0x00000010u, 0x00000061u, 0x00000060u, 0x0000001cu, 0x00040070u, 0x00000018u,
    0x00000062u, 0x00000061u, 0x00050085u, 0x00000018u, 0x00000063u, 0x00000062u, 0x00000033u, 0x00050041u,
    0x00000031u, 0x00000064u, 0x00000035u, 0x00000017u, 0x0003003eu, 0x00000064u, 0x00000063u, 0x000500c2u,
    0x00000010u, 0x00000065u, 0x00000051u, 0x0000001eu, 0x000500c7u, 0x00000010u, 0x00000066u, 0x00000065u,
    0x0000001cu, 0x00040070u, 0x00000018u, 0x00000067u, 0x00000066u, 0x00050085u, 0x00000018u, 0x00000068u,
    0x00000067u, 0x00000033u, 0x00050041u, 0x00000031u, 0x00000069u, 0x00000035u, 0x0000001bu, 0x0003003eu,
    0x00000069u, 0x00000068u, 0x000500c2u, 0x00000010u, 0x0000006au, 0x00000051u, 0x0000001fu, 0x000500c7u,
    0x00000010u, 0x0000006bu, 0x0000006au, 0x0000001cu, 0x00040070u, 0x00000018u, 0x0000006cu, 0x0000006bu,
    0x00050085u, 0x00000018u, 0x0000006du, 0x0000006cu, 0x00000033u, 0x00050041u, 0x00000031u, 0x0000006eu,
    0x00000035u, 0x00000020u, 0x0003003eu, 0x0000006eu, 0x0000006du, 0x0004003du, 0x00000027u, 0x0000006fu,
    0x00000035u, 0x0003003eu, 0x00000003u, 0x0000005au, 0x0003003eu, 0x00000004u, 0x0000006fu, 0x0003003eu,
    0x00000005u, 0x0000004eu, 0x000100fdu, 0x00010038u,
    // clang-format on
};

static constexpr auto imgui_frag_spv = std::array{
    // clang-format off
    0x07230203u, 0x00010300u, 0x000e0000u, 0x0000005au, 0x00000000u, 0x00020011u, 0x00000001u, 0x00020011u,
    0x000014b6u, 0x00020011u, 0x00000031u, 0x0008000au, 0x5f565053u, 0x5f545845u, 0x63736564u, 0x74706972u,
    0x695f726fu, 0x7865646eu, 0x00676e69u, 0x0006000bu, 0x00000001u, 0x4c534c47u, 0x6474732eu, 0x3035342eu,
    0x00000000u, 0x0003000eu, 0x00000000u, 0x00000001u, 0x0008000fu, 0x00000004u, 0x00000002u, 0x6d5f7366u,
    0x006e6961u, 0x00000003u, 0x00000004u, 0x00000005u, 0x00030010u, 0x00000002u, 0x00000007u, 0x00030003u,
    0x00000005u, 0x00000294u, 0x00060005u, 0x00000006u, 0x65707974u, 0x6d61732eu, 0x72656c70u, 0x00000000u,
    0x00070005u, 0x00000007u, 0x706d6153u, 0x5372656cu, 0x65746174u, 0x77656956u, 0x00000000u, 0x00060005u,
    0x00000008u, 0x65707974u, 0x2e64322eu, 0x67616d69u, 0x00000065u, 0x00070005u, 0x00000009u, 0x74786554u,
    0x32657275u, 0x65695644u, 0x6f6c6677u, 0x00347461u, 0x00080005u, 0x0000000au, 0x65707974u, 0x7375502eu,
    0x6e6f4368u, 0x6e617473u, 0x75502e74u, 0x00006873u, 0x00050006u, 0x0000000au, 0x00000000u, 0x6c616373u,
    0x00000065u, 0x00060006u, 0x0000000au, 0x00000001u, 0x6e617274u, 0x74616c73u, 0x00000065u, 0x00070006u,
    0x0000000au, 0x00000002u, 0x66756276u, 0x5f726566u, 0x7366666fu, 0x00007465u, 0x00070006u, 0x0000000au,
    0x00000003u, 0x66756269u, 0x5f726566u, 0x7366666fu, 0x00007465u, 0x00060006u, 0x0000000au, 0x00000004u,
    0x66756276u, 0x5f726566u, 0x00006469u, 0x00060006u, 0x0000000au, 0x00000005u, 0x66756269u, 0x5f726566u,
    0x00006469u, 0x00060006u, 0x0000000au, 0x00000006u, 0x74786574u, 0x30657275u, 0x0064695fu, 0x00060006u,
    0x0000000au, 0x00000007u, 0x706d6173u, 0x3072656cu, 0x0064695fu, 0x00050005u, 0x0000000bu, 0x66667542u,
    0x64497265u, 0x00000000u, 0x00070006u, 0x0000000bu, 0x00000000u, 0x66667562u, 0x695f7265u, 0x61765f64u,
    0x0065756cu, 0x00050005u, 0x0000000cu, 0x67616d49u, 0x65695665u, 0x00644977u, 0x00080006u, 0x0000000cu,
    0x00000000u, 0x67616d69u, 0x69765f65u, 0x695f7765u, 0x61765f64u, 0x0065756cu, 0x00050005u, 0x0000000du,
    0x706d6153u, 0x4972656cu, 0x00000064u, 0x00080006u, 0x0000000du, 0x00000000u, 0x706d6173u, 0x5f72656cu,
    0x765f6469u, 0x65756c61u, 0x00000000u, 0x00030005u, 0x0000000eu, 0x00000070u, 0x00060005u, 0x00000003u,
    0x762e6e69u, 0x432e7261u, 0x524f4c4fu, 0x00000030u, 0x00070005u, 0x00000004u, 0x762e6e69u, 0x542e7261u,
    0x4f435845u, 0x3044524fu, 0x00000000u, 0x00070005u, 0x00000005u, 0x2e74756fu, 0x2e726176u, 0x545f5653u,
    0x65677261u, 0x00000074u, 0x00040005u, 0x00000002u, 0x6d5f7366u, 0x006e6961u, 0x00070005u, 0x0000000fu,
    0x65707974u, 0x6d61732eu, 0x64656c70u, 0x616d692eu, 0x00006567u, 0x00040047u, 0x00000003u, 0x0000001eu,
    0x00000000u, 0x00040047u, 0x00000004u, 0x0000001eu, 0x00000001u, 0x00040047u, 0x00000005u, 0x0000001eu,
    0x00000000u, 0x00040047u, 0x00000007u, 0x00000022u, 0x00000000u, 0x00040047u, 0x00000007u, 0x00000021u,
    0x00000003u, 0x00040047u, 0x00000009u, 0x00000022u, 0x00000000u, 0x00040047u, 0x00000009u, 0x00000021u,
    0x00000002u, 0x00050048u, 0x0000000bu, 0x00000000u, 0x00000023u, 0x00000000u, 0x00050048u, 0x0000000cu,
    0x00000000u, 0x00000023u, 0x00000000u, 0x00050048u, 0x0000000du, 0x00000000u, 0x00000023u, 0x00000000u,
    0x00050048u, 0x0000000au, 0x00000000u, 0x00000023u, 0x00000000u, 0x00050048u, 0x0000000au, 0x00000001u,
    0x00000023u, 0x00000008u, 0x00050048u, 0x0000000au, 0x00000002u, 0x00000023u, 0x00000010u, 0x00050048u,
    0x0000000au, 0x00000003u, 0x00000023u, 0x00000014u, 0x00050048u, 0x0000000au, 0x00000004u, 0x00000023u,
    0x00000018u, 0x00050048u, 0x0000000au, 0x00000005u, 0x00000023u, 0x0000001cu, 0x00050048u, 0x0000000au,
    0x00000006u, 0x00000023u, 0x00000020u, 0x00050048u, 0x0000000au, 0x00000007u, 0x00000023u, 0x00000024u,
    0x00030047u, 0x0000000au, 0x00000002u, 0x00040015u, 0x00000010u, 0x00000020u, 0x00000001u, 0x0004002bu,
    0x00000010u, 0x00000011u, 0x00000006u, 0x0004002bu, 0x00000010u, 0x00000012u, 0x00000007u, 0x00040015u,
    0x00000013u, 0x00000020u, 0x00000000u, 0x0004002bu, 0x00000013u, 0x00000014u, 0x00ffffffu, 0x00030016u,
    0x00000015u, 0x00000020u, 0x0004002bu, 0x00000015u, 0x00000016u, 0x3d25aee6u, 0x00040017u, 0x00000017u,
    0x00000015u, 0x00000003u, 0x0006002cu, 0x00000017u, 0x00000018u, 0x00000016u, 0x00000016u, 0x00000016u,
    0x0004002bu, 0x00000015u, 0x00000019u, 0x00000000u, 0x0006002cu, 0x00000017u, 0x0000001au, 0x00000019u,
    0x00000019u, 0x00000019u, 0x0004002bu, 0x00000015u, 0x0000001bu, 0x3f800000u, 0x0006002cu, 0x00000017u,
    0x0000001cu, 0x0000001bu, 0x0000001bu, 0x0000001bu, 0x0004002bu, 0x00000015u, 0x0000001du, 0x3d6147aeu,
    0x0006002cu, 0x00000017u, 0x0000001eu, 0x0000001du, 0x0000001du, 0x0000001du, 0x0004002bu, 0x00000015u,
    0x0000001fu, 0x4019999au, 0x0006002cu, 0x00000017u, 0x00000020u, 0x0000001fu, 0x0000001fu, 0x0000001fu,
    0x0004002bu, 0x00000010u, 0x00000021u, 0x00000003u, 0x0002001au, 0x00000006u, 0x0003001du, 0x00000022u,
    0x00000006u, 0x00040020u, 0x00000023u, 0x00000000u, 0x00000022u, 0x00090019u, 0x00000008u, 0x00000015u,
    0x00000001u, 0x00000002u, 0x00000000u, 0x00000000u, 0x00000001u, 0x00000000u, 0x0003001du, 0x00000024u,
    0x00000008u, 0x00040020u, 0x00000025u, 0x00000000u, 0x00000024u, 0x00040017u, 0x00000026u, 0x00000015u,
    0x00000002u, 0x0003001eu, 0x0000000bu, 0x00000013u, 0x0003001eu, 0x0000000cu, 0x00000013u, 0x0003001eu,
    0x0000000du, 0x00000013u, 0x000a001eu, 0x0000000au, 0x00000026u, 0x00000026u, 0x00000013u, 0x00000013u,
    0x0000000bu, 0x0000000bu, 0x0000000cu, 0x0000000du, 0x00040020u, 0x00000027u, 0x00000009u, 0x0000000au,
    0x00040017u, 0x00000028u, 0x00000015u, 0x00000004u, 0x00040020u, 0x00000029u, 0x00000001u, 0x00000028u,
    0x00040020u, 0x0000002au, 0x00000001u, 0x00000026u, 0x00040020u, 0x0000002bu, 0x00000003u, 0x00000028u,
    0x00020013u, 0x0000002cu, 0x00030021u, 0x0000002du, 0x0000002cu, 0x00040020u, 0x0000002eu, 0x00000007u,
    0x00000028u, 0x00040020u, 0x0000002fu, 0x00000009u, 0x0000000cu, 0x00040020u, 0x00000030u, 0x00000009u,
    0x0000000du, 0x0003001bu, 0x0000000fu, 0x00000008u, 0x00040020u, 0x00000031u, 0x00000000u, 0x00000008u,
    0x00040020u, 0x00000032u, 0x00000000u, 0x00000006u, 0x00040020u, 0x00000033u, 0x00000007u, 0x00000015u,
    0x0004003bu, 0x00000023u, 0x00000007u, 0x00000000u, 0x0004003bu, 0x00000025u, 0x00000009u, 0x00000000u,
    0x0004003bu, 0x00000027u, 0x0000000eu, 0x00000009u, 0x0004003bu, 0x00000029u, 0x00000003u, 0x00000001u,
    0x0004003bu, 0x0000002au, 0x00000004u, 0x00000001u, 0x0004003bu, 0x0000002bu, 0x00000005u, 0x00000003u,
    0x0004002bu, 0x00000015u, 0x00000034u, 0x3d9e8391u, 0x0006002cu, 0x00000017u, 0x00000035u, 0x00000034u,
    0x00000034u, 0x00000034u, 0x0004002bu, 0x00000015u, 0x00000036u, 0x3f72a76fu, 0x0006002cu, 0x00000017u,
    0x00000037u, 0x00000036u, 0x00000036u, 0x00000036u, 0x00050036u, 0x0000002cu, 0x00000002u, 0x00000000u,
    0x0000002du, 0x000200f8u, 0x00000038u, 0x0004003bu, 0x0000002eu, 0x00000039u, 0x00000007u, 0x0004003du,
    0x00000028u, 0x0000003au, 0x00000003u, 0x0004003du, 0x00000026u, 0x0000003bu, 0x00000004u, 0x00050041u,
    0x0000002fu, 0x0000003cu, 0x0000000eu, 0x00000011u, 0x0004003du, 0x0000000cu, 0x0000003du, 0x0000003cu,
    0x00050051u, 0x00000013u, 0x0000003eu, 0x0000003du, 0x00000000u, 0x000500c7u, 0x00000013u, 0x0000003fu,
    0x00000014u, 0x0000003eu, 0x00050041u, 0x00000031u, 0x00000040u, 0x00000009u, 0x0000003fu, 0x0004003du,
    0x00000008u, 0x00000041u, 0x00000040u, 0x00050041u, 0x00000030u, 0x00000042u, 0x0000000eu, 0x00000012u,
    0x0004003du, 0x0000000du, 0x00000043u, 0x00000042u, 0x00050051u, 0x00000013u, 0x00000044u, 0x00000043u,
    0x00000000u, 0x000500c7u, 0x00000013u, 0x00000045u, 0x00000014u, 0x00000044u, 0x00050041u, 0x00000032u,
    0x00000046u, 0x00000007u, 0x00000045u, 0x0004003du, 0x00000006u, 0x00000047u, 0x00000046u, 0x0003003eu,
    0x00000039u, 0x0000003au, 0x0008004fu, 0x00000017u, 0x00000048u, 0x0000003au, 0x0000003au, 0x00000000u,
    0x00000001u, 0x00000002u, 0x00050083u, 0x00000017u, 0x00000049u, 0x00000048u, 0x00000018u, 0x0006000cu,
    0x00000017u, 0x0000004au, 0x00000001u, 0x00000009u, 0x00000049u, 0x0008000cu, 0x00000017u, 0x0000004bu,
    0x00000001u, 0x0000002bu, 0x0000004au, 0x0000001au, 0x0000001cu, 0x00050085u, 0x00000017u, 0x0000004cu,
    0x00000048u, 0x00000035u, 0x00050081u, 0x00000017u, 0x0000004du, 0x00000048u, 0x0000001eu, 0x00050085u,
    0x00000017u, 0x0000004eu, 0x0000004du, 0x00000037u, 0x0007000cu, 0x00000017u, 0x0000004fu, 0x00000001u,
    0x0000001au, 0x0000004eu, 0x00000020u, 0x0008000cu, 0x00000017u, 0x00000050u, 0x00000001u, 0x0000002eu,
    0x0000004cu, 0x0000004fu, 0x0000004bu, 0x00050041u, 0x00000033u, 0x00000051u, 0x00000039u, 0x00000021u,
    0x0004003du, 0x00000015u, 0x00000052u, 0x00000051u, 0x00050051u, 0x00000015u, 0x00000053u, 0x00000050u,
    0x00000000u, 0x00050051u, 0x00000015u, 0x00000054u, 0x00000050u, 0x00000001u, 0x00050051u, 0x00000015u,
    0x00000055u, 0x00000050u, 0x00000002u, 0x00070050u, 0x00000028u, 0x00000056u, 0x00000053u, 0x00000054u,
    0x00000055u, 0x00000052u, 0x00050056u, 0x0000000fu, 0x00000057u, 0x00000041u, 0x00000047u, 0x00060057u,
    0x00000028u, 0x00000058u, 0x00000057u, 0x0000003bu, 0x00000000u, 0x00050085u, 0x00000028u, 0x00000059u,
    0x00000056u, 0x00000058u, 0x0003003eu, 0x00000005u, 0x00000059u, 0x000100fdu, 0x00010038u,
    // clang-format on
};

namespace daxa
{
    ImGuiRenderer::ImGuiRenderer(ImGuiRendererInfo const & info)
        : ManagedPtr{new ImplImGuiRenderer(info)}
    {
    }

    ImGuiRenderer::~ImGuiRenderer() = default;

    void ImGuiRenderer::record_commands(ImDrawData * draw_data, CommandList & cmd_list, ImageId target_image, u32 size_x, u32 size_y)
    {
        auto & impl = *as<ImplImGuiRenderer>();
        impl.record_commands(draw_data, cmd_list, target_image, size_x, size_y);
    }

    void ImGuiRenderer::record_task(ImDrawData * /* draw_data */, TaskList & /* task_list */, TaskImageId /* task_swapchain_image */, u32 /* size_x */, u32 /* size_y */)
    {
    }

    void ImplImGuiRenderer::recreate_vbuffer(usize vbuffer_new_size)
    {
        vbuffer = info.device.create_buffer({
            .size = static_cast<u32>(vbuffer_new_size),
            .debug_name = std::string("dear ImGui vertex buffer"),
        });
    }
    void ImplImGuiRenderer::recreate_ibuffer(usize ibuffer_new_size)
    {
        ibuffer = info.device.create_buffer({
            .size = static_cast<u32>(ibuffer_new_size),
            .debug_name = std::string("dear ImGui index buffer"),
        });
    }

    void ImplImGuiRenderer::record_commands(ImDrawData * draw_data, CommandList & cmd_list, ImageId target_image, u32 size_x, u32 size_y)
    {
        ++frame_count;
        if ((draw_data != nullptr) && draw_data->TotalIdxCount > 0)
        {
            auto vbuffer_current_size = info.device.info_buffer(vbuffer).size;
            auto vbuffer_needed_size = static_cast<usize>(draw_data->TotalVtxCount) * sizeof(ImDrawVert);
            auto ibuffer_current_size = info.device.info_buffer(ibuffer).size;
            auto ibuffer_needed_size = static_cast<usize>(draw_data->TotalIdxCount) * sizeof(ImDrawIdx);

            if (vbuffer_needed_size > vbuffer_current_size)
            {
                auto vbuffer_new_size = vbuffer_needed_size + 4096;
                info.device.destroy_buffer(vbuffer);
                recreate_vbuffer(vbuffer_new_size);
            }
            if (ibuffer_needed_size > ibuffer_current_size)
            {
                auto ibuffer_new_size = ibuffer_needed_size + 4096;
                info.device.destroy_buffer(ibuffer);
                recreate_ibuffer(ibuffer_new_size);
            }

            auto staging_vbuffer = info.device.create_buffer({
                .memory_flags = MemoryFlagBits::HOST_ACCESS_RANDOM,
                .size = static_cast<u32>(vbuffer_needed_size),
                .debug_name = std::string("dear ImGui vertex staging buffer ") + std::to_string(frame_count),
            });
            auto * vtx_dst = info.device.get_host_address_as<ImDrawVert>(staging_vbuffer);
            for (i32 n = 0; n < draw_data->CmdListsCount; n++)
            {
                ImDrawList const * draws = draw_data->CmdLists[n];
                std::memcpy(vtx_dst, draws->VtxBuffer.Data, static_cast<usize>(draws->VtxBuffer.Size) * sizeof(ImDrawVert));
                vtx_dst += draws->VtxBuffer.Size;
            }
            cmd_list.destroy_buffer_deferred(staging_vbuffer);
            auto staging_ibuffer = info.device.create_buffer({
                .memory_flags = MemoryFlagBits::HOST_ACCESS_RANDOM,
                .size = static_cast<u32>(ibuffer_needed_size),
                .debug_name = std::string("dear ImGui index staging buffer ") + std::to_string(frame_count),
            });
            auto * idx_dst = info.device.get_host_address_as<ImDrawIdx>(staging_ibuffer);
            for (i32 n = 0; n < draw_data->CmdListsCount; n++)
            {
                ImDrawList const * draws = draw_data->CmdLists[n];
                std::memcpy(idx_dst, draws->IdxBuffer.Data, static_cast<usize>(draws->IdxBuffer.Size) * sizeof(ImDrawIdx));
                idx_dst += draws->IdxBuffer.Size;
            }
            cmd_list.destroy_buffer_deferred(staging_ibuffer);
            cmd_list.pipeline_barrier({
                .awaited_pipeline_access = daxa::AccessConsts::HOST_WRITE,
                .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
            });
            cmd_list.copy_buffer_to_buffer({
                .src_buffer = staging_ibuffer,
                .dst_buffer = ibuffer,
                .size = ibuffer_needed_size,
            });
            cmd_list.copy_buffer_to_buffer({
                .src_buffer = staging_vbuffer,
                .dst_buffer = vbuffer,
                .size = vbuffer_needed_size,
            });
            cmd_list.pipeline_barrier({
                .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
                .waiting_pipeline_access = daxa::AccessConsts::VERTEX_SHADER_READ | daxa::AccessConsts::INDEX_INPUT_READ,
            });

            cmd_list.set_pipeline(raster_pipeline);
            cmd_list.begin_renderpass({
                .color_attachments = {{.image_view = target_image.default_view(), .load_op = AttachmentLoadOp::LOAD}},
                .render_area = {.x = 0, .y = 0, .width = size_x, .height = size_y},
            });

            cmd_list.set_index_buffer(ibuffer, 0, sizeof(ImDrawIdx));

            auto push = Push{};
            push.scale = {2.0f / draw_data->DisplaySize.x, 2.0f / draw_data->DisplaySize.y};
            push.translate = {-1.0f - draw_data->DisplayPos.x * push.scale.x, -1.0f - draw_data->DisplayPos.y * push.scale.y};
            ImVec2 const clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
            ImVec2 const clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)
            i32 global_vtx_offset = 0;
            i32 global_idx_offset = 0;
            push.vbuffer_id = vbuffer;
            push.ibuffer_id = ibuffer;
            push.sampler0_id = sampler;

            for (i32 n = 0; n < draw_data->CmdListsCount; n++)
            {
                ImDrawList const * draws = draw_data->CmdLists[n];
                for (i32 cmd_i = 0; cmd_i < draws->CmdBuffer.Size; cmd_i++)
                {
                    ImDrawCmd const * pcmd = &draws->CmdBuffer[cmd_i];

                    // Project scissor/clipping rectangles into framebuffer space
                    ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                    ImVec2 const clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

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
                    push.texture0_id = *reinterpret_cast<daxa::ImageViewId const *>(&pcmd->TextureId);

                    push.vbuffer_offset = pcmd->VtxOffset + static_cast<u32>(global_vtx_offset);
                    push.ibuffer_offset = pcmd->IdxOffset + static_cast<u32>(global_idx_offset);

                    cmd_list.push_constant(push);
                    cmd_list.draw_indexed({
                        .index_count = pcmd->ElemCount,
                        .first_index = pcmd->IdxOffset + static_cast<u32>(global_idx_offset),
                        .vertex_offset = static_cast<i32>(pcmd->VtxOffset) + global_vtx_offset,
                    });
                }
                global_idx_offset += draws->IdxBuffer.Size;
                global_vtx_offset += draws->VtxBuffer.Size;
            }

            cmd_list.end_renderpass();
        }
    }

    ImplImGuiRenderer::ImplImGuiRenderer(ImGuiRendererInfo a_info)
        : info{std::move(a_info)},
          // clang-format off
        raster_pipeline{this->info.device.create_raster_pipeline({
            .vertex_shader_info = {.binary = {imgui_vert_spv.begin(), imgui_vert_spv.end()}, .entry_point = "vs_main"},
            .fragment_shader_info = {.binary = {imgui_frag_spv.begin(), imgui_frag_spv.end()}, .entry_point = "fs_main"},
            .color_attachments = {
                {
                    .format = info.format,
                    .blend = {
                        .blend_enable = 1u,
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
        })}
    // clang-format on
    {
        set_imgui_style();
        recreate_vbuffer(4096);
        recreate_ibuffer(4096);
        sampler = this->info.device.create_sampler({.debug_name = "dear ImGui sampler"});

        ImGuiIO & io = ImGui::GetIO();
        u8 * pixels = nullptr;
        i32 width = 0;
        i32 height = 0;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        usize const upload_size = static_cast<usize>(width) * static_cast<usize>(height) * 4 * sizeof(u8);
        font_sheet = this->info.device.create_image({
            .size = {static_cast<u32>(width), static_cast<u32>(height), 1},
            .usage = ImageUsageFlagBits::TRANSFER_DST | ImageUsageFlagBits::SHADER_READ_ONLY,
            .debug_name = "dear ImGui font sheet",
        });

        auto texture_staging_buffer = this->info.device.create_buffer({
            .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
            .size = static_cast<u32>(upload_size),
        });

        u8 * staging_buffer_data = this->info.device.get_host_address_as<u8>(texture_staging_buffer);
        std::memcpy(staging_buffer_data, pixels, upload_size);

        auto cmd_list = this->info.device.create_command_list({.debug_name = "dear ImGui Font Sheet Upload"});
        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::HOST_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ_WRITE,
            .after_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .image_slice = {
                .base_mip_level = 0,
                .level_count = 1,
                .base_array_layer = 0,
                .layer_count = 1,
            },
            .image_id = font_sheet,
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
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::FRAGMENT_SHADER_READ,
            .before_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .after_layout = daxa::ImageLayout::READ_ONLY_OPTIMAL,
            .image_slice = {
                .base_mip_level = 0,
                .level_count = 1,
                .base_array_layer = 0,
                .layer_count = 1,
            },
            .image_id = font_sheet,
        });
        cmd_list.complete();
        this->info.device.submit_commands({
            .command_lists = {cmd_list},
        });
        this->info.device.destroy_buffer(texture_staging_buffer);
        auto image_view = font_sheet.default_view();
        auto * imgui_texid = reinterpret_cast<ImTextureID>(static_cast<usize>(*reinterpret_cast<u32 *>(&image_view)));
        io.Fonts->SetTexID(imgui_texid);
    }

    ImplImGuiRenderer::~ImplImGuiRenderer()
    {
        this->info.device.destroy_buffer(vbuffer);
        this->info.device.destroy_buffer(ibuffer);
        this->info.device.destroy_sampler(sampler);
        this->info.device.destroy_image(font_sheet);
    }
} // namespace daxa

#endif
