#if DAXA_BUILT_WITH_UTILS_TASK_GRAPH && DAXA_BUILT_WITH_UTILS_IMGUI
#include <daxa/utils/task_graph_types.hpp>

#if DAXA_ENABLE_TASK_GRAPH_MK2

#include <daxa/utils/imgui.hpp>
#include <imgui_internal.h>
#include <implot.h>
#include "impl_task_graph_mk2.hpp"
#include "impl_task_graph_ui.hpp"
#include "impl_resource_viewer.slang"

static inline ImVec2 operator+(ImVec2 const & lhs, ImVec2 const & rhs);
static inline ImVec2 operator-(ImVec2 const & lhs, ImVec2 const & rhs);
static inline ImVec2 operator*(ImVec2 const & lhs, ImVec2 const & rhs);
static inline ImVec2 operator/(ImVec2 const & lhs, ImVec2 const & rhs);
static inline ImPlotPoint operator+(ImPlotPoint const & lhs, ImPlotPoint const & rhs) { return ImPlotPoint(lhs.x + rhs.x, lhs.y + rhs.y); }
static inline ImPlotPoint operator-(ImPlotPoint const & lhs, ImPlotPoint const & rhs) { return ImPlotPoint(lhs.x - rhs.x, lhs.y - rhs.y); }
static inline ImPlotPoint operator*(ImPlotPoint const & lhs, ImPlotPoint const & rhs) { return ImPlotPoint(lhs.x * rhs.x, lhs.y * rhs.y); }
static inline ImPlotPoint operator/(ImPlotPoint const & lhs, ImPlotPoint const & rhs) { return ImPlotPoint(lhs.x / rhs.x, lhs.y / rhs.y); }

namespace daxa
{

    enum struct ScalarKind
    {
        FLOAT,
        INT,
        UINT
    };

    auto channel_count_of_format(daxa::Format format) -> daxa::u32
    {
        switch (format)
        {
        case daxa::Format::UNDEFINED: return 0;
        case daxa::Format::R4G4_UNORM_PACK8: return 2;
        case daxa::Format::R4G4B4A4_UNORM_PACK16: return 4;
        case daxa::Format::B4G4R4A4_UNORM_PACK16: return 4;
        case daxa::Format::R5G6B5_UNORM_PACK16: return 3;
        case daxa::Format::B5G6R5_UNORM_PACK16: return 3;
        case daxa::Format::R5G5B5A1_UNORM_PACK16: return 4;
        case daxa::Format::B5G5R5A1_UNORM_PACK16: return 4;
        case daxa::Format::A1R5G5B5_UNORM_PACK16: return 4;
        case daxa::Format::R8_UNORM: return 1;
        case daxa::Format::R8_SNORM: return 1;
        case daxa::Format::R8_USCALED: return 1;
        case daxa::Format::R8_SSCALED: return 1;
        case daxa::Format::R8_UINT: return 1;
        case daxa::Format::R8_SINT: return 1;
        case daxa::Format::R8_SRGB: return 1;
        case daxa::Format::R8G8_UNORM: return 2;
        case daxa::Format::R8G8_SNORM: return 2;
        case daxa::Format::R8G8_USCALED: return 2;
        case daxa::Format::R8G8_SSCALED: return 2;
        case daxa::Format::R8G8_UINT: return 2;
        case daxa::Format::R8G8_SINT: return 2;
        case daxa::Format::R8G8_SRGB: return 2;
        case daxa::Format::R8G8B8_UNORM: return 3;
        case daxa::Format::R8G8B8_SNORM: return 3;
        case daxa::Format::R8G8B8_USCALED: return 3;
        case daxa::Format::R8G8B8_SSCALED: return 3;
        case daxa::Format::R8G8B8_UINT: return 3;
        case daxa::Format::R8G8B8_SINT: return 3;
        case daxa::Format::R8G8B8_SRGB: return 3;
        case daxa::Format::B8G8R8_UNORM: return 3;
        case daxa::Format::B8G8R8_SNORM: return 3;
        case daxa::Format::B8G8R8_USCALED: return 3;
        case daxa::Format::B8G8R8_SSCALED: return 3;
        case daxa::Format::B8G8R8_UINT: return 3;
        case daxa::Format::B8G8R8_SINT: return 3;
        case daxa::Format::B8G8R8_SRGB: return 3;
        case daxa::Format::R8G8B8A8_UNORM: return 4;
        case daxa::Format::R8G8B8A8_SNORM: return 4;
        case daxa::Format::R8G8B8A8_USCALED: return 4;
        case daxa::Format::R8G8B8A8_SSCALED: return 4;
        case daxa::Format::R8G8B8A8_UINT: return 4;
        case daxa::Format::R8G8B8A8_SINT: return 4;
        case daxa::Format::R8G8B8A8_SRGB: return 4;
        case daxa::Format::B8G8R8A8_UNORM: return 4;
        case daxa::Format::B8G8R8A8_SNORM: return 4;
        case daxa::Format::B8G8R8A8_USCALED: return 4;
        case daxa::Format::B8G8R8A8_SSCALED: return 4;
        case daxa::Format::B8G8R8A8_UINT: return 4;
        case daxa::Format::B8G8R8A8_SINT: return 4;
        case daxa::Format::B8G8R8A8_SRGB: return 4;
        case daxa::Format::A8B8G8R8_UNORM_PACK32: return 4;
        case daxa::Format::A8B8G8R8_SNORM_PACK32: return 4;
        case daxa::Format::A8B8G8R8_USCALED_PACK32: return 4;
        case daxa::Format::A8B8G8R8_SSCALED_PACK32: return 4;
        case daxa::Format::A8B8G8R8_UINT_PACK32: return 4;
        case daxa::Format::A8B8G8R8_SINT_PACK32: return 4;
        case daxa::Format::A8B8G8R8_SRGB_PACK32: return 4;
        case daxa::Format::A2R10G10B10_UNORM_PACK32: return 4;
        case daxa::Format::A2R10G10B10_SNORM_PACK32: return 4;
        case daxa::Format::A2R10G10B10_USCALED_PACK32: return 4;
        case daxa::Format::A2R10G10B10_SSCALED_PACK32: return 4;
        case daxa::Format::A2R10G10B10_UINT_PACK32: return 4;
        case daxa::Format::A2R10G10B10_SINT_PACK32: return 4;
        case daxa::Format::A2B10G10R10_UNORM_PACK32: return 4;
        case daxa::Format::A2B10G10R10_SNORM_PACK32: return 4;
        case daxa::Format::A2B10G10R10_USCALED_PACK32: return 4;
        case daxa::Format::A2B10G10R10_SSCALED_PACK32: return 4;
        case daxa::Format::A2B10G10R10_UINT_PACK32: return 4;
        case daxa::Format::A2B10G10R10_SINT_PACK32: return 4;
        case daxa::Format::R16_UNORM: return 1;
        case daxa::Format::R16_SNORM: return 1;
        case daxa::Format::R16_USCALED: return 1;
        case daxa::Format::R16_SSCALED: return 1;
        case daxa::Format::R16_UINT: return 1;
        case daxa::Format::R16_SINT: return 1;
        case daxa::Format::R16_SFLOAT: return 1;
        case daxa::Format::R16G16_UNORM: return 2;
        case daxa::Format::R16G16_SNORM: return 2;
        case daxa::Format::R16G16_USCALED: return 2;
        case daxa::Format::R16G16_SSCALED: return 2;
        case daxa::Format::R16G16_UINT: return 2;
        case daxa::Format::R16G16_SINT: return 2;
        case daxa::Format::R16G16_SFLOAT: return 2;
        case daxa::Format::R16G16B16_UNORM: return 3;
        case daxa::Format::R16G16B16_SNORM: return 3;
        case daxa::Format::R16G16B16_USCALED: return 3;
        case daxa::Format::R16G16B16_SSCALED: return 3;
        case daxa::Format::R16G16B16_UINT: return 3;
        case daxa::Format::R16G16B16_SINT: return 3;
        case daxa::Format::R16G16B16_SFLOAT: return 3;
        case daxa::Format::R16G16B16A16_UNORM: return 4;
        case daxa::Format::R16G16B16A16_SNORM: return 4;
        case daxa::Format::R16G16B16A16_USCALED: return 4;
        case daxa::Format::R16G16B16A16_SSCALED: return 4;
        case daxa::Format::R16G16B16A16_UINT: return 4;
        case daxa::Format::R16G16B16A16_SINT: return 4;
        case daxa::Format::R16G16B16A16_SFLOAT: return 4;
        case daxa::Format::R32_UINT: return 1;
        case daxa::Format::R32_SINT: return 1;
        case daxa::Format::R32_SFLOAT: return 1;
        case daxa::Format::R32G32_UINT: return 2;
        case daxa::Format::R32G32_SINT: return 2;
        case daxa::Format::R32G32_SFLOAT: return 2;
        case daxa::Format::R32G32B32_UINT: return 3;
        case daxa::Format::R32G32B32_SINT: return 3;
        case daxa::Format::R32G32B32_SFLOAT: return 3;
        case daxa::Format::R32G32B32A32_UINT: return 4;
        case daxa::Format::R32G32B32A32_SINT: return 4;
        case daxa::Format::R32G32B32A32_SFLOAT: return 4;
        case daxa::Format::R64_UINT: return 1;
        case daxa::Format::R64_SINT: return 1;
        case daxa::Format::R64_SFLOAT: return 1;
        case daxa::Format::R64G64_UINT: return 2;
        case daxa::Format::R64G64_SINT: return 2;
        case daxa::Format::R64G64_SFLOAT: return 2;
        case daxa::Format::R64G64B64_UINT: return 3;
        case daxa::Format::R64G64B64_SINT: return 3;
        case daxa::Format::R64G64B64_SFLOAT: return 3;
        case daxa::Format::R64G64B64A64_UINT: return 4;
        case daxa::Format::R64G64B64A64_SINT: return 4;
        case daxa::Format::R64G64B64A64_SFLOAT: return 4;
        case daxa::Format::B10G11R11_UFLOAT_PACK32: return 3;
        case daxa::Format::E5B9G9R9_UFLOAT_PACK32: return 4;
        case daxa::Format::D16_UNORM: return 1;
        case daxa::Format::X8_D24_UNORM_PACK32: return 2;
        case daxa::Format::D32_SFLOAT: return 1;
        case daxa::Format::S8_UINT: return 1;
        case daxa::Format::D16_UNORM_S8_UINT: return 2;
        case daxa::Format::D24_UNORM_S8_UINT: return 2;
        case daxa::Format::D32_SFLOAT_S8_UINT: return 2;
        case daxa::Format::BC1_RGB_UNORM_BLOCK: return 3;
        case daxa::Format::BC1_RGB_SRGB_BLOCK: return 3;
        case daxa::Format::BC1_RGBA_UNORM_BLOCK: return 4;
        case daxa::Format::BC1_RGBA_SRGB_BLOCK: return 4;
        case daxa::Format::BC2_UNORM_BLOCK: return 1;
        case daxa::Format::BC2_SRGB_BLOCK: return 1;
        case daxa::Format::BC3_UNORM_BLOCK: return 1;
        case daxa::Format::BC3_SRGB_BLOCK: return 1;
        case daxa::Format::BC4_UNORM_BLOCK: return 1;
        case daxa::Format::BC4_SNORM_BLOCK: return 1;
        case daxa::Format::BC5_UNORM_BLOCK: return 1;
        case daxa::Format::BC5_SNORM_BLOCK: return 1;
        case daxa::Format::BC6H_SFLOAT_BLOCK: return 3;
        case daxa::Format::BC6H_UFLOAT_BLOCK: return 3;
        case daxa::Format::BC7_SRGB_BLOCK: return 4;
        case daxa::Format::BC7_UNORM_BLOCK: return 4;
        default: return 0;
        }
    }

