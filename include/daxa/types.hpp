#pragma once

#include <cstdint>
#include <cstddef>
#include <array>

namespace daxa
{
    inline namespace types
    {
        using u8 = std::uint8_t;
        using u16 = std::uint16_t;
        using u32 = std::uint32_t;
        using u64 = std::uint64_t;
        using usize = std::size_t;

        using i8 = std::int8_t;
        using i16 = std::int16_t;
        using i32 = std::int32_t;
        using i64 = std::int64_t;
        using isize = std::ptrdiff_t;

        using f32 = float;
        using f64 = double;
    } // namespace types

    enum class Format
    {
        UNDEFINED = 0,
        R4G4_UNORM_PACK8 = 1,
        R4G4B4A4_UNORM_PACK16 = 2,
        B4G4R4A4_UNORM_PACK16 = 3,
        R5G6B5_UNORM_PACK16 = 4,
        B5G6R5_UNORM_PACK16 = 5,
        R5G5B5A1_UNORM_PACK16 = 6,
        B5G5R5A1_UNORM_PACK16 = 7,
        A1R5G5B5_UNORM_PACK16 = 8,
        R8_UNORM = 9,
        R8_SNORM = 10,
        R8_USCALED = 11,
        R8_SSCALED = 12,
        R8_UINT = 13,
        R8_SINT = 14,
        R8_SRGB = 15,
        R8G8_UNORM = 16,
        R8G8_SNORM = 17,
        R8G8_USCALED = 18,
        R8G8_SSCALED = 19,
        R8G8_UINT = 20,
        R8G8_SINT = 21,
        R8G8_SRGB = 22,
        R8G8B8_UNORM = 23,
        R8G8B8_SNORM = 24,
        R8G8B8_USCALED = 25,
        R8G8B8_SSCALED = 26,
        R8G8B8_UINT = 27,
        R8G8B8_SINT = 28,
        R8G8B8_SRGB = 29,
        B8G8R8_UNORM = 30,
        B8G8R8_SNORM = 31,
        B8G8R8_USCALED = 32,
        B8G8R8_SSCALED = 33,
        B8G8R8_UINT = 34,
        B8G8R8_SINT = 35,
        B8G8R8_SRGB = 36,
        R8G8B8A8_UNORM = 37,
        R8G8B8A8_SNORM = 38,
        R8G8B8A8_USCALED = 39,
        R8G8B8A8_SSCALED = 40,
        R8G8B8A8_UINT = 41,
        R8G8B8A8_SINT = 42,
        R8G8B8A8_SRGB = 43,
        B8G8R8A8_UNORM = 44,
        B8G8R8A8_SNORM = 45,
        B8G8R8A8_USCALED = 46,
        B8G8R8A8_SSCALED = 47,
        B8G8R8A8_UINT = 48,
        B8G8R8A8_SINT = 49,
        B8G8R8A8_SRGB = 50,
        A8B8G8R8_UNORM_PACK32 = 51,
        A8B8G8R8_SNORM_PACK32 = 52,
        A8B8G8R8_USCALED_PACK32 = 53,
        A8B8G8R8_SSCALED_PACK32 = 54,
        A8B8G8R8_UINT_PACK32 = 55,
        A8B8G8R8_SINT_PACK32 = 56,
        A8B8G8R8_SRGB_PACK32 = 57,
        A2R10G10B10_UNORM_PACK32 = 58,
        A2R10G10B10_SNORM_PACK32 = 59,
        A2R10G10B10_USCALED_PACK32 = 60,
        A2R10G10B10_SSCALED_PACK32 = 61,
        A2R10G10B10_UINT_PACK32 = 62,
        A2R10G10B10_SINT_PACK32 = 63,
        A2B10G10R10_UNORM_PACK32 = 64,
        A2B10G10R10_SNORM_PACK32 = 65,
        A2B10G10R10_USCALED_PACK32 = 66,
        A2B10G10R10_SSCALED_PACK32 = 67,
        A2B10G10R10_UINT_PACK32 = 68,
        A2B10G10R10_SINT_PACK32 = 69,
        R16_UNORM = 70,
        R16_SNORM = 71,
        R16_USCALED = 72,
        R16_SSCALED = 73,
        R16_UINT = 74,
        R16_SINT = 75,
        R16_SFLOAT = 76,
        R16G16_UNORM = 77,
        R16G16_SNORM = 78,
        R16G16_USCALED = 79,
        R16G16_SSCALED = 80,
        R16G16_UINT = 81,
        R16G16_SINT = 82,
        R16G16_SFLOAT = 83,
        R16G16B16_UNORM = 84,
        R16G16B16_SNORM = 85,
        R16G16B16_USCALED = 86,
        R16G16B16_SSCALED = 87,
        R16G16B16_UINT = 88,
        R16G16B16_SINT = 89,
        R16G16B16_SFLOAT = 90,
        R16G16B16A16_UNORM = 91,
        R16G16B16A16_SNORM = 92,
        R16G16B16A16_USCALED = 93,
        R16G16B16A16_SSCALED = 94,
        R16G16B16A16_UINT = 95,
        R16G16B16A16_SINT = 96,
        R16G16B16A16_SFLOAT = 97,
        R32_UINT = 98,
        R32_SINT = 99,
        R32_SFLOAT = 100,
        R32G32_UINT = 101,
        R32G32_SINT = 102,
        R32G32_SFLOAT = 103,
        R32G32B32_UINT = 104,
        R32G32B32_SINT = 105,
        R32G32B32_SFLOAT = 106,
        R32G32B32A32_UINT = 107,
        R32G32B32A32_SINT = 108,
        R32G32B32A32_SFLOAT = 109,
        R64_UINT = 110,
        R64_SINT = 111,
        R64_SFLOAT = 112,
        R64G64_UINT = 113,
        R64G64_SINT = 114,
        R64G64_SFLOAT = 115,
        R64G64B64_UINT = 116,
        R64G64B64_SINT = 117,
        R64G64B64_SFLOAT = 118,
        R64G64B64A64_UINT = 119,
        R64G64B64A64_SINT = 120,
        R64G64B64A64_SFLOAT = 121,
        B10G11R11_UFLOAT_PACK32 = 122,
        E5B9G9R9_UFLOAT_PACK32 = 123,
        D16_UNORM = 124,
        X8_D24_UNORM_PACK32 = 125,
        D32_SFLOAT = 126,
        S8_UINT = 127,
        D16_UNORM_S8_UINT = 128,
        D24_UNORM_S8_UINT = 129,
        D32_SFLOAT_S8_UINT = 130,
        BC1_RGB_UNORM_BLOCK = 131,
        BC1_RGB_SRGB_BLOCK = 132,
        BC1_RGBA_UNORM_BLOCK = 133,
        BC1_RGBA_SRGB_BLOCK = 134,
        BC2_UNORM_BLOCK = 135,
        BC2_SRGB_BLOCK = 136,
        BC3_UNORM_BLOCK = 137,
        BC3_SRGB_BLOCK = 138,
        BC4_UNORM_BLOCK = 139,
        BC4_SNORM_BLOCK = 140,
        BC5_UNORM_BLOCK = 141,
        BC5_SNORM_BLOCK = 142,
        BC6H_UFLOAT_BLOCK = 143,
        BC6H_SFLOAT_BLOCK = 144,
        BC7_UNORM_BLOCK = 145,
        BC7_SRGB_BLOCK = 146,
        ETC2_R8G8B8_UNORM_BLOCK = 147,
        ETC2_R8G8B8_SRGB_BLOCK = 148,
        ETC2_R8G8B8A1_UNORM_BLOCK = 149,
        ETC2_R8G8B8A1_SRGB_BLOCK = 150,
        ETC2_R8G8B8A8_UNORM_BLOCK = 151,
        ETC2_R8G8B8A8_SRGB_BLOCK = 152,
        EAC_R11_UNORM_BLOCK = 153,
        EAC_R11_SNORM_BLOCK = 154,
        EAC_R11G11_UNORM_BLOCK = 155,
        EAC_R11G11_SNORM_BLOCK = 156,
        ASTC_4x4_UNORM_BLOCK = 157,
        ASTC_4x4_SRGB_BLOCK = 158,
        ASTC_5x4_UNORM_BLOCK = 159,
        ASTC_5x4_SRGB_BLOCK = 160,
        ASTC_5x5_UNORM_BLOCK = 161,
        ASTC_5x5_SRGB_BLOCK = 162,
        ASTC_6x5_UNORM_BLOCK = 163,
        ASTC_6x5_SRGB_BLOCK = 164,
        ASTC_6x6_UNORM_BLOCK = 165,
        ASTC_6x6_SRGB_BLOCK = 166,
        ASTC_8x5_UNORM_BLOCK = 167,
        ASTC_8x5_SRGB_BLOCK = 168,
        ASTC_8x6_UNORM_BLOCK = 169,
        ASTC_8x6_SRGB_BLOCK = 170,
        ASTC_8x8_UNORM_BLOCK = 171,
        ASTC_8x8_SRGB_BLOCK = 172,
        ASTC_10x5_UNORM_BLOCK = 173,
        ASTC_10x5_SRGB_BLOCK = 174,
        ASTC_10x6_UNORM_BLOCK = 175,
        ASTC_10x6_SRGB_BLOCK = 176,
        ASTC_10x8_UNORM_BLOCK = 177,
        ASTC_10x8_SRGB_BLOCK = 178,
        ASTC_10x10_UNORM_BLOCK = 179,
        ASTC_10x10_SRGB_BLOCK = 180,
        ASTC_12x10_UNORM_BLOCK = 181,
        ASTC_12x10_SRGB_BLOCK = 182,
        ASTC_12x12_UNORM_BLOCK = 183,
        ASTC_12x12_SRGB_BLOCK = 184,
        G8B8G8R8_422_UNORM = 1000156000,
        B8G8R8G8_422_UNORM = 1000156001,
        G8_B8_R8_3PLANE_420_UNORM = 1000156002,
        G8_B8R8_2PLANE_420_UNORM = 1000156003,
        G8_B8_R8_3PLANE_422_UNORM = 1000156004,
        G8_B8R8_2PLANE_422_UNORM = 1000156005,
        G8_B8_R8_3PLANE_444_UNORM = 1000156006,
        R10X6_UNORM_PACK16 = 1000156007,
        R10X6G10X6_UNORM_2PACK16 = 1000156008,
        R10X6G10X6B10X6A10X6_UNORM_4PACK16 = 1000156009,
        G10X6B10X6G10X6R10X6_422_UNORM_4PACK16 = 1000156010,
        B10X6G10X6R10X6G10X6_422_UNORM_4PACK16 = 1000156011,
        G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16 = 1000156012,
        G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16 = 1000156013,
        G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16 = 1000156014,
        G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16 = 1000156015,
        G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16 = 1000156016,
        R12X4_UNORM_PACK16 = 1000156017,
        R12X4G12X4_UNORM_2PACK16 = 1000156018,
        R12X4G12X4B12X4A12X4_UNORM_4PACK16 = 1000156019,
        G12X4B12X4G12X4R12X4_422_UNORM_4PACK16 = 1000156020,
        B12X4G12X4R12X4G12X4_422_UNORM_4PACK16 = 1000156021,
        G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16 = 1000156022,
        G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16 = 1000156023,
        G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16 = 1000156024,
        G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16 = 1000156025,
        G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16 = 1000156026,
        G16B16G16R16_422_UNORM = 1000156027,
        B16G16R16G16_422_UNORM = 1000156028,
        G16_B16_R16_3PLANE_420_UNORM = 1000156029,
        G16_B16R16_2PLANE_420_UNORM = 1000156030,
        G16_B16_R16_3PLANE_422_UNORM = 1000156031,
        G16_B16R16_2PLANE_422_UNORM = 1000156032,
        G16_B16_R16_3PLANE_444_UNORM = 1000156033,
        G8_B8R8_2PLANE_444_UNORM = 1000330000,
        G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16 = 1000330001,
        G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16 = 1000330002,
        G16_B16R16_2PLANE_444_UNORM = 1000330003,
        A4R4G4B4_UNORM_PACK16 = 1000340000,
        A4B4G4R4_UNORM_PACK16 = 1000340001,
        ASTC_4x4_SFLOAT_BLOCK = 1000066000,
        ASTC_5x4_SFLOAT_BLOCK = 1000066001,
        ASTC_5x5_SFLOAT_BLOCK = 1000066002,
        ASTC_6x5_SFLOAT_BLOCK = 1000066003,
        ASTC_6x6_SFLOAT_BLOCK = 1000066004,
        ASTC_8x5_SFLOAT_BLOCK = 1000066005,
        ASTC_8x6_SFLOAT_BLOCK = 1000066006,
        ASTC_8x8_SFLOAT_BLOCK = 1000066007,
        ASTC_10x5_SFLOAT_BLOCK = 1000066008,
        ASTC_10x6_SFLOAT_BLOCK = 1000066009,
        ASTC_10x8_SFLOAT_BLOCK = 1000066010,
        ASTC_10x10_SFLOAT_BLOCK = 1000066011,
        ASTC_12x10_SFLOAT_BLOCK = 1000066012,
        ASTC_12x12_SFLOAT_BLOCK = 1000066013,
        PVRTC1_2BPP_UNORM_BLOCK_IMG = 1000054000,
        PVRTC1_4BPP_UNORM_BLOCK_IMG = 1000054001,
        PVRTC2_2BPP_UNORM_BLOCK_IMG = 1000054002,
        PVRTC2_4BPP_UNORM_BLOCK_IMG = 1000054003,
        PVRTC1_2BPP_SRGB_BLOCK_IMG = 1000054004,
        PVRTC1_4BPP_SRGB_BLOCK_IMG = 1000054005,
        PVRTC2_2BPP_SRGB_BLOCK_IMG = 1000054006,
        PVRTC2_4BPP_SRGB_BLOCK_IMG = 1000054007,
        ASTC_4x4_SFLOAT_BLOCK_EXT = ASTC_4x4_SFLOAT_BLOCK,
        ASTC_5x4_SFLOAT_BLOCK_EXT = ASTC_5x4_SFLOAT_BLOCK,
        ASTC_5x5_SFLOAT_BLOCK_EXT = ASTC_5x5_SFLOAT_BLOCK,
        ASTC_6x5_SFLOAT_BLOCK_EXT = ASTC_6x5_SFLOAT_BLOCK,
        ASTC_6x6_SFLOAT_BLOCK_EXT = ASTC_6x6_SFLOAT_BLOCK,
        ASTC_8x5_SFLOAT_BLOCK_EXT = ASTC_8x5_SFLOAT_BLOCK,
        ASTC_8x6_SFLOAT_BLOCK_EXT = ASTC_8x6_SFLOAT_BLOCK,
        ASTC_8x8_SFLOAT_BLOCK_EXT = ASTC_8x8_SFLOAT_BLOCK,
        ASTC_10x5_SFLOAT_BLOCK_EXT = ASTC_10x5_SFLOAT_BLOCK,
        ASTC_10x6_SFLOAT_BLOCK_EXT = ASTC_10x6_SFLOAT_BLOCK,
        ASTC_10x8_SFLOAT_BLOCK_EXT = ASTC_10x8_SFLOAT_BLOCK,
        ASTC_10x10_SFLOAT_BLOCK_EXT = ASTC_10x10_SFLOAT_BLOCK,
        ASTC_12x10_SFLOAT_BLOCK_EXT = ASTC_12x10_SFLOAT_BLOCK,
        ASTC_12x12_SFLOAT_BLOCK_EXT = ASTC_12x12_SFLOAT_BLOCK,
        G8B8G8R8_422_UNORM_KHR = G8B8G8R8_422_UNORM,
        B8G8R8G8_422_UNORM_KHR = B8G8R8G8_422_UNORM,
        G8_B8_R8_3PLANE_420_UNORM_KHR = G8_B8_R8_3PLANE_420_UNORM,
        G8_B8R8_2PLANE_420_UNORM_KHR = G8_B8R8_2PLANE_420_UNORM,
        G8_B8_R8_3PLANE_422_UNORM_KHR = G8_B8_R8_3PLANE_422_UNORM,
        G8_B8R8_2PLANE_422_UNORM_KHR = G8_B8R8_2PLANE_422_UNORM,
        G8_B8_R8_3PLANE_444_UNORM_KHR = G8_B8_R8_3PLANE_444_UNORM,
        R10X6_UNORM_PACK16_KHR = R10X6_UNORM_PACK16,
        R10X6G10X6_UNORM_2PACK16_KHR = R10X6G10X6_UNORM_2PACK16,
        R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR = R10X6G10X6B10X6A10X6_UNORM_4PACK16,
        G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR = G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
        B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR = B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
        G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR = G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
        G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR = G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
        G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR = G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
        G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR = G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
        G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR = G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
        R12X4_UNORM_PACK16_KHR = R12X4_UNORM_PACK16,
        R12X4G12X4_UNORM_2PACK16_KHR = R12X4G12X4_UNORM_2PACK16,
        R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR = R12X4G12X4B12X4A12X4_UNORM_4PACK16,
        G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR = G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
        B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR = B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
        G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR = G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
        G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR = G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
        G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR = G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
        G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR = G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
        G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR = G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
        G16B16G16R16_422_UNORM_KHR = G16B16G16R16_422_UNORM,
        B16G16R16G16_422_UNORM_KHR = B16G16R16G16_422_UNORM,
        G16_B16_R16_3PLANE_420_UNORM_KHR = G16_B16_R16_3PLANE_420_UNORM,
        G16_B16R16_2PLANE_420_UNORM_KHR = G16_B16R16_2PLANE_420_UNORM,
        G16_B16_R16_3PLANE_422_UNORM_KHR = G16_B16_R16_3PLANE_422_UNORM,
        G16_B16R16_2PLANE_422_UNORM_KHR = G16_B16R16_2PLANE_422_UNORM,
        G16_B16_R16_3PLANE_444_UNORM_KHR = G16_B16_R16_3PLANE_444_UNORM,
        G8_B8R8_2PLANE_444_UNORM_EXT = G8_B8R8_2PLANE_444_UNORM,
        G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT = G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16,
        G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT = G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16,
        G16_B16R16_2PLANE_444_UNORM_EXT = G16_B16R16_2PLANE_444_UNORM,
        A4R4G4B4_UNORM_PACK16_EXT = A4R4G4B4_UNORM_PACK16,
        A4B4G4R4_UNORM_PACK16_EXT = A4B4G4R4_UNORM_PACK16,
    };

