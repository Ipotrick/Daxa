#include <daxa/utils/task_graph_debug.hpp>

#define FMT_HEADER_ONLY
#define FMT_UNICODE 0
#include <fmt/format.h>

#include <iostream>

#include <daxa/utils/task_graph_debug.inl>

enum struct ScalarKind
{
    FLOAT,
    INT,
    UINT
};

auto channel_count_of_format(daxa::Format format) -> daxa_u32
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
    default: return 0;
    }
    return 0;
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

void TgDebugContext::init(daxa::Device & device, daxa::PipelineManager & pipeline_manager)
{
    this->device = device;
    auto result = pipeline_manager.add_compute_pipeline(daxa::ComputePipelineCompileInfo{
        .shader_info = {
            .source = daxa::ShaderFile{"daxa/utils/task_graph_debug.glsl"},
            .compile_options = {
                .entry_point = "entry_draw_debug_display",
                .language = daxa::ShaderLanguage::GLSL,
            },
        },
        .push_constant_size = sizeof(DebugTaskDrawDebugDisplayPush),
        .name = "debug_task_pipeline",
    });

    if (!result.value()->is_valid())
        std::cout << result.m.c_str() << std::endl;

    DAXA_DBG_ASSERT_TRUE_M(result.value()->is_valid(), "FAILED TO COMPILE DEBUG SHADER");

    this->debug_pipeline = result.value();
}

void TgDebugContext::cleanup()
{
    for (auto & inspector : inspector_states)
    {
        if (!inspector.second.display_image.is_empty())
            device.destroy_image((inspector.second.display_image));
        if (!inspector.second.frozen_image.is_empty())
            device.destroy_image((inspector.second.frozen_image));
        if (!inspector.second.stale_image.is_empty())
            device.destroy_image((inspector.second.stale_image));
        if (!inspector.second.stale_image1.is_empty())
            device.destroy_image((inspector.second.stale_image1));
        if (!inspector.second.readback_buffer.is_empty())
            device.destroy_buffer((inspector.second.readback_buffer));
    }
}

auto format_vec4_rows_float(daxa_f32vec4 vec) -> std::string
{
    return fmt::format("R: {:10.7}\nG: {:10.7}\nB: {:10.7}\nA: {:10.7}",
                       vec.x,
                       vec.y,
                       vec.z,
                       vec.w);
}
auto format_vec4_rows(Vec4Union vec_union, ScalarKind scalar_kind) -> std::string
{
    switch (scalar_kind)
    {
    case ScalarKind::FLOAT:
        return format_vec4_rows_float(vec_union._float);
    case ScalarKind::INT:
        return fmt::format("R: {:11}\nG: {:11}\nB: {:11}\nA: {:11}",
                           vec_union._int.x,
                           vec_union._int.y,
                           vec_union._int.z,
                           vec_union._int.w);
    case ScalarKind::UINT:
        return fmt::format("R: {:11}\nG: {:11}\nB: {:11}\nA: {:11}",
                           vec_union._uint.x,
                           vec_union._uint.y,
                           vec_union._uint.z,
                           vec_union._uint.w);
    }
    return std::string();
}