    auto scalar_kind_of_format(daxa::Format format) -> ScalarKind
    {
        switch (format)
        {
        case daxa::Format::UNDEFINED: return ScalarKind::FLOAT;
        case daxa::Format::R4G4_UNORM_PACK8: return ScalarKind::FLOAT;
        case daxa::Format::R4G4B4A4_UNORM_PACK16: return ScalarKind::FLOAT;
        case daxa::Format::B4G4R4A4_UNORM_PACK16: return ScalarKind::FLOAT;
        case daxa::Format::R5G6B5_UNORM_PACK16: return ScalarKind::FLOAT;
        case daxa::Format::B5G6R5_UNORM_PACK16: return ScalarKind::FLOAT;
        case daxa::Format::R5G5B5A1_UNORM_PACK16: return ScalarKind::FLOAT;
        case daxa::Format::B5G5R5A1_UNORM_PACK16: return ScalarKind::FLOAT;
        case daxa::Format::A1R5G5B5_UNORM_PACK16: return ScalarKind::FLOAT;
        case daxa::Format::R8_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::R8_SNORM: return ScalarKind::FLOAT;
        case daxa::Format::R8_USCALED: return ScalarKind::FLOAT;
        case daxa::Format::R8_SSCALED: return ScalarKind::FLOAT;
        case daxa::Format::R8_UINT: return ScalarKind::UINT;
        case daxa::Format::R8_SINT: return ScalarKind::INT;
        case daxa::Format::R8_SRGB: return ScalarKind::FLOAT;
        case daxa::Format::R8G8_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::R8G8_SNORM: return ScalarKind::FLOAT;
        case daxa::Format::R8G8_USCALED: return ScalarKind::FLOAT;
        case daxa::Format::R8G8_SSCALED: return ScalarKind::FLOAT;
        case daxa::Format::R8G8_UINT: return ScalarKind::UINT;
        case daxa::Format::R8G8_SINT: return ScalarKind::INT;
        case daxa::Format::R8G8_SRGB: return ScalarKind::FLOAT;
        case daxa::Format::R8G8B8_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::R8G8B8_SNORM: return ScalarKind::FLOAT;
        case daxa::Format::R8G8B8_USCALED: return ScalarKind::FLOAT;
        case daxa::Format::R8G8B8_SSCALED: return ScalarKind::FLOAT;
        case daxa::Format::R8G8B8_UINT: return ScalarKind::UINT;
        case daxa::Format::R8G8B8_SINT: return ScalarKind::INT;
        case daxa::Format::R8G8B8_SRGB: return ScalarKind::FLOAT;
        case daxa::Format::B8G8R8_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::B8G8R8_SNORM: return ScalarKind::FLOAT;
        case daxa::Format::B8G8R8_USCALED: return ScalarKind::FLOAT;
        case daxa::Format::B8G8R8_SSCALED: return ScalarKind::FLOAT;
        case daxa::Format::B8G8R8_UINT: return ScalarKind::UINT;
        case daxa::Format::B8G8R8_SINT: return ScalarKind::INT;
        case daxa::Format::B8G8R8_SRGB: return ScalarKind::FLOAT;
        case daxa::Format::R8G8B8A8_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::R8G8B8A8_SNORM: return ScalarKind::FLOAT;
        case daxa::Format::R8G8B8A8_USCALED: return ScalarKind::FLOAT;
        case daxa::Format::R8G8B8A8_SSCALED: return ScalarKind::FLOAT;
        case daxa::Format::R8G8B8A8_UINT: return ScalarKind::UINT;
        case daxa::Format::R8G8B8A8_SINT: return ScalarKind::INT;
        case daxa::Format::R8G8B8A8_SRGB: return ScalarKind::FLOAT;
        case daxa::Format::B8G8R8A8_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::B8G8R8A8_SNORM: return ScalarKind::FLOAT;
        case daxa::Format::B8G8R8A8_USCALED: return ScalarKind::FLOAT;
        case daxa::Format::B8G8R8A8_SSCALED: return ScalarKind::FLOAT;
        case daxa::Format::B8G8R8A8_UINT: return ScalarKind::UINT;
        case daxa::Format::B8G8R8A8_SINT: return ScalarKind::INT;
        case daxa::Format::B8G8R8A8_SRGB: return ScalarKind::FLOAT;
        case daxa::Format::A8B8G8R8_UNORM_PACK32: return ScalarKind::FLOAT;
        case daxa::Format::A8B8G8R8_SNORM_PACK32: return ScalarKind::FLOAT;
        case daxa::Format::A8B8G8R8_USCALED_PACK32: return ScalarKind::FLOAT;
        case daxa::Format::A8B8G8R8_SSCALED_PACK32: return ScalarKind::FLOAT;
        case daxa::Format::A8B8G8R8_UINT_PACK32: return ScalarKind::UINT;
        case daxa::Format::A8B8G8R8_SINT_PACK32: return ScalarKind::INT;
        case daxa::Format::A8B8G8R8_SRGB_PACK32: return ScalarKind::FLOAT;
        case daxa::Format::A2R10G10B10_UNORM_PACK32: return ScalarKind::FLOAT;
        case daxa::Format::A2R10G10B10_SNORM_PACK32: return ScalarKind::FLOAT;
        case daxa::Format::A2R10G10B10_USCALED_PACK32: return ScalarKind::FLOAT;
        case daxa::Format::A2R10G10B10_SSCALED_PACK32: return ScalarKind::FLOAT;
        case daxa::Format::A2R10G10B10_UINT_PACK32: return ScalarKind::UINT;
        case daxa::Format::A2R10G10B10_SINT_PACK32: return ScalarKind::INT;
        case daxa::Format::A2B10G10R10_UNORM_PACK32: return ScalarKind::FLOAT;
        case daxa::Format::A2B10G10R10_SNORM_PACK32: return ScalarKind::FLOAT;
        case daxa::Format::A2B10G10R10_USCALED_PACK32: return ScalarKind::FLOAT;
        case daxa::Format::A2B10G10R10_SSCALED_PACK32: return ScalarKind::FLOAT;
        case daxa::Format::A2B10G10R10_UINT_PACK32: return ScalarKind::UINT;
        case daxa::Format::A2B10G10R10_SINT_PACK32: return ScalarKind::INT;
        case daxa::Format::R16_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::R16_SNORM: return ScalarKind::FLOAT;
        case daxa::Format::R16_USCALED: return ScalarKind::FLOAT;
        case daxa::Format::R16_SSCALED: return ScalarKind::FLOAT;
        case daxa::Format::R16_UINT: return ScalarKind::UINT;
        case daxa::Format::R16_SINT: return ScalarKind::INT;
        case daxa::Format::R16_SFLOAT: return ScalarKind::FLOAT;
        case daxa::Format::R16G16_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::R16G16_SNORM: return ScalarKind::FLOAT;
        case daxa::Format::R16G16_USCALED: return ScalarKind::FLOAT;
        case daxa::Format::R16G16_SSCALED: return ScalarKind::FLOAT;
        case daxa::Format::R16G16_UINT: return ScalarKind::UINT;
        case daxa::Format::R16G16_SINT: return ScalarKind::INT;
        case daxa::Format::R16G16_SFLOAT: return ScalarKind::FLOAT;
        case daxa::Format::R16G16B16_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::R16G16B16_SNORM: return ScalarKind::FLOAT;
        case daxa::Format::R16G16B16_USCALED: return ScalarKind::FLOAT;
        case daxa::Format::R16G16B16_SSCALED: return ScalarKind::FLOAT;
        case daxa::Format::R16G16B16_UINT: return ScalarKind::UINT;
        case daxa::Format::R16G16B16_SINT: return ScalarKind::INT;
        case daxa::Format::R16G16B16_SFLOAT: return ScalarKind::FLOAT;
        case daxa::Format::R16G16B16A16_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::R16G16B16A16_SNORM: return ScalarKind::FLOAT;
        case daxa::Format::R16G16B16A16_USCALED: return ScalarKind::FLOAT;
        case daxa::Format::R16G16B16A16_SSCALED: return ScalarKind::FLOAT;
        case daxa::Format::R16G16B16A16_UINT: return ScalarKind::UINT;
        case daxa::Format::R16G16B16A16_SINT: return ScalarKind::INT;
        case daxa::Format::R16G16B16A16_SFLOAT: return ScalarKind::FLOAT;
        case daxa::Format::R32_UINT: return ScalarKind::UINT;
        case daxa::Format::R32_SINT: return ScalarKind::INT;
        case daxa::Format::R32_SFLOAT: return ScalarKind::FLOAT;
        case daxa::Format::R32G32_UINT: return ScalarKind::UINT;
        case daxa::Format::R32G32_SINT: return ScalarKind::INT;
        case daxa::Format::R32G32_SFLOAT: return ScalarKind::FLOAT;
        case daxa::Format::R32G32B32_UINT: return ScalarKind::UINT;
        case daxa::Format::R32G32B32_SINT: return ScalarKind::INT;
        case daxa::Format::R32G32B32_SFLOAT: return ScalarKind::FLOAT;
        case daxa::Format::R32G32B32A32_UINT: return ScalarKind::UINT;
        case daxa::Format::R32G32B32A32_SINT: return ScalarKind::INT;
        case daxa::Format::R32G32B32A32_SFLOAT: return ScalarKind::FLOAT;
        case daxa::Format::R64_UINT: return ScalarKind::UINT;
        case daxa::Format::R64_SINT: return ScalarKind::INT;
        case daxa::Format::R64_SFLOAT: return ScalarKind::FLOAT;
        case daxa::Format::R64G64_UINT: return ScalarKind::UINT;
        case daxa::Format::R64G64_SINT: return ScalarKind::INT;
        case daxa::Format::R64G64_SFLOAT: return ScalarKind::FLOAT;
        case daxa::Format::R64G64B64_UINT: return ScalarKind::UINT;
        case daxa::Format::R64G64B64_SINT: return ScalarKind::INT;
        case daxa::Format::R64G64B64_SFLOAT: return ScalarKind::FLOAT;
        case daxa::Format::R64G64B64A64_UINT: return ScalarKind::UINT;
        case daxa::Format::R64G64B64A64_SINT: return ScalarKind::INT;
        case daxa::Format::R64G64B64A64_SFLOAT: return ScalarKind::FLOAT;
        case daxa::Format::B10G11R11_UFLOAT_PACK32: return ScalarKind::FLOAT;
        case daxa::Format::E5B9G9R9_UFLOAT_PACK32: return ScalarKind::FLOAT;
        case daxa::Format::D16_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::X8_D24_UNORM_PACK32: return ScalarKind::FLOAT;
        case daxa::Format::D32_SFLOAT: return ScalarKind::FLOAT;
        case daxa::Format::S8_UINT: return ScalarKind::UINT;
        case daxa::Format::D16_UNORM_S8_UINT: return ScalarKind::UINT;
        case daxa::Format::D24_UNORM_S8_UINT: return ScalarKind::UINT;
        case daxa::Format::D32_SFLOAT_S8_UINT: return ScalarKind::UINT;
        case daxa::Format::BC1_RGB_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::BC1_RGB_SRGB_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::BC1_RGBA_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::BC1_RGBA_SRGB_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::BC2_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::BC2_SRGB_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::BC3_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::BC3_SRGB_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::BC4_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::BC4_SNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::BC5_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::BC5_SNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::BC6H_UFLOAT_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::BC6H_SFLOAT_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::BC7_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::BC7_SRGB_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ETC2_R8G8B8_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ETC2_R8G8B8_SRGB_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ETC2_R8G8B8A1_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ETC2_R8G8B8A1_SRGB_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ETC2_R8G8B8A8_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ETC2_R8G8B8A8_SRGB_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::EAC_R11_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::EAC_R11_SNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::EAC_R11G11_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::EAC_R11G11_SNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_4x4_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_4x4_SRGB_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_5x4_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_5x4_SRGB_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_5x5_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_5x5_SRGB_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_6x5_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_6x5_SRGB_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_6x6_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_6x6_SRGB_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_8x5_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_8x5_SRGB_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_8x6_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_8x6_SRGB_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_8x8_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_8x8_SRGB_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_10x5_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_10x5_SRGB_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_10x6_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_10x6_SRGB_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_10x8_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_10x8_SRGB_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_10x10_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_10x10_SRGB_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_12x10_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_12x10_SRGB_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_12x12_UNORM_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_12x12_SRGB_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::G8B8G8R8_422_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::B8G8R8G8_422_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::G8_B8_R8_3PLANE_420_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::G8_B8R8_2PLANE_420_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::G8_B8_R8_3PLANE_422_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::G8_B8R8_2PLANE_422_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::G8_B8_R8_3PLANE_444_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::R10X6_UNORM_PACK16: return ScalarKind::FLOAT;
        case daxa::Format::R10X6G10X6_UNORM_2PACK16: return ScalarKind::FLOAT;
        case daxa::Format::R10X6G10X6B10X6A10X6_UNORM_4PACK16: return ScalarKind::FLOAT;
        case daxa::Format::G10X6B10X6G10X6R10X6_422_UNORM_4PACK16: return ScalarKind::FLOAT;
        case daxa::Format::B10X6G10X6R10X6G10X6_422_UNORM_4PACK16: return ScalarKind::FLOAT;
        case daxa::Format::G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16: return ScalarKind::FLOAT;
        case daxa::Format::G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16: return ScalarKind::FLOAT;
        case daxa::Format::G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16: return ScalarKind::FLOAT;
        case daxa::Format::G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16: return ScalarKind::FLOAT;
        case daxa::Format::G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16: return ScalarKind::FLOAT;
        case daxa::Format::R12X4_UNORM_PACK16: return ScalarKind::FLOAT;
        case daxa::Format::R12X4G12X4_UNORM_2PACK16: return ScalarKind::FLOAT;
        case daxa::Format::R12X4G12X4B12X4A12X4_UNORM_4PACK16: return ScalarKind::FLOAT;
        case daxa::Format::G12X4B12X4G12X4R12X4_422_UNORM_4PACK16: return ScalarKind::FLOAT;
        case daxa::Format::B12X4G12X4R12X4G12X4_422_UNORM_4PACK16: return ScalarKind::FLOAT;
        case daxa::Format::G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16: return ScalarKind::FLOAT;
        case daxa::Format::G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16: return ScalarKind::FLOAT;
        case daxa::Format::G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16: return ScalarKind::FLOAT;
        case daxa::Format::G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16: return ScalarKind::FLOAT;
        case daxa::Format::G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16: return ScalarKind::FLOAT;
        case daxa::Format::G16B16G16R16_422_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::B16G16R16G16_422_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::G16_B16_R16_3PLANE_420_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::G16_B16R16_2PLANE_420_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::G16_B16_R16_3PLANE_422_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::G16_B16R16_2PLANE_422_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::G16_B16_R16_3PLANE_444_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::G8_B8R8_2PLANE_444_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16: return ScalarKind::FLOAT;
        case daxa::Format::G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16: return ScalarKind::FLOAT;
        case daxa::Format::G16_B16R16_2PLANE_444_UNORM: return ScalarKind::FLOAT;
        case daxa::Format::A4R4G4B4_UNORM_PACK16: return ScalarKind::FLOAT;
        case daxa::Format::A4B4G4R4_UNORM_PACK16: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_4x4_SFLOAT_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_5x4_SFLOAT_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_5x5_SFLOAT_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_6x5_SFLOAT_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_6x6_SFLOAT_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_8x5_SFLOAT_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_8x6_SFLOAT_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_8x8_SFLOAT_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_10x5_SFLOAT_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_10x6_SFLOAT_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_10x8_SFLOAT_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_10x10_SFLOAT_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_12x10_SFLOAT_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::ASTC_12x12_SFLOAT_BLOCK: return ScalarKind::FLOAT;
        case daxa::Format::PVRTC1_2BPP_UNORM_BLOCK_IMG: return ScalarKind::FLOAT;
        case daxa::Format::PVRTC1_4BPP_UNORM_BLOCK_IMG: return ScalarKind::FLOAT;
        case daxa::Format::PVRTC2_2BPP_UNORM_BLOCK_IMG: return ScalarKind::FLOAT;
        case daxa::Format::PVRTC2_4BPP_UNORM_BLOCK_IMG: return ScalarKind::FLOAT;
        case daxa::Format::PVRTC1_2BPP_SRGB_BLOCK_IMG: return ScalarKind::FLOAT;
        case daxa::Format::PVRTC1_4BPP_SRGB_BLOCK_IMG: return ScalarKind::FLOAT;
        case daxa::Format::PVRTC2_2BPP_SRGB_BLOCK_IMG: return ScalarKind::FLOAT;
        case daxa::Format::PVRTC2_4BPP_SRGB_BLOCK_IMG: return ScalarKind::FLOAT;
        case daxa::Format::MAX_ENUM: return ScalarKind::FLOAT;
        }
        return ScalarKind::FLOAT;
    }