    enum class MsgSeverity
    {
        VERBOSE = 0x00000001,
        INFO = 0x00000010,
        WARNING = 0x00000100,
        FAILURE = 0x00001000,
    };

    enum class MsgType
    {
        GENERAL = 0x00000001,
        VALIDATION = 0x00000002,
        PERFORMANCE = 0x00000004,
    };

    enum class PresentMode
    {
        DO_NOT_WAIT_FOR_VBLANK = 0,
        TRIPPLE_BUFFER_WAIT_FOR_VBLANK = 1,
        DOUBLE_BUFFER_WAIT_FOR_VBLANK = 2,
        DOUBLE_BUFFER_WAIT_FOR_VBLANK_RELAXED = 3,
    };

    enum class PresentOp
    {
        IDENTITY = 0x00000001,
        ROTATE_90 = 0x00000002,
        ROTATE_180 = 0x00000004,
        ROTATE_270 = 0x00000008,
        HORIZONTAL_MIRROR = 0x00000010,
        HORIZONTAL_MIRROR_ROTATE_90 = 0x00000020,
        HORIZONTAL_MIRROR_ROTATE_180 = 0x00000040,
        HORIZONTAL_MIRROR_ROTATE_270 = 0x00000080,
        INHERIT = 0x00000100,
    };