void TgDebugContext::debug_task(daxa::TaskInterface ti, bool pre_task)
{
    auto & tg_debug = *this;
    auto & pipeline = *this->debug_pipeline;

    if (pre_task)
    {
        std::string task_name = std::string(ti.task_name);
        size_t name_duplication = tg_debug.this_frame_duplicate_task_name_counter[task_name]++;
        if (name_duplication > 0)
        {
            task_name = task_name + " (" + std::to_string(name_duplication) + ")";
        }
        tg_debug.this_frame_task_attachments.push_back(TgDebugContext::TgDebugTask{.task_name = task_name});
    }
    size_t this_frame_task_index = tg_debug.this_frame_task_attachments.size() - 1ull;
    int inl_attachment_index = 0;
    for (uint32_t i = 0; i < ti.attachment_infos.size(); ++i)
    {
        if (ti.attachment_infos[i].type != daxa::TaskAttachmentType::IMAGE)
            continue;
        daxa::TaskImageAttachmentIndex src = {i};
        auto & attach_info = ti.get(src);

        std::string key = std::string(ti.task_name) + "::AT." + ti.attachment_infos[i].name();
        if (strcmp(ti.attachment_infos[i].name(), "inline attachment") == 0)
        {
            key += " ";
            key += std::to_string(inl_attachment_index);
            ++inl_attachment_index;
        }
        
        if (pre_task)
        {
            tg_debug.this_frame_task_attachments[this_frame_task_index].attachments.push_back(ti.attachment_infos[i]);
        }

        if (!tg_debug.inspector_states.contains(key))
            continue;

        auto & state = tg_debug.inspector_states.at(key);
        if (!state.active)
            continue;

        if (state.pre_task != pre_task)
            continue;

        // Destroy Stale images from last frame.
        if (!state.stale_image.is_empty())
        {
            ti.device.destroy_image(state.stale_image);
            state.stale_image = {};
        }
        if (!state.stale_image1.is_empty())
        {
            ti.device.destroy_image(state.stale_image1);
            state.stale_image1 = {};
        }
        if (state.readback_buffer.is_empty())
        {
            state.readback_buffer = ti.device.create_buffer({
                .size = sizeof(daxa_f32vec4) * 2 /*raw,color*/ * 4 /*frames in flight*/,
                .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                .name = std::string("readback buffer for ") + key,
            });
        }

        daxa::ImageId src_id = ti.get(src).ids[0];       // either src image id or frozen image id
        daxa::ImageInfo src_info = ti.info(src).value(); // either src image info ir frozen image info
        ScalarKind scalar_kind = scalar_kind_of_format(src_info.format);
        if (state.freeze_image)
        {
            bool const freeze_image_this_frame = state.frozen_image.is_empty() && state.freeze_image;
            if (freeze_image_this_frame)
            {
                daxa::ImageInfo image_frozen_info = src_info;
                image_frozen_info.usage |= daxa::ImageUsageFlagBits::TRANSFER_DST;
                image_frozen_info.name = std::string(src_info.name.data()) + " frozen copy";
                state.frozen_image = ti.device.create_image(image_frozen_info);

                daxa::ClearValue frozen_copy_clear = {};
                if (is_format_depth_stencil(src_info.format))
                {
                    frozen_copy_clear = daxa::DepthValue{.depth = 0.0f, .stencil = 0u};
                }
                else
                {
                    switch (scalar_kind)
                    {
                    case ScalarKind::FLOAT: frozen_copy_clear = std::array{1.0f, 0.0f, 1.0f, 1.0f}; break;
                    case ScalarKind::INT: frozen_copy_clear = std::array{1, 0, 0, 1}; break;
                    case ScalarKind::UINT: frozen_copy_clear = std::array{1u, 0u, 0u, 1u}; break;
                    }
                }

                daxa::ImageMipArraySlice slice = {
                    .level_count = src_info.mip_level_count,
                    .layer_count = src_info.array_layer_count,
                };

                ti.recorder.pipeline_barrier_image_transition(daxa::ImageMemoryBarrierInfo{
                    .dst_access = daxa::AccessConsts::TRANSFER_WRITE,
                    .dst_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .image_slice = slice,
                    .image_id = state.frozen_image,
                });

                ti.recorder.clear_image({
                    .clear_value = std::array{1.0f, 0.0f, 1.0f, 1.0f},
                    .dst_image = state.frozen_image,
                    .dst_slice = slice,
                });

                ti.recorder.pipeline_barrier(daxa::MemoryBarrierInfo{
                    .src_access = daxa::AccessConsts::TRANSFER_WRITE,
                    .dst_access = daxa::AccessConsts::TRANSFER_WRITE,
                });

                ti.recorder.pipeline_barrier_image_transition(daxa::ImageMemoryBarrierInfo{
                    .src_access = ti.get(src).access,
                    .dst_access = daxa::AccessConsts::TRANSFER_WRITE,
                    .src_layout = ti.get(src).layout,
                    .dst_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
                    .image_slice = ti.get(src).view.slice,
                    .image_id = src_id,
                });

                // Copy src image data to frozen image.
                for (uint32_t mip = slice.base_mip_level; mip < (slice.base_mip_level + slice.level_count); ++mip)
                {
                    ti.recorder.copy_image_to_image({
                        .src_image = src_id,
                        .dst_image = state.frozen_image,
                        .src_slice = daxa::ImageArraySlice::slice(slice, mip),
                        .dst_slice = daxa::ImageArraySlice::slice(slice, mip),
                        .extent = {
                            std::max(1u, src_info.size.x >> mip),
                            std::max(1u, src_info.size.y >> mip),
                            std::max(1u, src_info.size.z >> mip)},
                    });
                }

                ti.recorder.pipeline_barrier_image_transition(daxa::ImageMemoryBarrierInfo{
                    .src_access = daxa::AccessConsts::TRANSFER_WRITE,
                    .dst_access = ti.get(src).access,
                    .src_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
                    .dst_layout = ti.get(src).layout,
                    .image_slice = ti.get(src).view.slice,
                    .image_id = src_id,
                });
                ti.recorder.pipeline_barrier_image_transition(daxa::ImageMemoryBarrierInfo{
                    .src_access = daxa::AccessConsts::TRANSFER_WRITE,
                    .dst_access = daxa::AccessConsts::COMPUTE_SHADER_READ,
                    .src_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .dst_layout = daxa::ImageLayout::READ_ONLY_OPTIMAL,
                    .image_slice = slice,
                    .image_id = state.frozen_image,
                });
            }
            // Frozen image overwrites.
            src_id = state.frozen_image;
            src_info = ti.device.image_info(state.frozen_image).value();
        }
        else
        {
            // If not frozen, copy over new data for ui.
            state.attachment_info = ti.get(src);
            state.runtime_image_info = src_info;
            // Mark frozen copy for deletion in next frame.
            state.stale_image1 = state.frozen_image;
            state.frozen_image = {};
        }

        if (src_id.is_empty())
        {
            return;
        }

        state.slice_valid = state.attachment_info.view.slice.contains(daxa::ImageMipArraySlice{
            .base_mip_level = state.mip,
            .base_array_layer = state.layer,
        });

        daxa::ImageInfo display_image_info = {};
        display_image_info.dimensions = 2u;
        display_image_info.size.x = std::max(1u, src_info.size.x >> state.mip);
        display_image_info.size.y = std::max(1u, src_info.size.y >> state.mip);
        display_image_info.size.z = 1u;
        display_image_info.mip_level_count = 1u;
        display_image_info.array_layer_count = 1u;
        display_image_info.sample_count = 1u;
        display_image_info.format = daxa::Format::R16G16B16A16_SFLOAT;
        display_image_info.name = "tg image debug clone";
        display_image_info.usage = daxa::ImageUsageFlagBits::SHADER_SAMPLED | daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_DST;

        if (!state.display_image.is_empty())
        {
            auto current_clone_size = ti.device.image_info(state.display_image).value().size;
            if (display_image_info.size.x != current_clone_size.x || display_image_info.size.y != current_clone_size.y)
            {
                state.stale_image = state.display_image;
                state.display_image = ti.device.create_image(display_image_info);
            }
        }
        else
        {
            state.display_image = ti.device.create_image(display_image_info);
        }

        ti.recorder.pipeline_barrier_image_transition(daxa::ImageMemoryBarrierInfo{
            .dst_access = daxa::AccessConsts::TRANSFER_WRITE,
            .dst_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .image_id = state.display_image,
        });

        ti.recorder.clear_image({
            .clear_value = std::array{1.0f, 0.0f, 1.0f, 1.0f},
            .dst_image = state.display_image,
        });

        if (state.slice_valid)
        {
            ti.recorder.pipeline_barrier_image_transition(daxa::ImageMemoryBarrierInfo{
                .src_access = daxa::AccessConsts::TRANSFER_WRITE,
                .dst_access = daxa::AccessConsts::COMPUTE_SHADER_WRITE,
                .src_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                .dst_layout = daxa::ImageLayout::GENERAL,
                .image_slice = ti.device.image_view_info(state.display_image.default_view()).value().slice,
                .image_id = state.display_image,
            });
            if (!state.freeze_image)
            {
                ti.recorder.pipeline_barrier_image_transition(daxa::ImageMemoryBarrierInfo{
                    .src_access = ti.get(src).access,
                    .dst_access = daxa::AccessConsts::COMPUTE_SHADER_READ,
                    .src_layout = ti.get(src).layout,
                    .dst_layout = daxa::ImageLayout::READ_ONLY_OPTIMAL,
                    .image_slice = ti.get(src).view.slice,
                    .image_id = src_id,
                });
            }

            ti.recorder.set_pipeline(pipeline);

            daxa::ImageViewInfo src_image_view_info = ti.device.image_view_info(src_id.default_view()).value();
            src_image_view_info.slice.base_mip_level = state.mip;
            src_image_view_info.slice.base_array_layer = state.layer;
            daxa::ImageViewId src_view = ti.device.create_image_view(src_image_view_info);
            ti.recorder.destroy_image_view_deferred(src_view);
            ti.recorder.push_constant(DebugTaskDrawDebugDisplayPush{
                .src = src_view,
                .dst = state.display_image.default_view(),
                .src_size = {display_image_info.size.x, display_image_info.size.y},
                .image_view_type = static_cast<daxa_u32>(src_image_view_info.type),
                .format = static_cast<daxa_i32>(scalar_kind),
                .float_min = static_cast<daxa_f32>(state.min_v),
                .float_max = static_cast<daxa_f32>(state.max_v),
                .int_min = static_cast<daxa_i32>(state.min_v),
                .int_max = static_cast<daxa_i32>(state.max_v),
                .uint_min = static_cast<daxa_u32>(state.min_v),
                .uint_max = static_cast<daxa_u32>(state.max_v),
                .rainbow_ints = state.rainbow_ints,
                .enabled_channels = state.enabled_channels,
                .mouse_over_index = {
                    state.mouse_pos_relative_to_image_mip0.x >> state.mip,
                    state.mouse_pos_relative_to_image_mip0.y >> state.mip,
                },
                .readback_ptr = ti.device.device_address(state.readback_buffer).value(),
                .readback_index = tg_debug.readback_index,
            });
            ti.recorder.dispatch({
                (display_image_info.size.x + 15) / 16,
                (display_image_info.size.y + 15) / 16,
                1,
            });
            if (!state.freeze_image)
            {
                ti.recorder.pipeline_barrier_image_transition(daxa::ImageMemoryBarrierInfo{
                    .src_access = daxa::AccessConsts::COMPUTE_SHADER_READ,
                    .dst_access = ti.get(src).access,
                    .src_layout = daxa::ImageLayout::READ_ONLY_OPTIMAL,
                    .dst_layout = ti.get(src).layout,
                    .image_slice = ti.get(src).view.slice,
                    .image_id = src_id,
                });
            }
            ti.recorder.pipeline_barrier_image_transition(daxa::ImageMemoryBarrierInfo{
                .src_access = daxa::AccessConsts::COMPUTE_SHADER_WRITE,
                .dst_access = daxa::AccessConsts::FRAGMENT_SHADER_READ,
                .src_layout = daxa::ImageLayout::GENERAL,
                .dst_layout = daxa::ImageLayout::READ_ONLY_OPTIMAL,
                .image_id = state.display_image,
            });
        }
        else // ui image slice NOT valid.
        {
            ti.recorder.pipeline_barrier_image_transition(daxa::ImageMemoryBarrierInfo{
                .src_access = daxa::AccessConsts::TRANSFER_WRITE,
                .dst_access = daxa::AccessConsts::FRAGMENT_SHADER_READ,
                .src_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                .dst_layout = daxa::ImageLayout::READ_ONLY_OPTIMAL,
                .image_id = state.display_image,
            });
        }
    }
}