    auto is_format_depth_stencil(daxa::Format format) -> bool
    {
        switch (format)
        {
        case daxa::Format::D16_UNORM: return true;
        case daxa::Format::X8_D24_UNORM_PACK32: return true;
        case daxa::Format::D32_SFLOAT: return true;
        case daxa::Format::S8_UINT: return true;
        case daxa::Format::D16_UNORM_S8_UINT: return true;
        case daxa::Format::D24_UNORM_S8_UINT: return true;
        case daxa::Format::D32_SFLOAT_S8_UINT: return true;
        }
        return false;
    }

    auto resource_live_viewer_name(std::string_view resource_name, u32 version) -> std::string
    {
        return std::format("{}()", resource_name, version + 1);
    }

    void task_resource_viewer_debug_ui_hook(ImplTaskGraphDebugUi & context, ImplTaskGraph * impl, u32 task_index, daxa::TaskInterface & ti, bool is_pre_task)
    {
        DAXA_DBG_ASSERT_TRUE_M(context.resource_viewer_states.size() > 0, "IMPOSSIBLE CASE! Why are we calling this callback when attachment views are empty??");
        for (u32 attach_i = 0; attach_i < ti.attachment_infos.size(); ++attach_i)
        {
            ImplTask const & task = impl->tasks[task_index];
            ImplTaskResource const * resource = task.attachment_resources[attach_i].first;
            if (resource == nullptr)
            {
                continue;
            }

            TaskAttachmentInfo const & attachment_info = ti.attachment_infos[attach_i];

            if (!context.resource_viewer_states.contains(std::string(resource->name)))
            {
                continue;
            }

            ResourceViewerState & state = context.resource_viewer_states.at(std::string(resource->name));

            /// ===================
            /// ==== PRE CLEAR ====
            /// ===================

            bool const is_first_access = task.attachment_access_groups[attach_i].first == &resource->access_timeline[0];
            if (is_pre_task && state.clear_before_task && is_first_access)
            {
                if (attachment_info.type == TaskAttachmentType::BUFFER)
                {
                    BufferId buffer_id = ti.id(TaskBufferAttachmentIndex{attach_i});
                    {
                        ti.recorder.pipeline_barrier(daxa::BarrierInfo{
                            .src_access = daxa::AccessConsts::READ_WRITE,
                            .dst_access = daxa::AccessConsts::READ_WRITE,
                        });
                        ti.recorder.clear_buffer({.buffer = buffer_id});
                        ti.recorder.pipeline_barrier(daxa::BarrierInfo{
                            .src_access = daxa::AccessConsts::READ_WRITE,
                            .dst_access = daxa::AccessConsts::READ_WRITE,
                        });
                    }
                }
                if (attachment_info.type == TaskAttachmentType::IMAGE)
                {
                    ImageId image_id = ti.id(TaskImageAttachmentIndex{attach_i});
                    {
                        ti.recorder.pipeline_barrier(daxa::BarrierInfo{
                            .src_access = daxa::AccessConsts::READ_WRITE,
                            .dst_access = daxa::AccessConsts::READ_WRITE,
                        });
                        ti.recorder.clear_image({.dst_image = image_id});
                        ti.recorder.pipeline_barrier(daxa::BarrierInfo{
                            .src_access = daxa::AccessConsts::READ_WRITE,
                            .dst_access = daxa::AccessConsts::READ_WRITE,
                        });
                    }
                }
            }

            if (state.pre_task != is_pre_task)
            {
                continue;
            }

            if (task.attachment_access_groups[attach_i].first != &resource->access_timeline[state.timeline_index])
            {
                continue;
            }

            /// ==============================
            /// ==== HOOK BUFFER READBACK ====
            /// ==============================

            if (attachment_info.type == TaskAttachmentType::BUFFER)
            {
                BufferId buffer_id = ti.id(TaskBufferAttachmentIndex{attach_i});
                BufferInfo buffer_info = ti.info(TaskBufferAttachmentIndex{attach_i}).value();

                if (!state.freeze_resource)
                {
                    /// ===============================
                    /// ==== MAINTAIN CLONE BUFFER ====
                    /// ===============================

                    // Deferred destroy the clone buffer of last frame
                    if (!state.buffer.destroy_clone_buffer.is_empty())
                    {
                        context.device.destroy_buffer(state.buffer.destroy_clone_buffer);
                        state.buffer.destroy_clone_buffer = {};
                    }

                    // We technically race here.
                    // I dont care. Its only copied to be displayed later.
                    // Its not important if the data is mixed of two or more frames while reading back.
                    bool recreate_clone_buffer = true;
                    if (!state.buffer.clone_buffer.is_empty())
                    {
                        BufferInfo clone_buffer_info = context.device.buffer_info(state.buffer.clone_buffer).value();
                        recreate_clone_buffer = clone_buffer_info.size != (buffer_info.size);
                    }
                    if (recreate_clone_buffer)
                    {
                        if (!state.buffer.clone_buffer.is_empty())
                        {
                            state.buffer.destroy_clone_buffer = state.buffer.clone_buffer;
                        }
                        state.buffer.clone_buffer = context.device.create_buffer({
                            .size = buffer_info.size,
                            .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE, // Want it in vram
                            .name = std::format("{}::viewer::clone_buffer", resource->name),
                        });
                    }

                    ti.recorder.pipeline_barrier(daxa::BarrierInfo{
                        .src_access = daxa::AccessConsts::READ_WRITE,
                        .dst_access = daxa::AccessConsts::TRANSFER_READ,
                    });

                    ti.recorder.copy_buffer_to_buffer({
                        .src_buffer = buffer_id,
                        .dst_buffer = state.buffer.clone_buffer,
                        .src_offset = {},
                        .dst_offset = {},
                        .size = buffer_info.size,
                    });

                    for (auto & child_viewer : state.buffer.child_buffer_inspectors)
                    {
                        if (child_viewer.second.buffer.is_empty())
                        {
                            continue;
                        }

                        /// ==================================
                        /// ==== EXTEND RESOURCE LIFETIME ====
                        /// ==================================

                        {
                            // Attempt to increment reference counter.
                            bool success = context.device.inc_refcnt_buffer(child_viewer.second.buffer);
                            if (!success)
                            {
                                continue;
                            }

                            // This will decrement the refcount once the command buffer finished on the gpu.
                            ti.recorder.destroy_buffer_deferred(child_viewer.second.buffer);
                        }

                        BufferInfo child_buffer_info = context.device.buffer_info(child_viewer.second.buffer).value();

                        /// ===============================
                        /// ==== MAINTAIN CLONE BUFFER ====
                        /// ===============================

                        // Deferred destroy the clone buffer of last frame
                        if (!child_viewer.second.destroy_clone_buffer.is_empty())
                        {
                            context.device.destroy_buffer(child_viewer.second.destroy_clone_buffer);
                            child_viewer.second.destroy_clone_buffer = {};
                        }

                        // We technically race here.
                        // I dont care. Its only copied to be displayed later.
                        // Its not important if the data is mixed of two or more frames while reading back.
                        bool recreate_clone_buffer = true;
                        if (!child_viewer.second.clone_buffer.is_empty())
                        {
                            BufferInfo clone_buffer_info = context.device.buffer_info(child_viewer.second.clone_buffer).value();
                            recreate_clone_buffer = clone_buffer_info.size != (child_buffer_info.size);
                        }
                        if (recreate_clone_buffer)
                        {
                            if (!child_viewer.second.clone_buffer.is_empty())
                            {
                                child_viewer.second.destroy_clone_buffer = child_viewer.second.clone_buffer;
                            }
                            child_viewer.second.clone_buffer = context.device.create_buffer({
                                .size = child_buffer_info.size,
                                .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE, // Want it in vram
                                .name = std::format("{}::viewer::clone_buffer", child_buffer_info.name.c_str()),
                            });
                        }

                        ti.recorder.copy_buffer_to_buffer({
                            .src_buffer = child_viewer.second.buffer,
                            .dst_buffer = child_viewer.second.clone_buffer,
                            .src_offset = {},
                            .dst_offset = {},
                            .size = child_buffer_info.size,
                        });
                    }

                    ti.recorder.pipeline_barrier(daxa::BarrierInfo{
                        .src_access = daxa::AccessConsts::TRANSFER_WRITE,
                        .dst_access = daxa::AccessConsts::HOST_READ,
                    });
                }
            }

            if (attachment_info.type == TaskAttachmentType::IMAGE)
            {
                ImageId image_id = ti.id(TaskImageAttachmentIndex{attach_i});
                ImageInfo image_info = ti.info(TaskImageAttachmentIndex{attach_i}).value();
                u32 const display_image_x = std::max(1u, static_cast<u32>(image_info.size.x >> state.image.mip));
                u32 const display_image_y = std::max(1u, static_cast<u32>(image_info.size.y >> state.image.mip));

                /// ================================
                /// ==== MAINTAIN DISPLAY IMAGE ====
                /// ================================

                if (!state.image.destroy_display_image.is_empty())
                {
                    context.device.destroy_image(state.image.destroy_display_image);
                    state.image.destroy_display_image = {};
                }

                bool create_display_image = true;
                auto display_image_info_optional = ti.device.image_info(state.image.display_image);
                if (display_image_info_optional.has_value())
                {
                    ImageInfo const & display_image_info = display_image_info_optional.value();
                    bool const images_match =
                        display_image_info.size.x == display_image_x &&
                        display_image_info.size.y == display_image_y;
                    create_display_image = !images_match;
                    if (create_display_image)
                    {
                        state.image.destroy_display_image = state.image.display_image;
                        state.image.display_image = {};
                    }
                }
                if (create_display_image)
                {
                    ImageInfo display_image_info = {};
                    display_image_info.dimensions = 2u;
                    display_image_info.size.x = display_image_x;
                    display_image_info.size.y = display_image_y;
                    display_image_info.size.z = 1u;
                    display_image_info.mip_level_count = 1u;
                    display_image_info.array_layer_count = 1u;
                    display_image_info.sample_count = 1u;
                    display_image_info.format = daxa::Format::R16G16B16A16_SFLOAT;
                    display_image_info.name = std::format("{}::viewer::display_image", resource->name);
                    display_image_info.usage = daxa::ImageUsageFlagBits::SHADER_SAMPLED | daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_DST;

                    state.image.display_image = ti.device.create_image(display_image_info);
                }

                /// ==============================
                /// ==== MAINTAIN CLONE IMAGE ====
                /// ==============================

                if (!state.image.destroy_clone_image.is_empty())
                {
                    context.device.destroy_image(state.image.destroy_clone_image);
                    state.image.destroy_clone_image = {};
                }

                bool create_clone_image = true;
                auto clone_image_info_optional = ti.device.image_info(state.image.clone_image);
                if (clone_image_info_optional.has_value())
                {
                    ImageInfo const & clone_image_info = clone_image_info_optional.value();
                    auto const images_match =
                        clone_image_info.size.x == image_info.size.x &&
                        clone_image_info.size.y == image_info.size.y &&
                        clone_image_info.size.z == image_info.size.z &&
                        clone_image_info.mip_level_count == image_info.mip_level_count &&
                        clone_image_info.array_layer_count == image_info.array_layer_count &&
                        clone_image_info.format == image_info.format;
                    create_clone_image = !images_match;
                    if (create_clone_image)
                    {
                        state.image.destroy_clone_image = state.image.clone_image;
                        state.image.clone_image = {};
                    }
                }
                if (create_clone_image)
                {
                    ImageInfo clone_image_info = image_info;

                    if (clone_image_info.format == daxa::Format::D32_SFLOAT)
                    {
                        clone_image_info.format = daxa::Format::R32_SFLOAT;
                    }
                    if (clone_image_info.format == daxa::Format::D16_UNORM)
                    {
                        clone_image_info.format = daxa::Format::R16_UNORM;
                    }
                    // STORAGE is better than SAMPLED as it supports 64bit images.
                    clone_image_info.usage = daxa::ImageUsageFlagBits::TRANSFER_DST | daxa::ImageUsageFlagBits::SHADER_STORAGE;
                    clone_image_info.name = std::format("{}::viewer::clone_image", resource->name);

                    state.image.clone_image = ti.device.create_image(clone_image_info);
                }

                /// =========================
                /// ==== COPY INTO CLONE ====
                /// =========================

                if (!state.freeze_resource)
                {
                    auto slice = daxa::ImageMipArraySlice{
                        .level_count = image_info.mip_level_count,
                        .layer_count = image_info.array_layer_count,
                    };

                    ti.recorder.pipeline_barrier(daxa::BarrierInfo{
                        .src_access = daxa::AccessConsts::READ_WRITE,
                        .dst_access = daxa::AccessConsts::READ_WRITE,
                    });

                    // Copy src image data to frozen image.
                    for (daxa::u32 mip = slice.base_mip_level; mip < (slice.base_mip_level + slice.level_count); ++mip)
                    {
                        ti.recorder.copy_image_to_image({
                            .src_image = image_id,
                            .dst_image = state.image.clone_image,
                            .src_slice = daxa::ImageArraySlice::slice(slice, mip),
                            .dst_slice = daxa::ImageArraySlice::slice(slice, mip),
                            .extent = {
                                std::max(1u, image_info.size.x >> mip),
                                std::max(1u, image_info.size.y >> mip),
                                std::max(1u, image_info.size.z >> mip)},
                        });
                    }

                    ti.recorder.pipeline_barrier(daxa::BarrierInfo{
                        .src_access = daxa::AccessConsts::TRANSFER_WRITE,
                        .dst_access = daxa::AccessConsts::COMPUTE_SHADER_READ,
                    });
                }

                /// ===============================================
                /// ==== DISPATCH IMAGE RESOURCE VIEWER SHADER ====
                /// ===============================================

                {
                    ti.recorder.set_pipeline(*context.resource_viewer_pipeline);

                    u32 const readback_index = (state.open_frame_count) % READBACK_CIRCULAR_BUFFER_SIZE;
                    ImageInfo clone_image_info = ti.device.image_info(state.image.clone_image).value();
                    ImageInfo display_image_info = ti.device.image_info(state.image.display_image).value();
                    ImageViewInfo clone_image_view_info = ti.device.image_view_info(state.image.clone_image.default_view()).value();
                    clone_image_view_info.slice.level_count = 1;
                    clone_image_view_info.slice.layer_count = 1;
                    clone_image_view_info.slice.base_mip_level = state.image.mip;
                    clone_image_view_info.slice.base_array_layer = state.image.layer;
                    ImageViewId src_view = ti.device.create_image_view(clone_image_view_info);
                    ti.recorder.destroy_image_view_deferred(src_view);
                    ti.recorder.push_constant(TaskGraphDebugUiPush{
                        .src = src_view,
                        .dst = state.image.display_image.default_view(),
                        .display_image_size = {clone_image_info.size.x, clone_image_info.size.y},
                        .image_view_type = static_cast<u32>(clone_image_view_info.type),
                        .format = static_cast<i32>(scalar_kind_of_format(image_info.format)),
                        .display_min = std::bit_cast<u32>(state.image.min_display_value),
                        .display_max = std::bit_cast<u32>(state.image.max_display_value),
                        .rainbow_ints = state.image.rainbow_ints,
                        .gamma_correct = state.image.gamma_correct ? 1 : 0,
                        .enabled_channels = state.image.enabled_channels,
                        .mouse_over_index = {
                            state.image.mouse_texel_index.x, // >> state.mip,
                            state.image.mouse_texel_index.y, // >> state.mip,
                        },
                        .readback_ptr = ti.device.device_address(state.image.readback_buffer).value() + readback_index * sizeof(TaskGraphDebugUiImageReadbackStruct),
                    });
                    ti.recorder.dispatch({
                        (display_image_info.size.x + TASK_GRAPH_DEBUG_UI_X - 1) / TASK_GRAPH_DEBUG_UI_X,
                        (display_image_info.size.y + TASK_GRAPH_DEBUG_UI_Y - 1) / TASK_GRAPH_DEBUG_UI_Y,
                        1,
                    });
                }
            }
        }
    }

