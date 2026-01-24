#if DAXA_BUILT_WITH_UTILS_TASK_GRAPH && DAXA_BUILT_WITH_UTILS_IMGUI
#include <daxa/utils/task_graph_types.hpp>

#if ENABLE_TASK_GRAPH_MK2

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

    auto tg_debug_buffer_entry_type_bytesize(TG_DEBUG_BUFFER_ENTRY_DATA_TYPE type) -> u32
    {
        switch (type)
        {
        case TG_DEBUG_BUFFER_ENTRY_DATA_TYPE_F32: return 4u;
        case TG_DEBUG_BUFFER_ENTRY_DATA_TYPE_I32: return 4u;
        case TG_DEBUG_BUFFER_ENTRY_DATA_TYPE_U32: return 4u;
        default:
            DAXA_DBG_ASSERT_TRUE_M(false, "IMPOSSIBLE CASE! Data corruption or forgot to add new data type to this function");
            return ~0;
        }
    };

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

            if (attachment_info.type == TaskAttachmentType::BUFFER)
            {
                BufferId buffer_id = ti.id(TaskBufferAttachmentIndex{attach_i});
                BufferInfo buffer_info = ti.info(TaskBufferAttachmentIndex{attach_i}).value();

                u64 const clamped_readback_end = std::min(buffer_info.size, state.buffer.readback_offset + BUFFER_RESOURCE_VIEWER_READBACK_SIZE);
                u64 const clamped_readback_size = clamped_readback_end - state.buffer.readback_offset;
                u64 const readback_index = (state.open_frame_count) % READBACK_CIRCULAR_BUFFER_SIZE;
                u64 const readback_dst_offset = BUFFER_RESOURCE_VIEWER_READBACK_SIZE * readback_index;

                if (clamped_readback_size > 0)
                {
                    ti.recorder.pipeline_barrier(daxa::BarrierInfo{
                        .src_access = daxa::AccessConsts::READ_WRITE,
                        .dst_access = daxa::AccessConsts::READ_WRITE,
                    });
                    ti.recorder.copy_buffer_to_buffer({
                        .src_buffer = buffer_id,
                        .dst_buffer = state.buffer.readback_buffer,
                        .src_offset = state.buffer.readback_offset,
                        .dst_offset = readback_dst_offset,
                        .size = clamped_readback_size,
                    });
                    ti.recorder.pipeline_barrier(daxa::BarrierInfo{
                        .src_access = daxa::AccessConsts::READ_WRITE,
                        .dst_access = daxa::AccessConsts::READ_WRITE,
                    });
                }

                // Fill tail after the clipped readback with zeros
                std::byte* readback_host_ptr = context.device.buffer_host_address(state.buffer.readback_buffer).value();
                for (u64 b = clamped_readback_size; b < BUFFER_RESOURCE_VIEWER_READBACK_SIZE; ++b)
                {
                    readback_host_ptr[b] = {};
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
                    ImGui::SetNextItemWidth(80);
                    ImGui::InputInt("Mip", &state.image.mip);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(80);
                    ImGui::InputInt("Layer", &state.image.layer);

                    auto draw_color_checkbox = [&](const char * id, ImVec4 color, i32 & checked, bool enabled)
                    {
                        const float hovered_extra = 0.3f;
                        const float active_extra = 0.2f;
                        const float normal_downscale = 0.8f;
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
                        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0,0,0,1));
                        ImGui::BeginDisabled(!enabled);
                        ImGui::Checkbox(id, reinterpret_cast<bool *>(&checked));
                        ImGui::EndDisabled();
                        ImGui::PopStyleColor(4);
                    };

                    ImGui::SameLine();
                    draw_color_checkbox("##R", ImVec4(0.90590f, 0.29800f, 0.23530f, 1.0f), state.image.enabled_channels.x, channel_count > 0);
                    ImGui::SameLine();
                    draw_color_checkbox("##G", ImVec4(0.18040f, 0.80000f, 0.44310f, 1.0f), state.image.enabled_channels.y, channel_count > 1);
                    ImGui::SameLine();
                    draw_color_checkbox("##B", ImVec4(0.20390f, 0.59610f, 0.85880f, 1.0f), state.image.enabled_channels.z, channel_count > 2);
                    ImGui::SameLine();
                    draw_color_checkbox("##A", ImVec4(0.7f, 0.7f, 0.7f, 1.0f), state.image.enabled_channels.w, channel_count > 2);

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
                            ImGui::Text(std::format("Min {:<10.3e} Max {:<10.3e}", min_value, max_value).c_str());
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
                                        ImGui::Text(std::format("R {:<6.3e}", static_cast<f64>(std::bit_cast<f32>(state.image.latest_readback.hovered_value.x))).c_str());
                                        ImGui::SameLine();
                                    }
                                    if (state.image.enabled_channels.y)
                                    {
                                        ImGui::Text(std::format("G {:<6.3e}", static_cast<f64>(std::bit_cast<f32>(state.image.latest_readback.hovered_value.y))).c_str());
                                        ImGui::SameLine();
                                    }
                                    if (state.image.enabled_channels.z)
                                    {
                                        ImGui::Text(std::format("B {:<6.3e}", static_cast<f64>(std::bit_cast<f32>(state.image.latest_readback.hovered_value.z))).c_str());
                                        ImGui::SameLine();
                                    }
                                    if (state.image.enabled_channels.w)
                                    {
                                        ImGui::Text(std::format("A {:<6.3e}", static_cast<f64>(std::bit_cast<f32>(state.image.latest_readback.hovered_value.w))).c_str());
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
                constexpr u32 header_height = 100;
                constexpr u32 popout_default_width = 500;
                constexpr u32 popout_default_height = 380;

                ImGui::SetNextWindowSize(ImVec2(popout_default_width, popout_default_height), ImGuiCond_FirstUseEver);
                ImGui::Begin(std::format("{}::Viewer", resource.name).c_str(), &open, ImGuiWindowFlags_NoSavedSettings);
                begin_return = open;
            }

            if (context.device.is_id_valid(state.buffer.readback_buffer) && state.open_frame_count > READBACK_CIRCULAR_BUFFER_SIZE)
            {
                BufferInfo const & source_buffer_info = context.device.buffer_info(resource.id.buffer).value();
                if (state.buffer.buffer_format_config_open)
                {
                    u64 display_struct_size = 0;
                    ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);
                    if (ImGui::Begin(std::format("{} debug format config", source_buffer_info.name.data()).c_str(), &state.buffer.buffer_format_config_open, ImGuiWindowFlags_NoSavedSettings))
                    {
                        if (ImGui::BeginTable(std::format("##{}", source_buffer_info.name.data()).c_str(), 4, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg))
                        {
                            ImGui::TableSetupColumn("Name");
                            ImGui::TableSetupColumn("Type");
                            ImGui::TableSetupColumn("Array Size");
                            ImGui::TableSetupColumn("Offset");
                            ImGui::TableHeadersRow();
                            
                            for(i32 buffer_structure_entry = 0; buffer_structure_entry < state.buffer.tg_debug_buffer_structure.size(); ++buffer_structure_entry)
                            {
                                ImGui::PushID(buffer_structure_entry);
                                auto & entry = state.buffer.tg_debug_buffer_structure.at(buffer_structure_entry);
                                // Name column
                                ImGui::TableNextColumn();
                                constexpr u32 name_buffer_size = 256;
                                std::array<char, name_buffer_size> name_buffer = {};
                                std::memcpy(name_buffer.data(), entry.name.data(), entry.name.size());
                                ImGui::SetNextItemWidth(100);
                                ImGui::InputText("##", name_buffer.data(), name_buffer_size);
                                entry.name = std::string(name_buffer.data());

                                ImGui::TableNextColumn();
                                const char * data_type_labels = "F32\0U32\0I32\0"; 
                                ImGui::SetNextItemWidth(100);
                                ImGui::Combo("##data_type", reinterpret_cast<i32*>(&entry.data_type), data_type_labels, 3);

                                ImGui::TableNextColumn();
                                ImGui::SetNextItemWidth(100);
                                ImGui::InputInt("##array_size", &entry.array_size);
                                ImGui::PopID();

                                display_struct_size += entry.array_size * tg_debug_buffer_entry_type_bytesize(entry.data_type);

                                entry.offset = display_struct_size;

                                ImGui::TableNextColumn();
                                ImGui::Text("%d", entry.offset);
                            }

                            ImGui::EndTable();
                        }
                        if(ImGui::Button("Add Row"))
                        {
                            state.buffer.tg_debug_buffer_structure.push_back({});
                        }
                    }
                    ImGui::End();
                }

                if (begin_return)
                {
                    if (ImGui::Button("Open buffer layout config window", ImVec2({80, 20})))
                    {
                        state.buffer.buffer_format_config_open = true;
                    }
                }

                
                u64 ui_struct_size = 64;
                u64 const array_entries = 1000;//source_buffer_info.size / ui_struct_size;
                if (ImGui::BeginTable(std::format("##{}2", source_buffer_info.name.data()).c_str(), 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoHostExtendY | ImGuiTableFlags_ScrollY, ImVec2(0,300)))
                {
                    ImGui::TableSetupColumn("Name");
                    ImGui::TableSetupColumn("Value");
                    ImGui::TableHeadersRow();
                    for (u32 i = 0; i < array_entries; ++i)
                    {
                        ImGui::TableNextColumn();
                        ImGui::Text("test");
                    }
                    ImGui::EndTable();
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