    using ImageUsageFlags = u32;
    struct ImageUsageFlagBits
    {
        static inline constexpr ImageUsageFlags TRANSFER_SRC = 0x00000001;
        static inline constexpr ImageUsageFlags TRANSFER_DST = 0x00000002;
        static inline constexpr ImageUsageFlags SAMPLED = 0x00000004;
        static inline constexpr ImageUsageFlags STORAGE = 0x00000008;
        static inline constexpr ImageUsageFlags COLOR_ATTACHMENT = 0x00000010;
        static inline constexpr ImageUsageFlags DEPTH_STENCIL_ATTACHMENT = 0x00000020;
        static inline constexpr ImageUsageFlags TRANSIENT_ATTACHMENT = 0x00000040;
        static inline constexpr ImageUsageFlags FRAGMENT_DENSITY_MAP = 0x00000200;
        static inline constexpr ImageUsageFlags FRAGMENT_SHADING_RATE_ATTACHMENT = 0x00000100;
        static inline constexpr ImageUsageFlags SHADING_RATE_IMAGE = FRAGMENT_SHADING_RATE_ATTACHMENT;
    };

    using MemoryFlags = u32;
    struct MemoryFlagBits {
        static inline constexpr MemoryFlags DEDICATED_MEMORY = 0x00000001;
        static inline constexpr MemoryFlags CAN_ALIAS = 0x00000200;
        static inline constexpr MemoryFlags HOST_ACCESS_SEQUENTIAL_WRITE = 0x00000400;
        static inline constexpr MemoryFlags HOST_ACCESS_RANDOM = 0x00000800;
        static inline constexpr MemoryFlags STRATEGY_MIN_MEMORY = 0x00010000;
        static inline constexpr MemoryFlags STRATEGY_MIN_TIME = 0x00020000;
    };