    struct CompileBufferStructFormatResult
    {
        std::vector<TgDebugTypeDefinition> type_definitions = {};
        std::string error_message = {};
        bool success = {};
    };
    auto compile_buffer_struct_format(std::string_view const & code) -> CompileBufferStructFormatResult
    {
        auto parse_vector_matrix_type = [](std::string_view type_str) -> std::tuple<std::string_view, i32, i32>
        {
            // Returns: (base_type, rows, cols) where cols=1 for vectors
            auto vec_pos = type_str.find("vec");
            auto mat_pos = type_str.find("mat");
            if (vec_pos != std::string_view::npos)
            {
                auto base_type = type_str.substr(0, vec_pos);
                auto dim_str = type_str.substr(vec_pos + 3);
                if (dim_str.size() > 0 && std::isdigit(dim_str[0]))
                {
                    char* end;
                    long dim = std::strtol(dim_str.data(), &end, 10);
                    if (dim > 0 && dim <= 4)
                        return std::make_tuple(base_type, static_cast<i32>(dim), 1);
                }
            }
            else if (mat_pos != std::string_view::npos)
            {
                auto base_type = type_str.substr(0, mat_pos);
                auto dims_str = type_str.substr(mat_pos + 3);
                char* end;
                long rows = std::strtol(dims_str.data(), &end, 10);
                if (rows > 0 && rows <= 4)
                {
                    long cols = rows;  // Default: square matrix (matN means matNxN)
                    
                    // Check if there's an explicit 'x' separator for non-square matrices
                    if (end[0] == 'x')
                    {
                        cols = std::strtol(end + 1, &end, 10);
                        if (cols <= 0 || cols > 4)
                            return std::make_tuple(type_str, 0, 0);
                    }
                    
                    return std::make_tuple(base_type, static_cast<i32>(rows), static_cast<i32>(cols));
                }
            }
            return std::make_tuple(type_str, 0, 0);
        };
        
        static const std::array type_name_mappings = std::array{
            std::pair(std::hash<std::string_view>{}("f16"), TG_DEBUG_PRIMITIVE_TYPE_INDEX_F16),
            std::pair(std::hash<std::string_view>{}("half"), TG_DEBUG_PRIMITIVE_TYPE_INDEX_F16),
            std::pair(std::hash<std::string_view>{}("f32"), TG_DEBUG_PRIMITIVE_TYPE_INDEX_F32),
            std::pair(std::hash<std::string_view>{}("float"), TG_DEBUG_PRIMITIVE_TYPE_INDEX_F32),
            std::pair(std::hash<std::string_view>{}("f64"), TG_DEBUG_PRIMITIVE_TYPE_INDEX_F64),
            std::pair(std::hash<std::string_view>{}("double"), TG_DEBUG_PRIMITIVE_TYPE_INDEX_F64),
            std::pair(std::hash<std::string_view>{}("i8"), TG_DEBUG_PRIMITIVE_TYPE_INDEX_I8),
            std::pair(std::hash<std::string_view>{}("i16"), TG_DEBUG_PRIMITIVE_TYPE_INDEX_I16),
            std::pair(std::hash<std::string_view>{}("short"), TG_DEBUG_PRIMITIVE_TYPE_INDEX_I16),
            std::pair(std::hash<std::string_view>{}("i32"), TG_DEBUG_PRIMITIVE_TYPE_INDEX_I32),
            std::pair(std::hash<std::string_view>{}("int"), TG_DEBUG_PRIMITIVE_TYPE_INDEX_I32),
            std::pair(std::hash<std::string_view>{}("i64"), TG_DEBUG_PRIMITIVE_TYPE_INDEX_I64),
            std::pair(std::hash<std::string_view>{}("u8"), TG_DEBUG_PRIMITIVE_TYPE_INDEX_U8),
            std::pair(std::hash<std::string_view>{}("byte"), TG_DEBUG_PRIMITIVE_TYPE_INDEX_U8),
            std::pair(std::hash<std::string_view>{}("u16"), TG_DEBUG_PRIMITIVE_TYPE_INDEX_U16),
            std::pair(std::hash<std::string_view>{}("u32"), TG_DEBUG_PRIMITIVE_TYPE_INDEX_U32),
            std::pair(std::hash<std::string_view>{}("b32"), TG_DEBUG_PRIMITIVE_TYPE_INDEX_U32),
            std::pair(std::hash<std::string_view>{}("unsigned"), TG_DEBUG_PRIMITIVE_TYPE_INDEX_U32),
            std::pair(std::hash<std::string_view>{}("uint"), TG_DEBUG_PRIMITIVE_TYPE_INDEX_U32),
            std::pair(std::hash<std::string_view>{}("u64"), TG_DEBUG_PRIMITIVE_TYPE_INDEX_U64),
        };

        auto constexpr whitespace_characters = std::string_view({" " "\f" "\n" "\r" "\t" "\v"});
        // Helper lambda to skip whitespace and comments.
        auto jump_to_next_token = [&whitespace_characters](std::string_view const & in_string) -> std::string_view
        {
            std::string_view work_string = in_string;

            while (work_string.size() > 0)
            {
                auto const first_non_whitespace_char = work_string.find_first_not_of(whitespace_characters);
                work_string = work_string.substr(std::min(first_non_whitespace_char, work_string.size()));

                if(work_string.starts_with("//"))
                {
                    // Skip line comment
                    auto const line_end = work_string.find('\n');
                    work_string = work_string.substr(std::min(line_end + 1, work_string.size()));
                }
                else if (work_string.starts_with("/*"))
                {
                    // Skip block comment
                    work_string = work_string.substr(2);
                    auto const block_end = work_string.find("*/");
                    work_string = work_string.substr(std::min(block_end + 2, work_string.size()));
                }
                else { break; }
            }
            return work_string;
        };

        CompileBufferStructFormatResult result = {};
        auto work_code = code;
        std::vector<TgDebugTypeDefinition> parsed_type_defs = {};

        while(work_code.size() > 0 && work_code[0] != '\0')
        {
            work_code = jump_to_next_token(work_code);
            TgDebugTypeDefinition current_type_def = {};

            // Everything must begin with a struct keyword.
            if(! work_code.starts_with("struct"))
            {
                result.error_message = std::format("Expected 'struct' keyword got \"{}...\"", work_code.substr(0, std::min(work_code.size(), 20ull)));
                return result;
            }
            // Skip struct keyword.
            work_code = work_code.substr(6);

            work_code = jump_to_next_token(work_code);
            // Get struct name.
            auto const name_end = work_code.find_first_of(whitespace_characters);
            if(name_end == std::string_view::npos)
            {
                result.error_message = "Expected struct name after 'struct' keyword, got end of input.";
                return result;
            }
            auto const struct_name = work_code.substr(0, name_end);

            current_type_def.name = struct_name;

            work_code = work_code.substr(name_end);
            work_code = jump_to_next_token(work_code);
            // Expect opening brace.
            if(! work_code.starts_with("{"))
            {
                result.error_message = std::format("Expected '{{' after struct name '{}', got \"{}...\"", struct_name, work_code.substr(0, std::min(work_code.size(), 20ull)));
                return result;
            }
            work_code = work_code.substr(1);

            // Now we loop until we find the closing brace, adding members as we go.
            while(true)
            {
                work_code = jump_to_next_token(work_code);
                // Check for closing brace.
                if(work_code.starts_with("}"))
                {
                    work_code = work_code.substr(1);
                    if(!work_code.starts_with(";"))
                    {
                        result.error_message = std::format("Expected ';' after closing '}}' of struct '{}', got \"{}...\"", struct_name, work_code.substr(0, std::min(work_code.size(), 20ull)));
                        return result;
                    }
                    work_code = work_code.substr(1);
                    parsed_type_defs.push_back(current_type_def);
                    break;
                }

                // Get member type.
                auto const type_end = work_code.find_first_of(std::string(whitespace_characters) + "*");
                if(type_end == std::string_view::npos)
                {
                    result.error_message = std::format("Expected member type in struct '{}', got end of input.", struct_name);
                    return result;
                }

                auto member_type = work_code.substr(0, type_end);

                i32 pointer_depth = 0;

                // Remove daxa_ prefix for the BufferPtr and RWBufferPtr handle.
                if(member_type.starts_with("daxa_"))
                {
                    member_type.remove_prefix(5);
                }

                // Post process member type currently to look for daxa Buffer ptr
                if(member_type.starts_with("BufferPtr(") || member_type.starts_with("RWBufferPtr("))
                {
                    pointer_depth = 1;
                    if(!member_type.ends_with(")"))
                    {
                        result.error_message = std::format("Expected closing ')' for buffer pointer type in struct '{}', got \"{}...\"", struct_name, work_code.substr(0, std::min(work_code.size(), 20ull)));
                        return result;
                    }
                    member_type.remove_prefix(member_type.find('(') + 1);
                    member_type.remove_suffix(1);
                }

                // Post process member type to remove daxa_ prefix
                if(member_type.starts_with("daxa_"))
                {
                    member_type.remove_prefix(5);
                }


                // Check if this is a vector or matrix type
                auto [base_type, vec_dim, mat_cols] = parse_vector_matrix_type(member_type);
                i32 vec_array_multiplier = 1;
                std::string_view lookup_type = member_type;
                
                if (vec_dim > 0)
                {
                    lookup_type = base_type;
                    vec_array_multiplier = vec_dim * mat_cols;
                }

                // First search through static types.
                u64 const member_type_hash = std::hash<std::string_view>{}(lookup_type);
                auto const found_iterator = std::find_if(type_name_mappings.begin(), type_name_mappings.end(),
                    [&member_type_hash](const auto & pair){ return pair.first == member_type_hash; });

                i32 current_type = std::numeric_limits<i32>::max();
                // If not found, we need to search through previously defined structs.
                if(found_iterator == type_name_mappings.end())
                {
                    auto const user_type_iterator = std::find_if(
                        parsed_type_defs.begin(),
                        parsed_type_defs.end(),
                        [&lookup_type](const TgDebugTypeDefinition & def) { return def.name == lookup_type; });
                    if(user_type_iterator == parsed_type_defs.end())
                    {
                        result.error_message = std::format("Unknown member type '{}' in struct '{}'.", member_type, struct_name);
                        return result;
                    }
                    current_type = static_cast<i32>(std::distance(parsed_type_defs.begin(), user_type_iterator));
                }
                else
                {
                    current_type = found_iterator->second;
                }

                work_code = work_code.substr(type_end);
                work_code = jump_to_next_token(work_code);

                // Handle pointer.
                while(work_code.starts_with("*"))
                {
                    pointer_depth++;
                    work_code = work_code.substr(1);
                    work_code = jump_to_next_token(work_code);
                }

                auto const field_name_end = work_code.find_first_of(std::string(whitespace_characters) + ";" + "[");
                auto const member_name = work_code.substr(0, field_name_end);
                work_code = work_code.substr(field_name_end);

                // Handle array(s) - support for multidimensional arrays
                i32 array_size = vec_array_multiplier;
                if(work_code.starts_with("["))
                {
                    while(work_code.starts_with("["))
                    {
                        auto const array_end = work_code.find("]");
                        if(array_end == std::string_view::npos)
                        {
                            result.error_message = std::format("Expected closing ']' for array member '{}' in struct '{}', got end of input.", member_name, struct_name);
                            return result;
                        }

                        auto const array_size_str = work_code.substr(1, array_end - 1);
                        if(array_size_str.empty())
                        {
                            result.error_message = std::format("Expected array size for member '{}' in struct '{}', got empty size.", member_name, struct_name);
                            return result;
                        }

                        i32 dim_size = 0;
                        for(u32 char_idx = 0; char_idx < array_size_str.size(); ++char_idx)
                        {
                            if(!std::isdigit(array_size_str[char_idx]))
                            {
                                result.error_message = std::format("Invalid character '{}' in array size for member '{}' in struct '{}'.", array_size_str[char_idx], member_name, struct_name);
                                return result;
                            }
                            dim_size = dim_size * 10 + static_cast<u32>(array_size_str[char_idx] - '0');
                        }
                        
                        if (dim_size <= 0)
                        {
                            result.error_message = std::format("Array dimension must be positive for member '{}' in struct '{}', got {}.", member_name, struct_name, dim_size);
                            return result;
                        }
                        
                        array_size *= dim_size;

                        work_code = work_code.substr(array_end + 1);
                        work_code = jump_to_next_token(work_code);
                    }
                }

                if(!work_code.starts_with(";"))
                {
                    result.error_message = std::format("Expected ';' after member declaration of '{}' in struct '{}', got \"{}...\"", member_name, struct_name, work_code.substr(0, std::min(work_code.size(), 20ull)));
                    return result;
                }
                work_code = work_code.substr(1);

                current_type_def.fields.push_back(TgDebugStructFieldDefinition{
                    .name = std::string(member_name),
                    .type_index = current_type,
                    .array_size = array_size,
                    .pointer_depth = pointer_depth,
                });
            }
        }
        result.success = true;
        result.type_definitions = std::move(parsed_type_defs);
        return result;
    }