#include <imgui.h>

auto TgDebugContext::debug_ui_image_inspector(daxa::ImGuiRenderer & imgui_renderer, std::string active_inspector_key) -> bool
{
    auto & tg_debug = *this;

    tg_debug.readback_index = (tg_debug.readback_index + 1) % 3;
    auto & state = tg_debug.inspector_states[active_inspector_key];
    auto inspector_title = std::string("Inspector for ") + active_inspector_key;
    bool open = true;
    if (ImGui::Begin(inspector_title.c_str(), &open, {}))
    {
        // The ui update is staggered a frame.
        // This is because the ui gets information from the task graph with a delay of one frame.
        // Because of this we first shedule a draw for the previous frames debug image canvas.
        ImTextureID tex_id = {};
        daxa::ImageInfo clone_image_info = {};
        daxa::ImageInfo const & image_info = state.runtime_image_info;
        if (!state.display_image.is_empty())
        {
            clone_image_info = tg_debug.device.image_info(state.display_image).value();

            daxa::SamplerId sampler;
            //  = gpu_context->lin_clamp_sampler;
            // if (state.nearest_filtering)
            // {
            // 	sampler = gpu_context->nearest_clamp_sampler;
            // }
            tex_id = imgui_renderer.create_texture_id({
                .image_view_id = state.display_image.default_view(),
                .sampler_id = sampler,
            });
            daxa_u32 active_channels_of_format = channel_count_of_format(image_info.format);

            // Now we actually process the ui.
            daxa::TaskImageAttachmentInfo const & attachment_info = state.attachment_info;
            auto slice = attachment_info.view.slice;

            if (ImGui::BeginTable("Some Inspected Image", 2, ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedFit))
            {
                ImGui::TableSetupColumn("Inspector settings", ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedFit);
                ImGui::TableSetupColumn("Image view", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableNextColumn();
                ImGui::SeparatorText("Inspector settings");
                ImGui::Checkbox("pre task", &state.pre_task);
                ImGui::SameLine();
                ImGui::Checkbox("freeze image", &state.freeze_image);
                ImGui::SetItemTooltip("make sure to NOT have freeze image set when switching this setting. The frozen image is either pre or post.");
                ImGui::PushItemWidth(80);
                daxa_i32 imip = state.mip;
                ImGui::InputInt("mip", &imip, 1);
                state.mip = imip;
                daxa_i32 ilayer = state.layer;
                ImGui::SameLine();
                ImGui::InputInt("layer", &ilayer, 1);
                state.layer = ilayer;
                ImGui::PopItemWidth();
                ImGui::PushItemWidth(180);
                ImGui::Text("selected mip size: (%i,%i,%i)", std::max(image_info.size.x >> state.mip, 1u), std::max(image_info.size.y >> state.mip, 1u), std::max(image_info.size.z >> state.mip, 1u));
                if (!state.slice_valid)
                    ImGui::PushStyleColor(ImGuiCol_Text, 0xFF0000FF);
                ImGui::Text(state.slice_valid ? "" : "SELECTED SLICE INVALID");
                if (!state.slice_valid)
                    ImGui::PopStyleColor();
                auto modes = std::array{
                    "Linear",
                    "Nearest",
                };
                ImGui::Combo("sampler", &state.nearest_filtering, modes.data(), modes.size());

                if (ImGui::BeginTable("Channels", 4, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingFixedFit))
                {
                    std::array channel_names = {"r", "g", "b", "a"};
                    std::array<bool, 4> channels = {};

                    daxa_u32 active_channel_count = 0;
                    daxa_i32 last_active_channel = -1;
                    for (daxa_u32 channel = 0; channel < 4; ++channel)
                    {
                        channels[channel] = std::bit_cast<std::array<daxa_i32, 4>>(state.enabled_channels)[channel];
                        active_channel_count += channels[channel] ? 1u : 0u;
                        last_active_channel = channels[channel] ? channel : channels[channel];
                    }

                    for (daxa_u32 channel = 0; channel < 4; ++channel)
                    {
                        auto const disabled = channel >= active_channels_of_format;
                        ImGui::BeginDisabled(disabled);
                        if (disabled)
                            channels[channel] = false;
                        ImGui::TableNextColumn();
                        bool const clicked = ImGui::Checkbox(channel_names[channel], channels.data() + channel);
                        ImGui::EndDisabled();
                        if (disabled)
                            ImGui::SetItemTooltip("image format does not have this channel");
                    }

                    state.enabled_channels.x = channels[0];
                    state.enabled_channels.y = channels[1];
                    state.enabled_channels.z = channels[2];
                    state.enabled_channels.w = channels[3];
                    ImGui::EndTable();
                }
                ImGui::PopItemWidth();
                ImGui::PushItemWidth(90);
                ImGui::InputDouble("min", &state.min_v);
                ImGui::SetItemTooltip("min value only effects rgb, not alpha");
                ImGui::SameLine();
                ImGui::InputDouble("max", &state.max_v);
                ImGui::SetItemTooltip("max value only effects rgb, not alpha");

                Vec4Union readback_raw = {};
                daxa_f32vec4 readback_color = {};
                daxa_f32vec4 readback_color_min = {};
                daxa_f32vec4 readback_color_max = {};

                ScalarKind scalar_kind = scalar_kind_of_format(image_info.format);
                if (!state.readback_buffer.is_empty())
                {
                    switch (scalar_kind)
                    {
                    case ScalarKind::FLOAT: readback_raw._float = device.buffer_host_address_as<daxa_f32vec4>(state.readback_buffer).value()[tg_debug.readback_index * 2]; break;
                    case ScalarKind::INT: readback_raw._int = device.buffer_host_address_as<daxa_i32vec4>(state.readback_buffer).value()[tg_debug.readback_index * 2]; break;
                    case ScalarKind::UINT: readback_raw._uint = device.buffer_host_address_as<daxa_u32vec4>(state.readback_buffer).value()[tg_debug.readback_index * 2]; break;
                    }
                    auto floatvec_readback = device.buffer_host_address_as<daxa_f32vec4>(state.readback_buffer).value();
                    auto flt_min = std::numeric_limits<daxa_f32>::min();
                    auto flt_max = std::numeric_limits<daxa_f32>::max();
                    readback_color = floatvec_readback[tg_debug.readback_index * 2 + 1];
                }

                constexpr auto MOUSE_PICKER_FREEZE_COLOR = 0xFFBBFFFF;
                auto mouse_picker = [&](daxa_i32vec2 image_idx, bool frozen, Vec4Union readback_union)
                {
                    if (frozen)
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, MOUSE_PICKER_FREEZE_COLOR);
                    }
                    // ImGui::Dummy({0, 2});
                    constexpr auto MOUSE_PICKER_MAGNIFIER_TEXEL_WIDTH = 7;
                    constexpr auto MOUSE_PICKER_MAGNIFIER_DISPLAY_SIZE = ImVec2{70.0f, 70.0f};
                    daxa_i32vec2 image_idx_at_mip = {
                        image_idx.x >> state.mip,
                        image_idx.y >> state.mip,
                    };
                    ImVec2 magnify_start_uv = {
                        float(image_idx_at_mip.x - (MOUSE_PICKER_MAGNIFIER_TEXEL_WIDTH / 2)) * (1.0f / float(clone_image_info.size.x)),
                        float(image_idx_at_mip.y - (MOUSE_PICKER_MAGNIFIER_TEXEL_WIDTH / 2)) * (1.0f / float(clone_image_info.size.y)),
                    };
                    ImVec2 magnify_end_uv = {
                        float(image_idx_at_mip.x + MOUSE_PICKER_MAGNIFIER_TEXEL_WIDTH / 2 + 1) * (1.0f / float(clone_image_info.size.x)),
                        float(image_idx_at_mip.y + MOUSE_PICKER_MAGNIFIER_TEXEL_WIDTH / 2 + 1) * (1.0f / float(clone_image_info.size.y)),
                    };
                    if (tex_id && image_idx.x >= 0 && image_idx.y >= 0 && image_idx.x < image_info.size.x && image_idx.y < image_info.size.y)
                    {
                        ImGui::Image(tex_id, MOUSE_PICKER_MAGNIFIER_DISPLAY_SIZE, magnify_start_uv, magnify_end_uv);
                    }
                    if (frozen)
                    {
                        ImGui::PopStyleColor(1);
                    }
                    ImGui::SameLine();
                    daxa_i32vec2 index_at_mip = {
                        image_idx.x >> state.mip,
                        image_idx.y >> state.mip,
                    };
                    ImGui::Text("(%5d,%5d) %s\n%s",
                                index_at_mip.x,
                                index_at_mip.y,
                                frozen ? "FROZEN" : "      ",
                                format_vec4_rows(readback_union, scalar_kind).c_str());
                };

                ImGui::SeparatorText("Mouse Picker (?)\n");
                ImGui::SetItemTooltip(
                    "Usage:\n"
                    "  * left click on image to freeze selection, left click again to unfreeze\n"
                    "  * hold shift to replicate the selection on all other open inspector mouse pickers (also replicates freezes)\n"
                    "  * use middle mouse button to grab and move zoomed in image");
                if (state.display_image_hovered || state.freeze_image_hover_index || tg_debug.override_mouse_picker)
                {
                    mouse_picker(state.frozen_mouse_pos_relative_to_image_mip0, state.freeze_image_hover_index, state.frozen_readback_raw);
                }
                else
                {
                    mouse_picker(daxa_i32vec2{0, 0}, false, {});
                }

                ImGui::PopItemWidth();

                ImGui::TableNextColumn();
                ImGui::Text("slice used in task: %s", daxa::to_string(slice).c_str());
                ImGui::Text("size: (%i,%i,%i), mips: %i, layers: %i, format: %s",
                            image_info.size.x,
                            image_info.size.y,
                            image_info.size.z,
                            image_info.mip_level_count,
                            image_info.array_layer_count,
                            daxa::to_string(image_info.format).data());

                auto resolution_draw_modes = std::array{
                    "auto size",
                    "1x",
                    "1/2x",
                    "1/4x",
                    "1/8x",
                    "1/16x",
                    "2x",
                    "4x",
                    "8x",
                    "16x",
                };
                auto resolution_draw_mode_factors = std::array{
                    -1.0f,
                    1.0f,
                    1.0f / 2.0f,
                    1.0f / 4.0f,
                    1.0f / 8.0f,
                    1.0f / 16.0f,
                    2.0f,
                    4.0f,
                    8.0f,
                    16.0f,
                };
                ImGui::SetNextItemWidth(100.0f);
                ImGui::Combo("draw resolution mode", &state.resolution_draw_mode, resolution_draw_modes.data(), resolution_draw_modes.size());
                ImGui::SameLine();
                ImGui::Checkbox("fix mip sizes", &state.fixed_display_mip_sizes);
                ImGui::SetItemTooltip("fixes all displayed mip sizes to be the scaled size of mip 0");
                if (tex_id)
                {
                    float const aspect = static_cast<float>(clone_image_info.size.y) / static_cast<float>(clone_image_info.size.x);
                    ImVec2 const auto_sized_draw_size_x_based = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().x * aspect);
                    ImVec2 const auto_sized_draw_size_y_based = ImVec2(ImGui::GetContentRegionAvail().y / aspect, ImGui::GetContentRegionAvail().y);
                    ImVec2 const auto_sized_draw_size = auto_sized_draw_size_x_based.x < auto_sized_draw_size_y_based.x ? auto_sized_draw_size_x_based : auto_sized_draw_size_y_based;
                    ImVec2 image_display_size = auto_sized_draw_size;
                    if (state.resolution_draw_mode != 0)
                    {
                        ImVec2 fixed_size_draw_size = {};
                        if (state.fixed_display_mip_sizes)
                        {
                            fixed_size_draw_size.x = static_cast<float>(image_info.size.x) * resolution_draw_mode_factors[state.resolution_draw_mode];
                            fixed_size_draw_size.y = static_cast<float>(image_info.size.y) * resolution_draw_mode_factors[state.resolution_draw_mode];
                        }
                        else
                        {
                            fixed_size_draw_size.x = static_cast<float>(clone_image_info.size.x) * resolution_draw_mode_factors[state.resolution_draw_mode];
                            fixed_size_draw_size.y = static_cast<float>(clone_image_info.size.y) * resolution_draw_mode_factors[state.resolution_draw_mode];
                        };

                        image_display_size = fixed_size_draw_size;
                    }

                    ImVec2 start_pos = ImGui::GetCursorScreenPos();
                    state.display_image_size = daxa_i32vec2(image_display_size.x, image_display_size.y);
                    ImGui::BeginChild("scrollable image", ImVec2(0, 0), {}, ImGuiWindowFlags_HorizontalScrollbar);
                    ImVec2 scroll_offset = ImVec2{ImGui::GetScrollX(), ImGui::GetScrollY()};
                    if (state.display_image_hovered && ImGui::IsKeyDown(ImGuiKey_MouseMiddle))
                    {
                        ImGui::SetScrollX(ImGui::GetScrollX() - ImGui::GetIO().MouseDelta.x);
                        ImGui::SetScrollY(ImGui::GetScrollY() - ImGui::GetIO().MouseDelta.y);
                    }
                    ImGui::Image(tex_id, image_display_size);
                    ImVec2 const mouse_pos = ImGui::GetMousePos();
                    ImVec2 const end_pos = ImVec2{start_pos.x + image_display_size.x, start_pos.y + image_display_size.y};
                    ImVec2 const display_image_size = {
                        end_pos.x - start_pos.x,
                        end_pos.y - start_pos.y,
                    };
                    state.display_image_hovered = ImGui::IsMouseHoveringRect(start_pos, end_pos) && (ImGui::IsItemHovered() || ImGui::IsItemClicked());
                    state.freeze_image_hover_index = state.freeze_image_hover_index ^ (state.display_image_hovered && ImGui::IsItemClicked());
                    state.mouse_pos_relative_to_display_image = daxa_i32vec2(mouse_pos.x - start_pos.x, mouse_pos.y - start_pos.y);
                    tg_debug.request_mouse_picker_override |= state.display_image_hovered && ImGui::IsKeyDown(ImGuiKey_LeftShift);

                    bool const override_other_inspectors = tg_debug.override_mouse_picker && state.display_image_hovered;
                    bool const get_overriden = tg_debug.override_mouse_picker && !state.display_image_hovered;
                    if (override_other_inspectors)
                    {
                        tg_debug.override_frozen_state = state.freeze_image_hover_index;
                        tg_debug.override_mouse_picker_uv = {
                            float(state.mouse_pos_relative_to_display_image.x) / display_image_size.x,
                            float(state.mouse_pos_relative_to_display_image.y) / display_image_size.y,
                        };
                    }
                    if (get_overriden)
                    {
                        state.freeze_image_hover_index = tg_debug.override_frozen_state;
                        state.mouse_pos_relative_to_display_image = {
                            daxa_i32(tg_debug.override_mouse_picker_uv.x * display_image_size.x),
                            daxa_i32(tg_debug.override_mouse_picker_uv.y * display_image_size.y),
                        };
                    }

                    state.mouse_pos_relative_to_image_mip0 = daxa_i32vec2(
                        ((state.mouse_pos_relative_to_display_image.x + scroll_offset.x) / static_cast<float>(state.display_image_size.x)) * static_cast<float>(image_info.size.x),
                        ((state.mouse_pos_relative_to_display_image.y + scroll_offset.y) / static_cast<float>(state.display_image_size.y)) * static_cast<float>(image_info.size.y));

                    if (!state.freeze_image_hover_index)
                    {
                        state.frozen_mouse_pos_relative_to_image_mip0 = state.mouse_pos_relative_to_image_mip0;
                        state.frozen_readback_raw = readback_raw;
                        state.frozen_readback_color = readback_color;
                    }
                    if (state.display_image_hovered)
                    {
                        ImGui::BeginTooltip();
                        mouse_picker(state.mouse_pos_relative_to_image_mip0, false, readback_raw);
                        ImGui::EndTooltip();
                    }
                    if (state.display_image_hovered || tg_debug.override_mouse_picker)
                    {
                        ImVec2 const frozen_mouse_pos_relative_to_display_image = {
                            float(state.mouse_pos_relative_to_image_mip0.x) / float(image_info.size.x) * display_image_size.x,
                            float(state.mouse_pos_relative_to_image_mip0.y) / float(image_info.size.y) * display_image_size.y,
                        };
                        ImVec2 const window_marker_pos = {
                            start_pos.x + frozen_mouse_pos_relative_to_display_image.x,
                            start_pos.y + frozen_mouse_pos_relative_to_display_image.y,
                        };
                        ImGui::GetWindowDrawList()->AddCircle(
                            window_marker_pos, 5.0f,
                            ImGui::GetColorU32(ImVec4{
                                readback_color.x > 0.5f ? 0.0f : 1.0f,
                                readback_color.y > 0.5f ? 0.0f : 1.0f,
                                readback_color.z > 0.5f ? 0.0f : 1.0f,
                                1.0f,
                            }));
                    }
                    if (state.freeze_image_hover_index)
                    {
                        ImVec2 const frozen_mouse_pos_relative_to_display_image = {
                            float(state.frozen_mouse_pos_relative_to_image_mip0.x) / float(image_info.size.x) * display_image_size.x,
                            float(state.frozen_mouse_pos_relative_to_image_mip0.y) / float(image_info.size.y) * display_image_size.y,
                        };
                        ImVec2 const window_marker_pos = {
                            start_pos.x + frozen_mouse_pos_relative_to_display_image.x,
                            start_pos.y + frozen_mouse_pos_relative_to_display_image.y,
                        };
                        auto inv_color = ImVec4{
                            state.frozen_readback_color.x > 0.5f ? 0.0f : 1.0f,
                            state.frozen_readback_color.y > 0.5f ? 0.0f : 1.0f,
                            state.frozen_readback_color.z > 0.5f ? 0.0f : 1.0f,
                            1.0f,
                        };
                        ImGui::GetWindowDrawList()->AddCircle(window_marker_pos, 5.0f, ImGui::GetColorU32(inv_color));
                    }
                    ImGui::EndChild();
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();
    }

    return open;
}

void TgDebugContext::debug_ui(daxa::ImGuiRenderer & imgui_renderer)
{
    auto & tg_debug = *this;

    if (ImGui::Begin("TG Debug Clones", nullptr, ImGuiWindowFlags_NoCollapse))
    {
        bool const clear_search = ImGui::Button("clear");
        if (clear_search)
            tg_debug.search_substr = {};
        ImGui::SameLine();
        ImGui::SetNextItemWidth(200);
        ImGui::InputText("Search for Task", tg_debug.search_substr.data(), tg_debug.search_substr.size());
        for (auto & c : tg_debug.search_substr)
            c = std::tolower(c);

        bool const search_used = tg_debug.search_substr[0] != '\0';

        ImGui::BeginChild("Tasks");
        for (auto task : tg_debug.this_frame_task_attachments)
        {
            if (task.task_name.size() == 0 || task.task_name.c_str()[0] == 0)
                continue;

            if (search_used)
            {
                std::string compare_string = task.task_name;
                for (auto & c : compare_string)
                    c = std::tolower(c);
                if (!strstr(compare_string.c_str(), tg_debug.search_substr.data()))
                    continue;
            }

            bool has_image_attachment = false;
            for (auto attach : task.attachments)
            {
                if (attach.type == daxa::TaskAttachmentType::IMAGE)
                {
                    has_image_attachment = true;
                    break;
                }
            }

            if (!has_image_attachment)
            {
                ImGui::Text("* %s", task.task_name.c_str());
                continue;
            }

            if (ImGui::CollapsingHeader(task.task_name.c_str()))
            {
                ImGui::PushID(task.task_name.c_str());
                int inl_attachment_index = 0;
                for (auto attach : task.attachments)
                {
                    std::string inspector_key = task.task_name + "::AT." + attach.name();
                    if (strcmp(attach.name(), "inline attachment") == 0)
                    {
                        inspector_key += " ";
                        inspector_key += std::to_string(inl_attachment_index);
                        ++inl_attachment_index;
                    }
                    ImGui::PushID(inspector_key.c_str());
                    if (ImGui::Button(attach.name()))
                    {
                        bool already_active = tg_debug.inspector_states[inspector_key].active;
                        if (already_active)
                        {
                            auto iter = tg_debug.active_inspectors.find(inspector_key);
                            if (iter != tg_debug.active_inspectors.end())
                            {
                                tg_debug.active_inspectors.erase(iter);
                            }
                            tg_debug.inspector_states[inspector_key].active = false;
                        }
                        else
                        {
                            tg_debug.active_inspectors.emplace(inspector_key);

                            {
                                auto & state = tg_debug.inspector_states[inspector_key];
                                auto const & slice = attach.value.image.view.slice;
                                state.mip = std::min(std::max(state.mip, slice.base_mip_level), slice.base_mip_level + slice.level_count);
                                state.layer = std::min(std::max(state.layer, slice.base_array_layer), slice.base_array_layer + slice.layer_count);
                                state.active = true;
                            }
                        }
                    }
                    ImGui::SameLine();
                    switch (attach.type)
                    {
                    case daxa::TaskAttachmentType::IMAGE:
                        ImGui::Text("| view: %s", daxa::to_string(attach.value.image.view.slice).c_str());
                        ImGui::SameLine();
                        ImGui::Text("| task access: %s", std::string(daxa::to_string(attach.value.image.task_access)).c_str());
                        break;
                    default: break;
                    }
                    ImGui::PopID();
                }

                ImGui::PopID();
            }
        }
        ImGui::EndChild();

        ImGui::End();
        for (auto active_inspector_key : tg_debug.active_inspectors)
        {
            if (!debug_ui_image_inspector(imgui_renderer, active_inspector_key))
            {
                // remove
                // auto iter = tg_debug.active_inspectors.find(inspector_key);
                // if (iter != tg_debug.active_inspectors.end())
                // {
                //     tg_debug.active_inspectors.erase(iter);
                // }
            }
        }
    }
    if (tg_debug.request_mouse_picker_override)
    {
        tg_debug.override_mouse_picker = true;
    }
    else
    {
        tg_debug.override_mouse_picker = false;
    }
    tg_debug.request_mouse_picker_override = false;
    tg_debug.this_frame_task_attachments.clear();
    tg_debug.this_frame_duplicate_task_name_counter.clear();
}