    enum class ColorSpace
    {
        SRGB_NONLINEAR = 0,
        DISPLAY_P3_NONLINEAR = 1000104001,
        EXTENDED_SRGB_LINEAR = 1000104002,
        DISPLAY_P3_LINEAR = 1000104003,
        DCI_P3_NONLINEAR = 1000104004,
        BT709_LINEAR = 1000104005,
        BT709_NONLINEAR = 1000104006,
        BT2020_LINEAR = 1000104007,
        HDR10_ST2084 = 1000104008,
        DOLBYVISION = 1000104009,
        HDR10_HLG = 1000104010,
        ADOBERGB_LINEAR = 1000104011,
        ADOBERGB_NONLINEAR = 1000104012,
        PASS_THROUGH = 1000104013,
        EXTENDED_SRGB_NONLINEAR = 1000104014,
        DISPLAY_NATIVE = 1000213000,
        RGB_NONLINEAR = EXTENDED_SRGB_NONLINEAR,
        DCI_P3_LINEAR = DISPLAY_P3_LINEAR,
    };

    using ImageAspectFlags = u32;
    struct ImageAspectFlagBits
    {
        static inline constexpr ImageAspectFlags NONE = {};
        static inline constexpr ImageAspectFlags COLOR = 0x00000001;
        static inline constexpr ImageAspectFlags DEPTH = 0x00000002;
        static inline constexpr ImageAspectFlags STENCIL = 0x00000004;
        static inline constexpr ImageAspectFlags METADATA = 0x00000008;
        static inline constexpr ImageAspectFlags PLANE_0 = 0x00000010;
        static inline constexpr ImageAspectFlags PLANE_1 = 0x00000020;
        static inline constexpr ImageAspectFlags PLANE_2 = 0x00000040;
    };