    auto resource_viewer_ui(
        ImplTaskGraphDebugUi & context,
        ImplTaskGraph * impl,
        std::string const & resource_name,
        ResourceViewerState & state) -> bool
    {
        if (!impl->name_to_resource_table.contains(resource_name))
        {
            return false;
        }
        ImplTaskResource const & resource = *impl->name_to_resource_table.at(resource_name).first;
        bool open = true;

        auto viewer_shared_header = [&]()
        {
            ImGui::Checkbox("PopOut", &state.free_window_next_frame);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(80);
            ImGui::SliderInt("Access", &state.timeline_index, 0, static_cast<i32>(resource.access_timeline.size() - 1));
            ImGui::SameLine();
            ImGui::Checkbox("Before", reinterpret_cast<bool *>(&state.pre_task));
            ImGui::SameLine();
            ImGui::Checkbox("Freeze", reinterpret_cast<bool *>(&state.freeze_resource));
            ImGui::SameLine();
            ImGui::Checkbox("PreClear", reinterpret_cast<bool *>(&state.clear_before_task));
        };

        bool begin_return = true;
        if (resource.kind == TaskResourceKind::IMAGE)
        {
            if (state.free_window)
            {
                constexpr u32 header_height = 100;
                constexpr u32 popout_default_width = 500;
                constexpr u32 default_popout_height = 380;

                f32 window_height = default_popout_height;
                if (context.device.is_id_valid(state.image.clone_image) && state.open_frame_count > READBACK_CIRCULAR_BUFFER_SIZE)
                {
                    ImageInfo clone_image_info = context.device.image_info(state.image.clone_image).value();
                    f32 const clone_image_aspect_ratio = static_cast<f32>(clone_image_info.size.x) / static_cast<f32>(clone_image_info.size.y);
                    f32 const desired_image_height = popout_default_width * (1.0f / clone_image_aspect_ratio);
                    window_height = desired_image_height + header_height;
                }
                ImGui::SetNextWindowSize(ImVec2(popout_default_width, window_height), ImGuiCond_FirstUseEver);
                ImGui::Begin(std::format("{}::Viewer", resource.name).c_str(), &open, ImGuiWindowFlags_NoSavedSettings);
                begin_return = open;
            }

            if (begin_return)
            {
                if (context.device.is_id_valid(state.image.clone_image) && state.open_frame_count > READBACK_CIRCULAR_BUFFER_SIZE)
                {
                    ImageInfo clone_image_info = context.device.image_info(state.image.clone_image).value();
                    u32 size_x = clone_image_info.size.x >> state.image.mip;
                    u32 size_y = clone_image_info.size.y >> state.image.mip;

                    u32 const channel_count = channel_count_of_format(clone_image_info.format);

                    if (channel_count < 4)
                    {
                        state.image.enabled_channels.w = 0;
                    }
                    if (channel_count < 3)
                    {
                        state.image.enabled_channels.z = 0;
                    }
                    if (channel_count < 2)
                    {
                        state.image.enabled_channels.y = 0;
                    }

                    viewer_shared_header();
                    ImGui::SetNextItemWidth(80);
                    ImGui::InputInt("Mip", &state.image.mip);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(80);
                    ImGui::InputInt("Layer", &state.image.layer);

                    auto draw_color_checkbox = [&](char const * id, ImVec4 color, i32 & checked, bool enabled)
                    {
                        float const hovered_extra = 0.3f;
                        float const active_extra = 0.2f;
                        float const normal_downscale = 0.8f;
                        ImVec4 bg_color = color;
                        bg_color.x *= normal_downscale;
                        bg_color.y *= normal_downscale;
                        bg_color.z *= normal_downscale;
                        ImVec4 hovered_color = color;
                        hovered_color.x = std::min(hovered_color.x, hovered_color.x + hovered_extra);
                        hovered_color.y = std::min(hovered_color.y, hovered_color.y + hovered_extra);
                        hovered_color.z = std::min(hovered_color.z, hovered_color.z + hovered_extra);
                        ImVec4 active_color = color;
                        active_color.x = std::min(active_color.x, active_color.x + active_extra);
                        active_color.y = std::min(active_color.y, active_color.y + active_extra);
                        active_color.z = std::min(active_color.z, active_color.z + active_extra);
                        ImGui::PushStyleColor(ImGuiCol_FrameBg, bg_color);
                        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, hovered_color);
                        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, active_color);
                        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0, 0, 0, 1));
                        ImGui::BeginDisabled(!enabled);
                        ImGui::Checkbox(id, reinterpret_cast<bool *>(&checked));
                        ImGui::EndDisabled();
                        ImGui::PopStyleColor(4);
                    };

                    ImGui::SameLine();
                    draw_color_checkbox("##R", ColorPalette::RED, state.image.enabled_channels.x, channel_count > 0);
                    ImGui::SameLine();
                    draw_color_checkbox("##G", ColorPalette::GREEN, state.image.enabled_channels.y, channel_count > 1);
                    ImGui::SameLine();
                    draw_color_checkbox("##B", ColorPalette::BLUE, state.image.enabled_channels.z, channel_count > 2);
                    ImGui::SameLine();
                    draw_color_checkbox("##A", ColorPalette::GREY, state.image.enabled_channels.w, channel_count > 2);

                    ImGui::SameLine(0, 10);
                    ImGui::Text("|");
                    ImGui::SameLine();
                    ImGui::Checkbox("Srgb", &state.image.gamma_correct);
                    ImGui::SameLine();
                    ImGui::Checkbox("Hex", &state.display_as_hexadecimal);