    enum class ImageLayout
    {
        UNDEFINED = 0,
        GENERAL = 1,
        COLOR_ATTACHMENT_OPTIMAL = 2,
        DEPTH_STENCIL_ATTACHMENT_OPTIMAL = 3,
        DEPTH_STENCIL_READ_ONLY_OPTIMAL = 4,
        SHADER_READ_ONLY_OPTIMAL = 5,
        TRANSFER_SRC_OPTIMAL = 6,
        TRANSFER_DST_OPTIMAL = 7,
        PREINITIALIZED = 8,
        DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL = 1000117000,
        DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL = 1000117001,
        DEPTH_ATTACHMENT_OPTIMAL = 1000241000,
        DEPTH_READ_ONLY_OPTIMAL = 1000241001,
        STENCIL_ATTACHMENT_OPTIMAL = 1000241002,
        STENCIL_READ_ONLY_OPTIMAL = 1000241003,
        READ_ONLY_OPTIMAL = 1000314000,
        ATTACHMENT_OPTIMAL = 1000314001,
        PRESENT_SRC = 1000001002,
        SHARED_PRESENT = 1000111000,
        FRAGMENT_DENSITY_MAP_OPTIMAL_EXT = 1000218000,
        FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL = 1000164003,
    };

    struct ImageMipArraySlice
    {
        ImageAspectFlags image_aspect = ImageAspectFlagBits::COLOR;
        u32 base_mip_level = 0;
        u32 level_count = 1;
        u32 base_array_layer = 0;
        u32 layer_count = 1;
    };
    struct ImageArraySlice
    {
        ImageAspectFlags image_aspect = ImageAspectFlagBits::COLOR;
        u32 mip_level = 0;
        u32 base_array_layer = 0;
        u32 layer_count = 1;
    };
    struct ImageSlice
    {
        ImageAspectFlags image_aspect = ImageAspectFlagBits::COLOR;
        u32 mip_level = 0;
        u32 array_layer = 0;
    };