                    f64 min_value = {};
                    f64 max_value = {};
                    bool const neg_value_written = state.image.latest_readback.neg_min_value != ~0u;
                    bool const pos_value_written = state.image.latest_readback.pos_min_value != ~0u;
                    switch (scalar_kind_of_format(clone_image_info.format))
                    {
                    case ScalarKind::FLOAT:
                        min_value = std::min(
                            neg_value_written ? -std::bit_cast<f32>(state.image.latest_readback.neg_max_value) : std::numeric_limits<f32>::max(),
                            pos_value_written ? +std::bit_cast<f32>(state.image.latest_readback.pos_min_value) : std::numeric_limits<f32>::max());
                        max_value = std::max(
                            neg_value_written ? -std::bit_cast<f32>(state.image.latest_readback.neg_min_value) : std::numeric_limits<f32>::lowest(),
                            pos_value_written ? +std::bit_cast<f32>(state.image.latest_readback.pos_max_value) : std::numeric_limits<f32>::lowest());
                        break;
                    case ScalarKind::INT:
                        min_value = std::min(
                            neg_value_written ? -std::bit_cast<i32>(state.image.latest_readback.neg_max_value) : std::numeric_limits<i32>::max(),
                            pos_value_written ? +std::bit_cast<i32>(state.image.latest_readback.pos_min_value) : std::numeric_limits<i32>::max());
                        max_value = std::max(
                            neg_value_written ? -std::bit_cast<i32>(state.image.latest_readback.neg_min_value) : std::numeric_limits<i32>::lowest(),
                            pos_value_written ? +std::bit_cast<i32>(state.image.latest_readback.pos_max_value) : std::numeric_limits<i32>::lowest());
                        break;
                    case ScalarKind::UINT:
                        DAXA_DBG_ASSERT_TRUE_M(!neg_value_written, "IMPOSSIBLE CASE! Negative values cannot be written for uints");
                        min_value = state.image.latest_readback.pos_min_value;
                        max_value = state.image.latest_readback.pos_max_value;
                        break;
                    }

                    if (state.display_as_hexadecimal)
                    {
                        u32 hex_min = {};
                        u32 hex_max = {};
                        switch (scalar_kind_of_format(clone_image_info.format))
                        {
                        case ScalarKind::FLOAT:
                        {
                            hex_min = std::bit_cast<u32>(static_cast<f32>(min_value));
                            hex_max = std::bit_cast<u32>(static_cast<f32>(max_value));
                            break;
                        }
                        case ScalarKind::INT:
                        {
                            hex_min = std::bit_cast<u32>(static_cast<i32>(min_value));
                            hex_max = std::bit_cast<u32>(static_cast<i32>(max_value));
                            break;
                        }
                        case ScalarKind::UINT:
                        {
                            hex_min = std::bit_cast<u32>(static_cast<u32>(min_value));
                            hex_max = std::bit_cast<u32>(static_cast<u32>(max_value));
                            break;
                        }
                        }
                        ImGui::Text(std::format("Min {:#010x} Max {:#010x}", hex_min, hex_max).c_str());
                    }
                    else
                    {
                        switch (scalar_kind_of_format(clone_image_info.format))
                        {
                        case ScalarKind::FLOAT:
                        {
                            ImGui::Text(std::format("Min {:<10.3} Max {:<10.3}", min_value, max_value).c_str());
                            break;
                        }
                        case ScalarKind::INT:
                        {
                            ImGui::Text(std::format("Min {:<10} Max {:<10}", static_cast<i32>(min_value), static_cast<i32>(max_value)).c_str());
                            break;
                        }
                        case ScalarKind::UINT:
                        {
                            ImGui::Text(std::format("Min {:<10} Max {:<10}", static_cast<u32>(min_value), static_cast<u32>(max_value)).c_str());
                            break;
                        }
                        }
                    }
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(80);
                    switch (scalar_kind_of_format(clone_image_info.format))
                    {
                    case ScalarKind::FLOAT:
                    {
                        f32 const min_f32 = static_cast<f32>(min_value);
                        f32 const max_f32 = static_cast<f32>(max_value);
                        if (!state.image.display_value_range_initialized)
                        {
                            if (max_value == 0.0 && min_value == 0.0)
                            {
                                max_value = 1.0;
                            }
                            state.image.min_display_value._f32 = static_cast<f32>(min_value);
                            state.image.max_display_value._f32 = static_cast<f32>(max_value);
                            state.image.display_value_range_initialized = true;
                        }
                        ImGui::SetNextItemWidth(160);
                        f64 values[2] = {static_cast<f64>(state.image.min_display_value._f32), static_cast<f64>(state.image.max_display_value._f32)};
                        f64 min_values[2] = {static_cast<f64>(min_f32), static_cast<f64>(min_f32)};
                        f64 max_values[2] = {static_cast<f64>(max_f32), static_cast<f64>(max_f32)};
                        ImGui::SliderScalarN("Display Range", ImGuiDataType_Double, values, 2, min_values, max_values);
                        state.image.min_display_value._f32 = static_cast<f32>(values[0]);
                        state.image.max_display_value._f32 = static_cast<f32>(values[1]);
                        break;
                    }
                    case ScalarKind::INT:
                    {
                        i32 const min_int = static_cast<i32>(min_value);
                        i32 const max_int = static_cast<i32>(max_value);
                        if (!state.image.display_value_range_initialized)
                        {
                            if (max_value == 0.0 && min_value == 0.0)
                            {
                                max_value = 1.0;
                            }
                            state.image.min_display_value._i32 = static_cast<i32>(min_value);
                            state.image.max_display_value._i32 = static_cast<i32>(max_value);
                            state.image.display_value_range_initialized = true;
                        }
                        ImGui::SetNextItemWidth(160);
                        i64 values[2] = {static_cast<i64>(state.image.min_display_value._i32), static_cast<i64>(state.image.max_display_value._i32)};
                        i64 min_values[2] = {static_cast<i64>(min_int), static_cast<i64>(min_int)};
                        i64 max_values[2] = {static_cast<i64>(max_int), static_cast<i64>(max_int)};
                        ImGui::SliderScalarN("Display Range", ImGuiDataType_S64, values, 2, min_values, max_values);
                        state.image.min_display_value._i32 = static_cast<i32>(values[0]);
                        state.image.max_display_value._i32 = static_cast<i32>(values[1]);
                        break;
                    }
                    case ScalarKind::UINT:
                    {
                        u32 const min_uint = static_cast<u32>(min_value);
                        u32 const max_uint = static_cast<u32>(max_value);
                        if (!state.image.display_value_range_initialized)
                        {
                            if (max_value == 0.0 && min_value == 0.0)
                            {
                                max_value = 1.0;
                            }
                            state.image.min_display_value._u32 = static_cast<u32>(min_value);
                            state.image.max_display_value._u32 = static_cast<u32>(max_value);
                            state.image.display_value_range_initialized = true;
                        }
                        ImGui::SetNextItemWidth(160);
                        u64 values[2] = {static_cast<u64>(state.image.min_display_value._u32), static_cast<u64>(state.image.max_display_value._u32)};
                        u64 min_values[2] = {static_cast<u64>(min_uint), static_cast<u64>(min_uint)};
                        u64 max_values[2] = {static_cast<u64>(max_uint), static_cast<u64>(max_uint)};
                        ImGui::SliderScalarN("Display Range", ImGuiDataType_U64, values, 2, min_values, max_values);
                        state.image.min_display_value._u32 = static_cast<u32>(values[0]);
                        state.image.max_display_value._u32 = static_cast<u32>(values[1]);
                        break;
                    }
                    }

                    state.image.mip = std::clamp(state.image.mip, 0, static_cast<i32>(clone_image_info.mip_level_count) - 1);
                    state.image.layer = std::clamp(state.image.layer, 0, static_cast<i32>(clone_image_info.array_layer_count) - 1);