    enum class Filter
    {
        NEAREST = 0,
        LINEAR = 1,
        CUBIC_IMG = 1000015000,
    };

    struct Offset3D
    {
        i32 x = {};
        i32 y = {};
        i32 z = {};
    };

    struct Extent3D
    {
        u32 x = {};
        u32 y = {};
        u32 z = {};
    };

    union ClearColor
    {
        std::array<f32, 4> f32_value;
        std::array<i32, 4> i32_value;
        std::array<u32, 4> u32_value;
        struct
        {
            f32 depth;
            u32 stencil;
        } depth_stencil;
    };

    using PipelineStageAccessFlags = u64;
    struct PipelineStageAccessFlagBits
    {
        static inline constexpr u64 READ_ACCESS = 0x8000'0000'0000'0000;
        static inline constexpr u64 WRITE_ACCESS = 0x4000'0000'0000'0000;

        static inline constexpr PipelineStageAccessFlags NONE = 0x00000000ull;
        static inline constexpr PipelineStageAccessFlags TOP_OF_PIPE_READ = 0x00000001ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags DRAW_INDIRECT_READ = 0x00000002ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags VERTEX_INPUT_READ = 0x00000004ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags VERTEX_SHADER_READ = 0x00000008ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags TESSELLATION_CONTROL_SHADER_READ = 0x00000010ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags TESSELLATION_EVALUATION_SHADER_READ = 0x00000020ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags GEOMETRY_SHADER_READ = 0x00000040ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags FRAGMENT_SHADER_READ = 0x00000080ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags EARLY_FRAGMENT_TESTS_READ = 0x00000100ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags LATE_FRAGMENT_TESTS_READ = 0x00000200ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags COLOR_ATTACHMENT_OUTPUT_READ = 0x00000400ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags COMPUTE_SHADER_READ = 0x00000800ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags ALL_TRANSFER_READ = 0x00001000ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags TRANSFER_READ = 0x00001000ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags BOTTOM_OF_PIPE_READ = 0x00002000ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags HOST_READ = 0x00004000ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags ALL_GRAPHICS_READ = 0x00008000ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags ALL_COMMANDS_READ = 0x00010000ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags COPY_READ = 0x100000000ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags RESOLVE_READ = 0x200000000ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags BLIT_READ = 0x400000000ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags CLEAR_READ = 0x800000000ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags INDEX_INPUT_READ = 0x1000000000ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags VERTEX_ATTRIBUTE_INPUT_READ = 0x2000000000ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags PRE_RASTERIZATION_SHADERS_READ = 0x4000000000ull | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags TOP_OF_PIPE_WRITE = 0x00000001ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags DRAW_INDIRECT_WRITE = 0x00000002ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags VERTEX_INPUT_WRITE = 0x00000004ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags VERTEX_SHADER_WRITE = 0x00000008ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags TESSELLATION_CONTROL_SHADER_WRITE = 0x00000010ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags TESSELLATION_EVALUATION_SHADER_WRITE = 0x00000020ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags GEOMETRY_SHADER_WRITE = 0x00000040ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags FRAGMENT_SHADER_WRITE = 0x00000080ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags EARLY_FRAGMENT_TESTS_WRITE = 0x00000100ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags LATE_FRAGMENT_TESTS_WRITE = 0x00000200ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags COLOR_ATTACHMENT_OUTPUT_WRITE = 0x00000400ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags COMPUTE_SHADER_WRITE = 0x00000800ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags ALL_TRANSFER_WRITE = 0x00001000ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags TRANSFER_WRITE = 0x00001000ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags BOTTOM_OF_PIPE_WRITE = 0x00002000ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags HOST_WRITE = 0x00004000ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags ALL_GRAPHICS_WRITE = 0x00008000ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags ALL_COMMANDS_WRITE = 0x00010000ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags COPY_WRITE = 0x100000000ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags RESOLVE_WRITE = 0x200000000ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags BLIT_WRITE = 0x400000000ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags CLEAR_WRITE = 0x800000000ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags INDEX_INPUT_WRITE = 0x1000000000ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags VERTEX_ATTRIBUTE_INPUT_WRITE = 0x2000000000ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags PRE_RASTERIZATION_SHADERS_WRITE = 0x4000000000ull | WRITE_ACCESS;
        static inline constexpr PipelineStageAccessFlags TOP_OF_PIPE_READ_WRITE = 0x00000001ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags DRAW_INDIRECT_READ_WRITE = 0x00000002ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags VERTEX_INPUT_READ_WRITE = 0x00000004ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags VERTEX_SHADER_READ_WRITE = 0x00000008ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags TESSELLATION_CONTROL_SHADER_READ_WRITE = 0x00000010ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags TESSELLATION_EVALUATION_SHADER_READ_WRITE = 0x00000020ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags GEOMETRY_SHADER_READ_WRITE = 0x00000040ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags FRAGMENT_SHADER_READ_WRITE = 0x00000080ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags EARLY_FRAGMENT_TESTS_READ_WRITE = 0x00000100ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags LATE_FRAGMENT_TESTS_READ_WRITE = 0x00000200ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags COLOR_ATTACHMENT_OUTPUT_READ_WRITE = 0x00000400ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags COMPUTE_SHADER_READ_WRITE = 0x00000800ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags ALL_TRANSFER_READ_WRITE = 0x00001000ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags TRANSFER_READ_WRITE = 0x00001000ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags BOTTOM_OF_PIPE_READ_WRITE = 0x00002000ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags HOST_READ_WRITE = 0x00004000ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags ALL_GRAPHICS_READ_WRITE = 0x00008000ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags ALL_COMMANDS_READ_WRITE = 0x00010000ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags COPY_READ_WRITE = 0x100000000ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags RESOLVE_READ_WRITE = 0x200000000ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags BLIT_READ_WRITE = 0x400000000ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags CLEAR_READ_WRITE = 0x800000000ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags INDEX_INPUT_READ_WRITE = 0x1000000000ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags VERTEX_ATTRIBUTE_INPUT_READ_WRITE = 0x2000000000ull | WRITE_ACCESS | READ_ACCESS;
        static inline constexpr PipelineStageAccessFlags PRE_RASTERIZATION_SHADERS_READ_WRITE = 0x4000000000ull | WRITE_ACCESS | READ_ACCESS;
    };
} // namespace daxa