                    if (!state.image.display_image.is_empty())
                    {
                        state.image.imgui_image_id = context.imgui_renderer.create_texture_id({
                            .image_view_id = state.image.display_image.default_view(),
                            .sampler_id = context.resource_viewer_sampler_id,
                        });
                        ImPlot::PushStyleVar(ImPlotStyleVar_PlotBorderSize, 2);
                        ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(0.0f, 0.0f));
                        ImPlot::PushStyleVar(ImPlotStyleVar_LabelPadding, ImVec2(0.0f, 0.0f));
                        f32 const aspect_ratio = static_cast<f32>(size_x) / static_cast<f32>(size_y);
                        ImVec2 const available_region = ImGui::GetContentRegionAvail();
                        f32 const wanted_height = available_region.x * (1.0f / aspect_ratio);
                        f32 const real_height = wanted_height > available_region.y ? available_region.y : wanted_height;
                        f32 const real_width = real_height * aspect_ratio;
                        ImPlotPoint mouse_pos = ImPlotPoint(std::numeric_limits<f64>::max(), std::numeric_limits<f64>::max());
                        if (ImPlot::BeginPlot("##image", ImVec2(real_width, real_height), ImPlotFlags_NoTitle | ImPlotFlags_NoLegend | ImPlotFlags_NoMouseText))
                        {
                            u32 const flags = ImPlotAxisFlags_NoDecorations & (~ImPlotAxisFlags_NoTickMarks);
                            ImPlot::SetupAxes("", "", flags, flags);
                            if (ImPlot::IsPlotHovered())
                            {
                                auto raw_mouse_pos = ImPlot::GetPlotMousePos(IMPLOT_AUTO, IMPLOT_AUTO);
                                mouse_pos = ImPlotPoint(raw_mouse_pos.x, 1.0f - raw_mouse_pos.y) * ImPlotPoint(size_x, size_y);
                            }

                            ImPlot::PlotImage("##my image", state.image.imgui_image_id, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
                            ImPlot::EndPlot();
                        }
                        state.image.mouse_texel_index = {
                            std::clamp(static_cast<i32>(mouse_pos.x), 0, static_cast<i32>(size_x) - 1),
                            std::clamp(static_cast<i32>(mouse_pos.y), 0, static_cast<i32>(size_y) - 1)};

                        if (ImGui::IsItemHovered())
                        {
                            ImGui::BeginTooltip();
                            ImGui::Text(std::format("{}, {}", static_cast<i32>(mouse_pos.x), static_cast<i32>(mouse_pos.y)).c_str());

                            if (state.display_as_hexadecimal)
                            {
                                if (state.image.enabled_channels.x)
                                {
                                    ImGui::Text(std::format("R {:#010x}", std::bit_cast<u32>(state.image.latest_readback.hovered_value.x)).c_str());
                                    ImGui::SameLine();
                                }
                                if (state.image.enabled_channels.y)
                                {
                                    ImGui::Text(std::format("G {:#010x}", std::bit_cast<u32>(state.image.latest_readback.hovered_value.y)).c_str());
                                    ImGui::SameLine();
                                }
                                if (state.image.enabled_channels.z)
                                {
                                    ImGui::Text(std::format("B {:#010x}", std::bit_cast<u32>(state.image.latest_readback.hovered_value.z)).c_str());
                                    ImGui::SameLine();
                                }
                                if (state.image.enabled_channels.w)
                                {
                                    ImGui::Text(std::format("A {:#010x}", std::bit_cast<u32>(state.image.latest_readback.hovered_value.w)).c_str());
                                    ImGui::SameLine();
                                }
                                ImGui::Dummy({});
                            }
                            else
                            {
                                switch (scalar_kind_of_format(clone_image_info.format))
                                {
                                case ScalarKind::FLOAT:
                                {
                                    if (state.image.enabled_channels.x)
                                    {
                                        ImGui::Text(std::format("R {:<10.3}", static_cast<f64>(std::bit_cast<f32>(state.image.latest_readback.hovered_value.x))).c_str());
                                        ImGui::SameLine();
                                    }
                                    if (state.image.enabled_channels.y)
                                    {
                                        ImGui::Text(std::format("G {:<10.3}", static_cast<f64>(std::bit_cast<f32>(state.image.latest_readback.hovered_value.y))).c_str());
                                        ImGui::SameLine();
                                    }
                                    if (state.image.enabled_channels.z)
                                    {
                                        ImGui::Text(std::format("B {:<10.3}", static_cast<f64>(std::bit_cast<f32>(state.image.latest_readback.hovered_value.z))).c_str());
                                        ImGui::SameLine();
                                    }
                                    if (state.image.enabled_channels.w)
                                    {
                                        ImGui::Text(std::format("A {:<10.3}", static_cast<f64>(std::bit_cast<f32>(state.image.latest_readback.hovered_value.w))).c_str());
                                    }
                                    ImGui::Dummy({});
                                    break;
                                }
                                case ScalarKind::INT:
                                {

                                    if (state.image.enabled_channels.x)
                                    {
                                        ImGui::Text(std::format("R {:<10}", std::bit_cast<i32>(state.image.latest_readback.hovered_value.x)).c_str());
                                        ImGui::SameLine();
                                    }
                                    if (state.image.enabled_channels.y)
                                    {
                                        ImGui::Text(std::format("G {:<10}", std::bit_cast<i32>(state.image.latest_readback.hovered_value.y)).c_str());
                                        ImGui::SameLine();
                                    }
                                    if (state.image.enabled_channels.z)
                                    {
                                        ImGui::Text(std::format("B {:<10}", std::bit_cast<i32>(state.image.latest_readback.hovered_value.z)).c_str());
                                        ImGui::SameLine();
                                    }
                                    if (state.image.enabled_channels.w)
                                    {
                                        ImGui::Text(std::format("A {:<10}", std::bit_cast<i32>(state.image.latest_readback.hovered_value.w)).c_str());
                                    }
                                    ImGui::Dummy({});
                                    break;
                                }
                                case ScalarKind::UINT:
                                {
                                    if (state.image.enabled_channels.x)
                                    {
                                        ImGui::Text(std::format("R {:<10}", std::bit_cast<u32>(state.image.latest_readback.hovered_value.x)).c_str());
                                        ImGui::SameLine();
                                    }
                                    if (state.image.enabled_channels.y)
                                    {
                                        ImGui::Text(std::format("G {:<10}", std::bit_cast<u32>(state.image.latest_readback.hovered_value.y)).c_str());
                                        ImGui::SameLine();
                                    }
                                    if (state.image.enabled_channels.z)
                                    {
                                        ImGui::Text(std::format("B {:<10}", std::bit_cast<u32>(state.image.latest_readback.hovered_value.z)).c_str());
                                        ImGui::SameLine();
                                    }
                                    if (state.image.enabled_channels.w)
                                    {
                                        ImGui::Text(std::format("A {:<10}", std::bit_cast<u32>(state.image.latest_readback.hovered_value.w)).c_str());
                                    }
                                    ImGui::Dummy({});
                                    break;
                                }
                                }
                            }
                            ImGui::EndTooltip();
                        }

                        ImPlot::PopStyleVar(3);
                    }
                }
            }
            if (state.free_window)
            {
                ImGui::End();
            }
        }
        else if (resource.kind == TaskResourceKind::BUFFER)
        {
            if (state.free_window)
            {
                constexpr u32 popout_default_width = 500;
                constexpr u32 popout_default_height = 380;

                ImGui::SetNextWindowSize(ImVec2(popout_default_width, popout_default_height), ImGuiCond_FirstUseEver);
                ImGui::Begin(std::format("{}::Viewer", resource.name).c_str(), &open, ImGuiWindowFlags_NoSavedSettings);
                begin_return = open;
            }

            bool recompiled_format = false;
            if (context.device.is_id_valid(state.buffer.clone_buffer) && state.open_frame_count > READBACK_CIRCULAR_BUFFER_SIZE)
            {
                BufferInfo const & source_buffer_info = context.device.buffer_info(resource.id.buffer).value();

                viewer_shared_header();
                ImGui::SameLine();
                ImGui::Checkbox("format", &state.buffer.buffer_format_config_open);
                bool const default_initialized = state.buffer.tg_debug_struct_definitions.empty();
                bool const cache_loaded = !state.buffer.format_c_struct_code.empty();

                auto compile_and_cache = [&]() -> CompileBufferStructFormatResult
                {
                    CompileBufferStructFormatResult result = {};
                    result = compile_buffer_struct_format(std::string_view{state.buffer.format_c_struct_code.data(), state.buffer.format_c_struct_code.size()});
                    if (result.success)
                    {
                        state.buffer.tg_debug_struct_definitions = result.type_definitions;
                        if(context.buffer_layout_cache_folder.has_value())
                        {
                            // write the file to disk
                            std::string file_path = std::format("{}/{}_{}.dbgstruct", context.buffer_layout_cache_folder.value().string(), impl->info.name.data(), resource.name.data());
                            std::ofstream outfile(file_path, std::ios::binary);
                            if (outfile.is_open())
                            {
                                outfile.write(state.buffer.format_c_struct_code.data(), state.buffer.format_c_struct_code.find_first_of('\0') + 1);
                                outfile.close();
                            }
                        }
                    }
                    return result;
                };

                if (default_initialized && cache_loaded)
                {
                    recompiled_format = compile_and_cache().success;
                }

                if (state.buffer.buffer_format_config_open)
                {
                    state.buffer.format_c_struct_code.resize(1u << 13u);
                    ImGui::SetNextWindowSize(ImVec2(500,700), ImGuiCond_Appearing);
                    if (ImGui::Begin("struct format", &state.buffer.buffer_format_config_open, ImGuiWindowFlags_NoSavedSettings))
                    {
                        CompileBufferStructFormatResult result = {};
                        if (ImGui::Button("Compile"))
                        {
                            result = compile_and_cache();
                            recompiled_format = result.success;
                            if (result.success)
                            {
                                state.buffer.format_c_struct_code_compile_error_message = {};
                            }
                            else
                            {
                                state.buffer.format_c_struct_code_compile_error_message = result.error_message;
                            }
                        }
                        if (state.buffer.format_c_struct_code_compile_error_message.size() > 0)
                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, ColorPalette::RED);
                            ImGui::TextWrapped(state.buffer.format_c_struct_code_compile_error_message.data());
                            ImGui::PopStyleColor();
                        }
                        ImGui::InputTextMultiline("##input", state.buffer.format_c_struct_code.data(), state.buffer.format_c_struct_code.size(), ImGui::GetContentRegionAvail());
                    }
                    ImGui::End();
                }

                auto field_element_size_align = [&](TgDebugStructFieldDefinition const & field_def) -> std::pair<u64, u64>
                {
                    if (field_def.pointer_depth > 0)
                    {
                        return { sizeof(void*), alignof(void*) };
                    }

                    TgDebugTypeDefinition const & field_type = field_def.type_index < 0 ? TG_DEBUG_PRIMITIVE_DEFINITIONS[abs(field_def.type_index) - 1] : state.buffer.tg_debug_struct_definitions.at(field_def.type_index);
                    return { field_type.size, field_type.alignment };
                };

                auto calc_memory_size_align = [&](i32 type_index, auto calc_memory_size_align_ref)
                {
                    if (type_index < 0)
                    {
                        return;
                    }

                    TgDebugTypeDefinition & type = state.buffer.tg_debug_struct_definitions.at(type_index);

                    if (type.alignment != ~0ull && type.size != ~0ull)
                    {
                        return;
                    }

                    type.alignment = {};
                    type.size = {};

                    for (u32 field_i = 0; field_i < type.fields.size(); ++field_i)
                    {
                        TgDebugStructFieldDefinition & field_def = type.fields[field_i];

                        calc_memory_size_align_ref(field_def.type_index, calc_memory_size_align_ref);

                        u64 field_size = {};
                        u64 field_alignment = {};
                        if (field_def.pointer_depth > 0)
                        {
                            field_size = field_def.array_size * sizeof(void*);
                            field_alignment = alignof(void*);
                        }
                        else
                        {
                            TgDebugTypeDefinition const & field_type = field_def.type_index < 0 ? TG_DEBUG_PRIMITIVE_DEFINITIONS[abs(field_def.type_index) - 1] : state.buffer.tg_debug_struct_definitions.at(field_def.type_index);
                            field_size = field_def.array_size * field_type.size;
                            field_alignment = field_type.alignment;
                        }

                        field_def.in_struct_memory_offset = align_up(type.size, field_alignment);

                        type.alignment = std::max(type.alignment, field_alignment);
                        type.size = align_up(type.size, field_alignment);
                        type.size += field_size;
                    }

                    type.size = align_up(type.size, type.alignment); // c style alignment and size
                };

                if (state.buffer.tg_debug_struct_definitions.empty())
                {
                    state.buffer.tg_debug_struct_definitions.push_back(TgDebugTypeDefinition{
                        .name = "Default Type",
                        .fields = {
                            TgDebugStructFieldDefinition{
                                .name = "data",
                                .type_index = TG_DEBUG_PRIMITIVE_TYPE_INDEX_U32,
                                .array_size = (1u << 10u),
                            },
                        },
                    });
                    calc_memory_size_align(state.buffer.tg_debug_struct_definitions.size() - 1, calc_memory_size_align);
                }

                if (recompiled_format)
                {
                    calc_memory_size_align(state.buffer.tg_debug_struct_definitions.size() - 1, calc_memory_size_align);
                }

                static constexpr ImGuiTableFlags type_flags = ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingStretchSame;
                static constexpr ImGuiTableFlags primitive_flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingStretchSame;
                static constexpr f32 NESTED_CHILD_HEIGHT_CONSTRAINT = 400.0f;
                static constexpr f32 DEFAULT_ROOT_TABLE_HEIGHT = 600.0f;
                static constexpr f32 TYPE_COL_WIDTH = 120.0f;
                static constexpr f32 FIELD_NAME_COL_WIDTH = 120.0f;
                static constexpr f32 VALUE_COL_WIDTH = 140.0f;
                static constexpr f32 OFFSET_COL_WIDTH = 40.0f;
                static constexpr f32 STRUCT_NEST_INDENTATION = 15.0f;
                static constexpr f32 STRUCT_NEST_MUL = 0.85f;

                static constexpr ImGuiChildFlags NESTED_CHILD_FLAGS = ImGuiChildFlags_Border;

                std::vector<TgDebugTypeDefinition> & type_defs = state.buffer.tg_debug_struct_definitions;

                auto draw_leaf = [&](TgDebugStructFieldDefinition const & field_def, u32 array_index, u64 memory_offset, std::byte* readback_ptr, u64 max_memory_offset)
                {
                    TgDebugTypeDefinition const & type_def = field_def.type_index < 0 ? TG_DEBUG_PRIMITIVE_DEFINITIONS[abs(field_def.type_index)-1] : type_defs[field_def.type_index];
                    bool const is_array = field_def.array_size > 1;
                    bool const is_pointer = field_def.pointer_depth > 0;
                    bool const memory_offset_out_of_bounds = memory_offset + type_def.size > max_memory_offset;

                    if ((array_index & 0x1) == 0)
                    {
                        ImGui::PushStyleColor(ImGuiCol_TableRowBg, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyleColorVec4(ImGuiCol_TableRowBgAlt)));
                    }
                    
                    if(memory_offset_out_of_bounds)
                    {
                        ImGui::PushStyleColor(ImGuiCol_TableRowBg, ColorPalette::DARK_RED);
                    }
                    if(ImGui::BeginTable("leaf", 4, primitive_flags))
                    {
                        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, TYPE_COL_WIDTH);
                        ImGui::TableSetupColumn("Field", ImGuiTableColumnFlags_WidthFixed, FIELD_NAME_COL_WIDTH);
                        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, VALUE_COL_WIDTH);
                        ImGui::TableSetupColumn("Offset", ImGuiTableColumnFlags_WidthFixed, OFFSET_COL_WIDTH);
                        ImGui::TableNextColumn();
                        if (is_pointer)
                        {
                            std::string text = std::format("{}{}", type_def.name, std::string(field_def.pointer_depth, '*'));
                            ImGui::Text(text.c_str());
                        }
                        else
                        {
                            ImGui::Text(type_def.name.c_str());
                        }
                        ImGui::TableNextColumn();
                        if (is_array)
                        {
                            ImGui::Text("[%i]", array_index);
                        }
                        else
                        {
                            ImGui::Text("%s", field_def.name.c_str());
                        }
                        ImGui::TableNextColumn();

                        if (memory_offset_out_of_bounds)
                        {
                            ImGui::Text("OUT OF READBACK BOUNDS");
                        }
                        else if (!is_pointer)
                        {
                            DAXA_DBG_ASSERT_TRUE_M(field_def.type_index < 0, "IMPOSSIBLE CASE! If not pointer, the type must be a primitive in a leaf!");
                            switch(field_def.type_index)
                            {
                                case TG_DEBUG_PRIMITIVE_TYPE_INDEX_F16:
                                {
                                    ImGui::Text(std::format({":#05x"}, *reinterpret_cast<u16 const*>(readback_ptr + memory_offset)).c_str());
                                }
                                break;
                                case TG_DEBUG_PRIMITIVE_TYPE_INDEX_F32: 
                                {
                                    ImGui::Text(std::format("{}", *reinterpret_cast<f32 const*>(readback_ptr + memory_offset)).c_str());
                                }
                                break;
                                case TG_DEBUG_PRIMITIVE_TYPE_INDEX_F64:
                                {
                                    ImGui::Text(std::format("{}", *reinterpret_cast<f64 const*>(readback_ptr + memory_offset)).c_str());
                                }
                                break;
                                case TG_DEBUG_PRIMITIVE_TYPE_INDEX_I8:
                                {
                                    ImGui::Text(std::format("{}", *reinterpret_cast<i8 const*>(readback_ptr + memory_offset)).c_str());
                                }
                                break;
                                case TG_DEBUG_PRIMITIVE_TYPE_INDEX_I16:
                                {
                                    ImGui::Text(std::format("{}", *reinterpret_cast<i16 const*>(readback_ptr + memory_offset)).c_str());
                                }
                                break;
                                case TG_DEBUG_PRIMITIVE_TYPE_INDEX_I32: 
                                {
                                    ImGui::Text(std::format("{}", *reinterpret_cast<i32 const*>(readback_ptr + memory_offset)).c_str());
                                }
                                break;
                                case TG_DEBUG_PRIMITIVE_TYPE_INDEX_I64:
                                {
                                    ImGui::Text(std::format("{}", *reinterpret_cast<i64 const*>(readback_ptr + memory_offset)).c_str());
                                }
                                break;
                                case TG_DEBUG_PRIMITIVE_TYPE_INDEX_U8:
                                {
                                    ImGui::Text(std::format("{}", *reinterpret_cast<u8 const*>(readback_ptr + memory_offset)).c_str());
                                }
                                break;
                                case TG_DEBUG_PRIMITIVE_TYPE_INDEX_U16:
                                {
                                    ImGui::Text(std::format("{}", *reinterpret_cast<u16 const*>(readback_ptr + memory_offset)).c_str());
                                }
                                break;
                                case TG_DEBUG_PRIMITIVE_TYPE_INDEX_U32: 
                                {
                                    ImGui::Text(std::format("{}", *reinterpret_cast<u32 const*>(readback_ptr + memory_offset)).c_str());
                                }
                                break;
                                case TG_DEBUG_PRIMITIVE_TYPE_INDEX_U64: 
                                {
                                    ImGui::Text(std::format("{}", *reinterpret_cast<u64 const*>(readback_ptr + memory_offset)).c_str());
                                }
                                break;
                            }
                        }
                        else
                        {
                            u64 address = *reinterpret_cast<u64 const*>(readback_ptr + memory_offset);
                            // Display pointers as hex values

                            if (ImGui::Button(std::format("{:#020x}##P{}", address, memory_offset).c_str()))
                            {
                                auto result = context.device.buffer_device_address_to_buffer(std::bit_cast<DeviceAddress>(address));
                                if (result.has_value())
                                {
                                    BufferId child_buffer = result.value().buffer_id;
                                    BufferInfo const info = context.device.info(child_buffer).value();
                                    printf("Belongs to buffer %s\n", info.name.c_str());

                                    if (!state.buffer.child_buffer_inspectors.contains(address))
                                    {
                                        state.buffer.child_buffer_inspectors[address] = {
                                            .field_def = field_def,
                                            .buffer = child_buffer,
                                            .base_offset = result.value().offset,
                                        };
                                    }
                                }
                            }
                        }
                        ImGui::TableNextColumn();
                        ImGui::Text("%i", memory_offset);
                        ImGui::EndTable();
                    }
                    if ((array_index & 0x1) == 0)
                    {
                        ImGui::PopStyleColor();
                    }
                    if(memory_offset_out_of_bounds)
                    {
                        ImGui::PopStyleColor();
                    }
                };

                auto get_custom_id = [&]() -> u64
                {
                    return std::hash<std::string>{}(state.buffer.custom_ui_id_stack.back());
                };
                [[maybe_unused]] auto push_custom_id = [&](std::string const & str) -> u64
                {
                    auto const & curr = state.buffer.custom_ui_id_stack.back();
                    state.buffer.custom_ui_id_stack.push_back(curr + std::string("::") + str);
                    return get_custom_id();
                };
                auto pop_custom_id = [&]()
                {
                    state.buffer.custom_ui_id_stack.pop_back();
                };
                
                ImVec4 child_bg_color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);

                /// ==============================
                /// ==== MAIN TYPED VIEWER UI ====
                /// ==============================

                auto draw_type = [&](TgDebugTypeDefinition const& type_def, std::string const & in_parent_field_name, u32 array_index, u64 memory_offset, auto & draw_type_ref, std::byte* readback_ptr, u64 max_memory_offset) -> void
                {
                    push_custom_id(in_parent_field_name);
                    ImGui::PushID(array_index);
                    if(ImGui::BeginTable("type", 2, primitive_flags))
                    {
                        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, TYPE_COL_WIDTH);
                        ImGui::TableSetupColumn("Field", ImGuiTableColumnFlags_None);
                        ImGui::TableNextColumn();
                        ImGui::Text(type_def.name.c_str());
                        ImGui::TableNextColumn();
                        if (array_index != ~0u)
                        {
                            ImGui::Text("%s[%i]", in_parent_field_name.c_str(), array_index);
                        }
                        else
                        {
                            ImGui::Text("%s", in_parent_field_name.c_str());
                        }
                        ImGui::EndTable();
                    }

                    child_bg_color.x *= STRUCT_NEST_MUL;
                    child_bg_color.y *= STRUCT_NEST_MUL;
                    child_bg_color.z *= STRUCT_NEST_MUL;
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, child_bg_color);
                    if (ImGui::BeginChild("##fields", ImVec2(0,0), NESTED_CHILD_FLAGS | (ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeY)))
                    {
                        for(i32 field_i = 0; field_i < type_def.fields.size(); ++field_i)
                        {
                            ImGui::Indent(STRUCT_NEST_INDENTATION);
                            ImGui::PushID(field_i);
                            TgDebugStructFieldDefinition const & field_def = type_def.fields[field_i];
                            bool const is_primitive_type = field_def.type_index < 0;
                            bool const is_array = field_def.array_size > 1;

                            auto [field_element_size, field_element_align] = field_element_size_align(field_def);

                            // Pointers are always treated as u64 for memory layout purposes
                            TgDebugTypeDefinition const & field_type_def = is_primitive_type ? TG_DEBUG_PRIMITIVE_DEFINITIONS[abs(field_def.type_index) - 1] : type_defs[field_def.type_index];
                            
                            u64 custom_id = push_custom_id(field_def.name);
                            ImGuiUiStorage & field_ui_storage = state.buffer.ui_storage[custom_id];

                            bool should_draw_field = true;
                            if (is_array)
                            {
                                if(ImGui::BeginTable("struct", 3, primitive_flags))
                                {
                                    ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, TYPE_COL_WIDTH);
                                    ImGui::TableSetupColumn("Field", ImGuiTableColumnFlags_None);
                                    ImGui::TableSetupColumn("Settings", ImGuiTableColumnFlags_None);
                                    ImGui::TableNextColumn();
                                    ImGui::Text(field_type_def.name.c_str());
                                    ImGui::TableNextColumn();
                                    ImGui::Text("%s[%i]", field_def.name.c_str(), field_def.array_size);
                                    ImGui::TableNextColumn();
                                    ImGui::Checkbox("Constrain", &field_ui_storage.array_collapsed);
                                    if (field_ui_storage.array_collapsed)
                                    {
                                        ImGui::SameLine();
                                        ImGui::SetNextItemWidth(100);
                                        ImGui::SliderFloat("", &field_ui_storage.constrained_height, 1.0f, 500.0f);
                                    }
                                    else
                                    {
                                        ImGui::SameLine();
                                        ImGui::Checkbox("Hide", &field_ui_storage.array_hidden);
                                    }
                                    ImGui::EndTable();
                                }

                                should_draw_field = false;
                                if(!field_ui_storage.array_hidden)
                                {
                                    if(field_ui_storage.array_collapsed) { ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, 0.0f), ImVec2(FLT_MAX, field_ui_storage.constrained_height)); }
                                    child_bg_color.x *= STRUCT_NEST_MUL;
                                    child_bg_color.y *= STRUCT_NEST_MUL;
                                    child_bg_color.z *= STRUCT_NEST_MUL;
                                    ImGui::PushStyleColor(ImGuiCol_ChildBg, child_bg_color);
                                    should_draw_field = ImGui::BeginChild("##array", ImVec2(0,0), NESTED_CHILD_FLAGS | (ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeY));
                                }
                            }

                            if (should_draw_field)
                            {
                                if (is_array)
                                {
                                    ImGui::Indent(STRUCT_NEST_INDENTATION);
                                }
                                ImGuiListClipper clipper = {};
                                clipper.Begin(field_def.array_size);
                                while (clipper.Step())
                                {
                                    for(i32 array_i = clipper.DisplayStart; array_i < clipper.DisplayEnd; ++array_i)
                                    {
                                        u64 array_element_memory_offset = memory_offset + field_def.in_struct_memory_offset + field_element_size * array_i;
                                        if (is_primitive_type || field_def.pointer_depth > 0)
                                        {
                                            draw_leaf(field_def, array_i, array_element_memory_offset, readback_ptr, max_memory_offset);
                                        }
                                        else
                                        {
                                            draw_type_ref(field_type_def, field_def.name, is_array ? array_i : ~0u, array_element_memory_offset, draw_type_ref, readback_ptr, max_memory_offset);
                                        }
                                    }
                                }
                                if (is_array)
                                {
                                    ImGui::Unindent(STRUCT_NEST_INDENTATION);
                                }
                            }
                            if (is_array && !field_ui_storage.array_hidden)
                            {
                                child_bg_color.x /= STRUCT_NEST_MUL;
                                child_bg_color.y /= STRUCT_NEST_MUL;
                                child_bg_color.z /= STRUCT_NEST_MUL;
                                ImGui::PopStyleColor();
                                ImGui::EndChild();
                            }
                            pop_custom_id();
                            ImGui::PopID();
                            ImGui::Unindent(STRUCT_NEST_INDENTATION);
                        }
                    }

                    child_bg_color.x /= STRUCT_NEST_MUL;
                    child_bg_color.y /= STRUCT_NEST_MUL;
                    child_bg_color.z /= STRUCT_NEST_MUL;
                    ImGui::PopStyleColor();
                    pop_custom_id();

                    ImGui::EndChild();
                    ImGui::PopID();
                };

                /// ===============================
                /// ==== DRAW BUFFER VIEWER UI ====
                /// ===============================
                
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(ImGui::GetStyle().CellPadding.x, 1));
                if (ImGui::BeginChild("root child",ImVec2(0, DEFAULT_ROOT_TABLE_HEIGHT + 200), NESTED_CHILD_FLAGS | ImGuiChildFlags_ResizeY))
                {
                    auto readback_host_ptr = context.device.buffer_host_address(state.buffer.clone_buffer).value();
                    draw_type(type_defs.back(), "root", ~0u, 0ull, draw_type, readback_host_ptr, source_buffer_info.size);
                }
                ImGui::PopStyleVar(2);
                ImGui::EndChild();

                /// =====================================
                /// ==== DRAW CHILD BUFFER VIEWER UI ====
                /// =====================================
                {
                    for (auto iter = state.buffer.child_buffer_inspectors.begin(); iter != state.buffer.child_buffer_inspectors.end(); )
                    {
                        bool close_viewer = false;
                        {
                            ChildBufferResourceViewerState & child_viewer_state = iter->second;

                            bool const buffer_still_alive = context.device.is_buffer_id_valid(child_viewer_state.buffer);

                            if (!child_viewer_state.clone_buffer.is_empty())
                            {
                                bool gui_open = true;
                                ImGui::SetNextWindowSize(ImVec2(600,600), ImGuiCond_Appearing);
                                if (ImGui::Begin(std::format("Pointer Viewer at {:#020x}", iter->first).c_str(), &gui_open, ImGuiWindowFlags_NoSavedSettings))
                                {
                                    BufferId child_buffer = child_viewer_state.buffer;
                                    BufferInfo info = context.device.buffer_info(child_buffer).value();

                                    auto child_readback_host_ptr = context.device.buffer_host_address(child_viewer_state.clone_buffer).value();

                                    ImGui::Text(std::format("pointer address to \"{:<32}\" offset {:<20}", info.name.c_str(), child_viewer_state.base_offset).c_str());
                                    ImGui::SetNextItemWidth(120);
                                    ImGui::InputInt("Array Size", &child_viewer_state.display_array_size);
                                    ImGui::SameLine();
                                    ImGui::Checkbox("Freeze", &state.freeze_resource);
                                    if (buffer_still_alive)
                                    {
                                        TgDebugStructFieldDefinition field = child_viewer_state.field_def;
                                        field.pointer_depth = std::max(child_viewer_state.field_def.pointer_depth, 1) - 1;
                                        field.array_size = child_viewer_state.display_array_size;

                                        TgDebugTypeDefinition viewer_type = {
                                            .name = "Root",
                                            .fields = {
                                                field,
                                            },
                                        };

                                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 0));
                                        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(ImGui::GetStyle().CellPadding.x, 1));
                                        draw_type(viewer_type, "root", ~0u, child_viewer_state.base_offset, draw_type, child_readback_host_ptr, info.size);
                                        ImGui::PopStyleVar(2);
                                    }
                                    else
                                    {
                                        ImGui::Text("Resource Was Destroyed, Viewer Pointer Invalid!");
                                    }
                                }
                                ImGui::End();

                                close_viewer = !gui_open;

                            }
                        }
                        if (close_viewer)
                        {
                            if (context.device.is_buffer_id_valid(iter->second.clone_buffer))
                            {
                                context.device.destroy_buffer(iter->second.clone_buffer);
                            }
                            if (context.device.is_buffer_id_valid(iter->second.destroy_clone_buffer))
                            {
                                context.device.destroy_buffer(iter->second.destroy_clone_buffer);
                            }
                            iter = state.buffer.child_buffer_inspectors.erase(iter);
                        }
                        else
                        {
                            ++iter;
                        }
                    }
                }
            }

            if (state.free_window)
            {
                ImGui::End();
            }
        }

        return begin_return;
    }
} // namespace daxa

#endif
#endif