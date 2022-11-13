#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include <functional>
#include <chrono>
#include <variant>
#include <cstdint>
#include <cstddef>
#include <array>
#include <concepts>

namespace daxa
{
    inline namespace types
    {
        using u8 = std::uint8_t;
        using u16 = std::uint16_t;
        using u32 = std::uint32_t;
        using u64 = std::uint64_t;
        using usize = std::size_t;
        using b32 = u32;

        using i8 = std::int8_t;
        using i16 = std::int16_t;
        using i32 = std::int32_t;
        using i64 = std::int64_t;
        using isize = std::ptrdiff_t;

        using f32 = float;
        using f64 = double;

        using BufferDeviceAddress = u64;

        namespace detail
        {
            template <typename T, usize N>
            struct GenericVecMembers
            {
                std::array<T, N> array;
                constexpr T & operator[](usize i) noexcept { return array[i]; }
                constexpr T const & operator[](usize i) const noexcept { return array[i]; }
            };

            template <typename T>
            struct GenericVecMembers<T, 2>
            {
                T x, y;
                constexpr T & operator[](usize i) noexcept
                {
                    switch (i)
                    {
                    case 1: return y;
                    default: return x;
                    }
                }
                constexpr T const & operator[](usize i) const noexcept
                {
                    switch (i)
                    {
                    case 1: return y;
                    default: return x;
                    }
                }
            };
            template <typename T>
            struct GenericVecMembers<T, 3>
            {
                T x, y, z;
                constexpr T & operator[](usize i) noexcept
                {
                    switch (i)
                    {
                    case 1: return y;
                    case 2: return z;
                    default: return x;
                    }
                }
                constexpr T const & operator[](usize i) const noexcept
                {
                    switch (i)
                    {
                    case 1: return y;
                    case 2: return z;
                    default: return x;
                    }
                }
            };
            template <typename T>
            struct GenericVecMembers<T, 4>
            {
                T x, y, z, w;
                constexpr T & operator[](usize i) noexcept
                {
                    switch (i)
                    {
                    case 1: return y;
                    case 2: return z;
                    case 3: return w;
                    default: return x;
                    }
                }
                constexpr T const & operator[](usize i) const noexcept
                {
                    switch (i)
                    {
                    case 1: return y;
                    case 2: return z;
                    case 3: return w;
                    default: return x;
                    }
                }
            };

            template <typename T, usize N>
            struct GenericVector : GenericVecMembers<T, N>
            {
            };

            template <typename T, usize M, usize N>
            struct GenericMatrix : GenericVector<GenericVector<T, N>, M>
            {
            };
        } // namespace detail

        using b32vec2 = detail::GenericVector<b32, 2>;
        using b32vec3 = detail::GenericVector<b32, 3>;
        using b32vec4 = detail::GenericVector<b32, 4>;
        using f32vec2 = detail::GenericVector<f32, 2>;
        using f32mat2x2 = detail::GenericMatrix<f32, 2, 2>;
        using f32mat2x3 = detail::GenericMatrix<f32, 2, 3>;
        using f32mat2x4 = detail::GenericMatrix<f32, 2, 4>;
        using f64vec2 = detail::GenericVector<f64, 2>;
        using f64mat2x2 = detail::GenericMatrix<f64, 2, 2>;
        using f64mat2x3 = detail::GenericMatrix<f64, 2, 3>;
        using f64mat2x4 = detail::GenericMatrix<f64, 2, 4>;
        using f32vec3 = detail::GenericVector<f32, 3>;
        using f32mat3x2 = detail::GenericMatrix<f32, 3, 2>;
        using f32mat3x3 = detail::GenericMatrix<f32, 3, 3>;
        using f32mat3x4 = detail::GenericMatrix<f32, 3, 4>;
        using f64vec3 = detail::GenericVector<f64, 3>;
        using f64mat3x2 = detail::GenericMatrix<f64, 3, 2>;
        using f64mat3x3 = detail::GenericMatrix<f64, 3, 3>;
        using f64mat3x4 = detail::GenericMatrix<f64, 3, 4>;
        using f32vec4 = detail::GenericVector<f32, 4>;
        using f32mat4x2 = detail::GenericMatrix<f32, 4, 2>;
        using f32mat4x3 = detail::GenericMatrix<f32, 4, 3>;
        using f32mat4x4 = detail::GenericMatrix<f32, 4, 4>;
        using f64vec4 = detail::GenericVector<f64, 4>;
        using f64mat4x2 = detail::GenericMatrix<f64, 4, 2>;
        using f64mat4x3 = detail::GenericMatrix<f64, 4, 3>;
        using f64mat4x4 = detail::GenericMatrix<f64, 4, 4>;
        using f32vec4 = detail::GenericVector<f32, 4>;
        using f32mat4x2 = detail::GenericMatrix<f32, 4, 2>;
        using f32mat4x3 = detail::GenericMatrix<f32, 4, 3>;
        using f32mat4x4 = detail::GenericMatrix<f32, 4, 4>;
        using i32vec2 = detail::GenericVector<i32, 2>;
        using i32mat2x2 = detail::GenericMatrix<i32, 2, 2>;
        using i32mat2x3 = detail::GenericMatrix<i32, 2, 3>;
        using i32mat2x4 = detail::GenericMatrix<i32, 2, 4>;
        using u32vec2 = detail::GenericVector<u32, 2>;
        using u32mat2x2 = detail::GenericMatrix<u32, 2, 2>;
        using u32mat2x3 = detail::GenericMatrix<u32, 2, 3>;
        using u32mat2x4 = detail::GenericMatrix<u32, 2, 4>;
        using i32vec3 = detail::GenericVector<i32, 3>;
        using i32mat3x2 = detail::GenericMatrix<i32, 3, 2>;
        using i32mat3x3 = detail::GenericMatrix<i32, 3, 3>;
        using i32mat3x4 = detail::GenericMatrix<i32, 3, 4>;
        using u32vec3 = detail::GenericVector<u32, 3>;
        using u32mat3x2 = detail::GenericMatrix<u32, 3, 2>;
        using u32mat3x3 = detail::GenericMatrix<u32, 3, 3>;
        using u32mat3x4 = detail::GenericMatrix<u32, 3, 4>;
        using i32vec4 = detail::GenericVector<i32, 4>;
        using i32mat4x2 = detail::GenericMatrix<i32, 4, 2>;
        using i32mat4x3 = detail::GenericMatrix<i32, 4, 3>;
        using i32mat4x4 = detail::GenericMatrix<i32, 4, 4>;
        using u32vec4 = detail::GenericVector<u32, 4>;
        using u32mat4x2 = detail::GenericMatrix<u32, 4, 2>;
        using u32mat4x3 = detail::GenericMatrix<u32, 4, 3>;
        using u32mat4x4 = detail::GenericMatrix<u32, 4, 4>;
    } // namespace types

    enum struct Format
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
    };

    template <typename Properties>
    struct Flags final
    {
        typename Properties::Data data;
        inline constexpr auto operator|=(Flags const & other) -> Flags &
        {
            data |= other.data;
            return *this;
        }
        inline constexpr auto operator&=(Flags const & other) -> Flags &
        {
            data &= other.data;
            return *this;
        }
        inline constexpr auto operator^=(Flags const & other) -> Flags &
        {
            data ^= other.data;
            return *this;
        }
        inline constexpr auto operator~() const -> Flags
        {
            return {~data};
        }
        inline constexpr auto operator|(Flags const & other) const -> Flags
        {
            return {data | other.data};
        }
        inline constexpr auto operator&(Flags const & other) const -> Flags
        {
            return {data & other.data};
        }
        inline constexpr auto operator^(Flags const & other) const -> Flags
        {
            return {data ^ other.data};
        }
        inline constexpr auto operator<=>(Flags const & other) const = default;
    };

    enum struct MsgSeverity
    {
        VERBOSE = 0x00000001,
        INFO = 0x00000010,
        WARNING = 0x00000100,
        FAILURE = 0x00001000,
    };

    enum struct MsgType
    {
        GENERAL = 0x00000001,
        VALIDATION = 0x00000002,
        PERFORMANCE = 0x00000004,
    };

    enum struct PresentMode
    {
        DO_NOT_WAIT_FOR_VBLANK = 0,
        TRIPLE_BUFFER_WAIT_FOR_VBLANK = 1,
        DOUBLE_BUFFER_WAIT_FOR_VBLANK = 2,
        DOUBLE_BUFFER_WAIT_FOR_VBLANK_RELAXED = 3,
    };

    enum struct PresentOp
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

    struct ImageUsageFlagsProperties
    {
        using Data = u32;
    };
    using ImageUsageFlags = Flags<ImageUsageFlagsProperties>;
    struct ImageUsageFlagBits
    {
        static inline constexpr ImageUsageFlags NONE = {0x00000000};
        static inline constexpr ImageUsageFlags TRANSFER_SRC = {0x00000001};
        static inline constexpr ImageUsageFlags TRANSFER_DST = {0x00000002};
        static inline constexpr ImageUsageFlags SHADER_READ_ONLY = {0x00000004};
        static inline constexpr ImageUsageFlags SHADER_READ_WRITE = {0x00000008};
        static inline constexpr ImageUsageFlags COLOR_ATTACHMENT = {0x00000010};
        static inline constexpr ImageUsageFlags DEPTH_STENCIL_ATTACHMENT = {0x00000020};
        static inline constexpr ImageUsageFlags TRANSIENT_ATTACHMENT = {0x00000040};
        static inline constexpr ImageUsageFlags FRAGMENT_DENSITY_MAP = {0x00000200};
        static inline constexpr ImageUsageFlags FRAGMENT_SHADING_RATE_ATTACHMENT = {0x00000100};
        static inline constexpr ImageUsageFlags SHADING_RATE_IMAGE = FRAGMENT_SHADING_RATE_ATTACHMENT;
    };

    struct MemoryFlagsProperties
    {
        using Data = u32;
    };
    using MemoryFlags = Flags<MemoryFlagsProperties>;
    struct MemoryFlagBits
    {
        static inline constexpr MemoryFlags NONE = {0x00000000};
        static inline constexpr MemoryFlags DEDICATED_MEMORY = {0x00000001};
        static inline constexpr MemoryFlags CAN_ALIAS = {0x00000200};
        static inline constexpr MemoryFlags HOST_ACCESS_SEQUENTIAL_WRITE = {0x00000400};
        static inline constexpr MemoryFlags HOST_ACCESS_RANDOM = {0x00000800};
        static inline constexpr MemoryFlags STRATEGY_MIN_MEMORY = {0x00010000};
        static inline constexpr MemoryFlags STRATEGY_MIN_TIME = {0x00020000};
    };

    enum struct ColorSpace
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

    enum struct ImageViewType
    {
        REGULAR_1D = 0,
        REGULAR_2D = 1,
        REGULAR_3D = 2,
        CUBE = 3,
        REGULAR_1D_ARRAY = 4,
        REGULAR_2D_ARRAY = 5,
        CUBE_ARRAY = 6,
    };

    struct ImageAspectFlagsProperties
    {
        using Data = u32;
    };
    using ImageAspectFlags = Flags<ImageAspectFlagsProperties>;
    struct ImageAspectFlagBits
    {
        static inline constexpr ImageAspectFlags NONE = {};
        static inline constexpr ImageAspectFlags COLOR = {0x00000001};
        static inline constexpr ImageAspectFlags DEPTH = {0x00000002};
        static inline constexpr ImageAspectFlags STENCIL = {0x00000004};
        static inline constexpr ImageAspectFlags METADATA = {0x00000008};
        static inline constexpr ImageAspectFlags PLANE_0 = {0x00000010};
        static inline constexpr ImageAspectFlags PLANE_1 = {0x00000020};
        static inline constexpr ImageAspectFlags PLANE_2 = {0x00000040};
    };

    auto to_string(ImageAspectFlags aspect_flags) -> std::string;

    enum struct ImageLayout
    {
        UNDEFINED = 0,
        GENERAL = 1,
        TRANSFER_SRC_OPTIMAL = 6,
        TRANSFER_DST_OPTIMAL = 7,
        READ_ONLY_OPTIMAL = 1000314000,
        ATTACHMENT_OPTIMAL = 1000314001,
        PRESENT_SRC = 1000001002,
    };

    auto to_string(ImageLayout layout) -> std::string_view;

    struct ImageMipArraySlice
    {
        ImageAspectFlags image_aspect = ImageAspectFlagBits::COLOR;
        u32 base_mip_level = 0;
        u32 level_count = 1;
        u32 base_array_layer = 0;
        u32 layer_count = 1;

        friend auto operator<=>(ImageMipArraySlice const &, ImageMipArraySlice const &) = default;

        auto contains(ImageMipArraySlice const & slice) const -> bool;
        auto intersects(ImageMipArraySlice const & slice) const -> bool;
        auto intersect(ImageMipArraySlice const & slice) const -> ImageMipArraySlice;
        auto subtract(ImageMipArraySlice const & slice) const -> std::tuple<std::array<ImageMipArraySlice, 4>, usize>;
    };

    auto to_string(ImageMipArraySlice image_mip_array_slice) -> std::string;

    struct ImageArraySlice
    {
        ImageAspectFlags image_aspect = ImageAspectFlagBits::COLOR;
        u32 mip_level = 0;
        u32 base_array_layer = 0;
        u32 layer_count = 1;

        friend auto operator<=>(ImageArraySlice const &, ImageArraySlice const &) = default;

        static auto slice(ImageMipArraySlice const & mip_array_slice, u32 mip_level = 0) -> ImageArraySlice;

        auto contained_in(ImageMipArraySlice const & slice) const -> bool;
    };

    auto to_string(ImageArraySlice image_array_slice) -> std::string;

    struct ImageSlice
    {
        ImageAspectFlags image_aspect = ImageAspectFlagBits::COLOR;
        u32 mip_level = 0;
        u32 array_layer = 0;

        friend auto operator<=>(ImageSlice const &, ImageSlice const &) = default;

        static auto slice(ImageArraySlice const & mip_array_slice, u32 array_layer = 0) -> ImageSlice;

        auto contained_in(ImageMipArraySlice const & slice) const -> bool;
        auto contained_in(ImageArraySlice const & slice) const -> bool;
    };

    auto to_string(ImageSlice image_slice) -> std::string;

    enum struct Filter
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

        friend auto operator<=>(Offset3D const &, Offset3D const &) = default;
    };

    struct Extent2D
    {
        u32 x = {};
        u32 y = {};

        friend auto operator<=>(Extent2D const &, Extent2D const &) = default;
    };

    struct Extent3D
    {
        u32 x = {};
        u32 y = {};
        u32 z = {};

        friend auto operator<=>(Extent3D const &, Extent3D const &) = default;
    };

    struct DepthValue
    {
        f32 depth;
        u32 stencil;

        friend auto operator<=>(DepthValue const &, DepthValue const &) = default;
    };

    using ClearValue = std::variant<std::array<f32, 4>, std::array<i32, 4>, std::array<u32, 4>, DepthValue>;

    struct AccessTypeFlagsProperties
    {
        using Data = u32;
    };
    using AccessTypeFlags = Flags<AccessTypeFlagsProperties>;
    struct AccessTypeFlagBits
    {
        static inline constexpr AccessTypeFlags NONE = {0x00000000};
        static inline constexpr AccessTypeFlags READ = {0x00008000};
        static inline constexpr AccessTypeFlags WRITE = {0x00010000};
        static inline constexpr AccessTypeFlags READ_WRITE = READ | WRITE;
    };

    auto to_string(AccessTypeFlags flags) -> std::string;

    struct PipelineStageFlagsProperties
    {
        using Data = u64;
    };
    using PipelineStageFlags = Flags<PipelineStageFlagsProperties>;
    struct PipelineStageFlagBits
    {
        static inline constexpr PipelineStageFlags NONE = {0x00000000ull};

        static inline constexpr PipelineStageFlags TOP_OF_PIPE = {0x00000001ull};
        static inline constexpr PipelineStageFlags DRAW_INDIRECT = {0x00000002ull};
        static inline constexpr PipelineStageFlags VERTEX_SHADER = {0x00000008ull};
        static inline constexpr PipelineStageFlags TESSELLATION_CONTROL_SHADER = {0x00000010ull};
        static inline constexpr PipelineStageFlags TESSELLATION_EVALUATION_SHADER = {0x00000020ull};
        static inline constexpr PipelineStageFlags GEOMETRY_SHADER = {0x00000040ull};
        static inline constexpr PipelineStageFlags FRAGMENT_SHADER = {0x00000080ull};
        static inline constexpr PipelineStageFlags EARLY_FRAGMENT_TESTS = {0x00000100ull};
        static inline constexpr PipelineStageFlags LATE_FRAGMENT_TESTS = {0x00000200ull};
        static inline constexpr PipelineStageFlags COLOR_ATTACHMENT_OUTPUT = {0x00000400ull};
        static inline constexpr PipelineStageFlags COMPUTE_SHADER = {0x00000800ull};
        static inline constexpr PipelineStageFlags TRANSFER = {0x00001000ull};
        static inline constexpr PipelineStageFlags BOTTOM_OF_PIPE = {0x00002000ull};
        static inline constexpr PipelineStageFlags HOST = {0x00004000ull};
        static inline constexpr PipelineStageFlags ALL_GRAPHICS = {0x00008000ull};
        static inline constexpr PipelineStageFlags ALL_COMMANDS = {0x00010000ull};
        static inline constexpr PipelineStageFlags COPY = {0x100000000ull};
        static inline constexpr PipelineStageFlags RESOLVE = {0x200000000ull};
        static inline constexpr PipelineStageFlags BLIT = {0x400000000ull};
        static inline constexpr PipelineStageFlags CLEAR = {0x800000000ull};
        static inline constexpr PipelineStageFlags INDEX_INPUT = {0x1000000000ull};
        static inline constexpr PipelineStageFlags PRE_RASTERIZATION_SHADERS = {0x4000000000ull};
    };

    auto to_string(PipelineStageFlags flags) -> std::string;

    struct Access
    {
        PipelineStageFlags stages = PipelineStageFlagBits::NONE;
        AccessTypeFlags type = AccessTypeFlagBits::NONE;

        friend auto operator<=>(Access const &, Access const &) = default;
    };

    auto operator|(Access const & a, Access const & b) -> Access;
    auto operator&(Access const & a, Access const & b) -> Access;

    auto to_string(Access access) -> std::string;

    namespace AccessConsts
    {
        static inline constexpr Access NONE = {.stages = PipelineStageFlagBits::NONE, .type = AccessTypeFlagBits::NONE};

        static inline constexpr Access TOP_OF_PIPE_READ = {.stages = PipelineStageFlagBits::TOP_OF_PIPE, .type = AccessTypeFlagBits::READ};
        static inline constexpr Access DRAW_INDIRECT_READ = {.stages = PipelineStageFlagBits::DRAW_INDIRECT, .type = AccessTypeFlagBits::READ};
        static inline constexpr Access VERTEX_SHADER_READ = {.stages = PipelineStageFlagBits::VERTEX_SHADER, .type = AccessTypeFlagBits::READ};
        static inline constexpr Access TESSELLATION_CONTROL_SHADER_READ = {.stages = PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER, .type = AccessTypeFlagBits::READ};
        static inline constexpr Access TESSELLATION_EVALUATION_SHADER_READ = {.stages = PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER, .type = AccessTypeFlagBits::READ};
        static inline constexpr Access GEOMETRY_SHADER_READ = {.stages = PipelineStageFlagBits::GEOMETRY_SHADER, .type = AccessTypeFlagBits::READ};
        static inline constexpr Access FRAGMENT_SHADER_READ = {.stages = PipelineStageFlagBits::FRAGMENT_SHADER, .type = AccessTypeFlagBits::READ};
        static inline constexpr Access EARLY_FRAGMENT_TESTS_READ = {.stages = PipelineStageFlagBits::EARLY_FRAGMENT_TESTS, .type = AccessTypeFlagBits::READ};
        static inline constexpr Access LATE_FRAGMENT_TESTS_READ = {.stages = PipelineStageFlagBits::LATE_FRAGMENT_TESTS, .type = AccessTypeFlagBits::READ};
        static inline constexpr Access COLOR_ATTACHMENT_OUTPUT_READ = {.stages = PipelineStageFlagBits::COLOR_ATTACHMENT_OUTPUT, .type = AccessTypeFlagBits::READ};
        static inline constexpr Access COMPUTE_SHADER_READ = {.stages = PipelineStageFlagBits::COMPUTE_SHADER, .type = AccessTypeFlagBits::READ};
        static inline constexpr Access TRANSFER_READ = {.stages = PipelineStageFlagBits::TRANSFER, .type = AccessTypeFlagBits::READ};
        static inline constexpr Access BOTTOM_OF_PIPE_READ = {.stages = PipelineStageFlagBits::BOTTOM_OF_PIPE, .type = AccessTypeFlagBits::READ};
        static inline constexpr Access HOST_READ = {.stages = PipelineStageFlagBits::HOST, .type = AccessTypeFlagBits::READ};
        static inline constexpr Access ALL_GRAPHICS_READ = {.stages = PipelineStageFlagBits::ALL_GRAPHICS, .type = AccessTypeFlagBits::READ};
        static inline constexpr Access READ = {.stages = PipelineStageFlagBits::ALL_COMMANDS, .type = AccessTypeFlagBits::READ};
        static inline constexpr Access COPY_READ = {.stages = PipelineStageFlagBits::COPY, .type = AccessTypeFlagBits::READ};
        static inline constexpr Access RESOLVE_READ = {.stages = PipelineStageFlagBits::RESOLVE, .type = AccessTypeFlagBits::READ};
        static inline constexpr Access BLIT_READ = {.stages = PipelineStageFlagBits::BLIT, .type = AccessTypeFlagBits::READ};
        static inline constexpr Access CLEAR_READ = {.stages = PipelineStageFlagBits::CLEAR, .type = AccessTypeFlagBits::READ};
        static inline constexpr Access INDEX_INPUT_READ = {.stages = PipelineStageFlagBits::INDEX_INPUT, .type = AccessTypeFlagBits::READ};
        static inline constexpr Access PRE_RASTERIZATION_SHADERS_READ = {.stages = PipelineStageFlagBits::PRE_RASTERIZATION_SHADERS, .type = AccessTypeFlagBits::READ};

        static inline constexpr Access TOP_OF_PIPE_WRITE = {.stages = PipelineStageFlagBits::TOP_OF_PIPE, .type = AccessTypeFlagBits::WRITE};
        static inline constexpr Access DRAW_INDIRECT_WRITE = {.stages = PipelineStageFlagBits::DRAW_INDIRECT, .type = AccessTypeFlagBits::WRITE};
        static inline constexpr Access VERTEX_SHADER_WRITE = {.stages = PipelineStageFlagBits::VERTEX_SHADER, .type = AccessTypeFlagBits::WRITE};
        static inline constexpr Access TESSELLATION_CONTROL_SHADER_WRITE = {.stages = PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER, .type = AccessTypeFlagBits::WRITE};
        static inline constexpr Access TESSELLATION_EVALUATION_SHADER_WRITE = {.stages = PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER, .type = AccessTypeFlagBits::WRITE};
        static inline constexpr Access GEOMETRY_SHADER_WRITE = {.stages = PipelineStageFlagBits::GEOMETRY_SHADER, .type = AccessTypeFlagBits::WRITE};
        static inline constexpr Access FRAGMENT_SHADER_WRITE = {.stages = PipelineStageFlagBits::FRAGMENT_SHADER, .type = AccessTypeFlagBits::WRITE};
        static inline constexpr Access EARLY_FRAGMENT_TESTS_WRITE = {.stages = PipelineStageFlagBits::EARLY_FRAGMENT_TESTS, .type = AccessTypeFlagBits::WRITE};
        static inline constexpr Access LATE_FRAGMENT_TESTS_WRITE = {.stages = PipelineStageFlagBits::LATE_FRAGMENT_TESTS, .type = AccessTypeFlagBits::WRITE};
        static inline constexpr Access COLOR_ATTACHMENT_OUTPUT_WRITE = {.stages = PipelineStageFlagBits::COLOR_ATTACHMENT_OUTPUT, .type = AccessTypeFlagBits::WRITE};
        static inline constexpr Access COMPUTE_SHADER_WRITE = {.stages = PipelineStageFlagBits::COMPUTE_SHADER, .type = AccessTypeFlagBits::WRITE};
        static inline constexpr Access TRANSFER_WRITE = {.stages = PipelineStageFlagBits::TRANSFER, .type = AccessTypeFlagBits::WRITE};
        static inline constexpr Access BOTTOM_OF_PIPE_WRITE = {.stages = PipelineStageFlagBits::BOTTOM_OF_PIPE, .type = AccessTypeFlagBits::WRITE};
        static inline constexpr Access HOST_WRITE = {.stages = PipelineStageFlagBits::HOST, .type = AccessTypeFlagBits::WRITE};
        static inline constexpr Access ALL_GRAPHICS_WRITE = {.stages = PipelineStageFlagBits::ALL_GRAPHICS, .type = AccessTypeFlagBits::WRITE};
        static inline constexpr Access WRITE = {.stages = PipelineStageFlagBits::ALL_COMMANDS, .type = AccessTypeFlagBits::WRITE};
        static inline constexpr Access COPY_WRITE = {.stages = PipelineStageFlagBits::COPY, .type = AccessTypeFlagBits::WRITE};
        static inline constexpr Access RESOLVE_WRITE = {.stages = PipelineStageFlagBits::RESOLVE, .type = AccessTypeFlagBits::WRITE};
        static inline constexpr Access BLIT_WRITE = {.stages = PipelineStageFlagBits::BLIT, .type = AccessTypeFlagBits::WRITE};
        static inline constexpr Access CLEAR_WRITE = {.stages = PipelineStageFlagBits::CLEAR, .type = AccessTypeFlagBits::WRITE};
        static inline constexpr Access INDEX_INPUT_WRITE = {.stages = PipelineStageFlagBits::INDEX_INPUT, .type = AccessTypeFlagBits::WRITE};
        static inline constexpr Access PRE_RASTERIZATION_SHADERS_WRITE = {.stages = PipelineStageFlagBits::PRE_RASTERIZATION_SHADERS, .type = AccessTypeFlagBits::WRITE};

        static inline constexpr Access TOP_OF_PIPE_READ_WRITE = {.stages = PipelineStageFlagBits::TOP_OF_PIPE, .type = AccessTypeFlagBits::READ_WRITE};
        static inline constexpr Access DRAW_INDIRECT_READ_WRITE = {.stages = PipelineStageFlagBits::DRAW_INDIRECT, .type = AccessTypeFlagBits::READ_WRITE};
        static inline constexpr Access VERTEX_SHADER_READ_WRITE = {.stages = PipelineStageFlagBits::VERTEX_SHADER, .type = AccessTypeFlagBits::READ_WRITE};
        static inline constexpr Access TESSELLATION_CONTROL_SHADER_READ_WRITE = {.stages = PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER, .type = AccessTypeFlagBits::READ_WRITE};
        static inline constexpr Access TESSELLATION_EVALUATION_SHADER_READ_WRITE = {.stages = PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER, .type = AccessTypeFlagBits::READ_WRITE};
        static inline constexpr Access GEOMETRY_SHADER_READ_WRITE = {.stages = PipelineStageFlagBits::GEOMETRY_SHADER, .type = AccessTypeFlagBits::READ_WRITE};
        static inline constexpr Access FRAGMENT_SHADER_READ_WRITE = {.stages = PipelineStageFlagBits::FRAGMENT_SHADER, .type = AccessTypeFlagBits::READ_WRITE};
        static inline constexpr Access EARLY_FRAGMENT_TESTS_READ_WRITE = {.stages = PipelineStageFlagBits::EARLY_FRAGMENT_TESTS, .type = AccessTypeFlagBits::READ_WRITE};
        static inline constexpr Access LATE_FRAGMENT_TESTS_READ_WRITE = {.stages = PipelineStageFlagBits::LATE_FRAGMENT_TESTS, .type = AccessTypeFlagBits::READ_WRITE};
        static inline constexpr Access COLOR_ATTACHMENT_OUTPUT_READ_WRITE = {.stages = PipelineStageFlagBits::COLOR_ATTACHMENT_OUTPUT, .type = AccessTypeFlagBits::READ_WRITE};
        static inline constexpr Access COMPUTE_SHADER_READ_WRITE = {.stages = PipelineStageFlagBits::COMPUTE_SHADER, .type = AccessTypeFlagBits::READ_WRITE};
        static inline constexpr Access TRANSFER_READ_WRITE = {.stages = PipelineStageFlagBits::TRANSFER, .type = AccessTypeFlagBits::READ_WRITE};
        static inline constexpr Access BOTTOM_OF_PIPE_READ_WRITE = {.stages = PipelineStageFlagBits::BOTTOM_OF_PIPE, .type = AccessTypeFlagBits::READ_WRITE};
        static inline constexpr Access HOST_READ_WRITE = {.stages = PipelineStageFlagBits::HOST, .type = AccessTypeFlagBits::READ_WRITE};
        static inline constexpr Access ALL_GRAPHICS_READ_WRITE = {.stages = PipelineStageFlagBits::ALL_GRAPHICS, .type = AccessTypeFlagBits::READ_WRITE};
        static inline constexpr Access READ_WRITE = {.stages = PipelineStageFlagBits::ALL_COMMANDS, .type = AccessTypeFlagBits::READ_WRITE};
        static inline constexpr Access COPY_READ_WRITE = {.stages = PipelineStageFlagBits::COPY, .type = AccessTypeFlagBits::READ_WRITE};
        static inline constexpr Access RESOLVE_READ_WRITE = {.stages = PipelineStageFlagBits::RESOLVE, .type = AccessTypeFlagBits::READ_WRITE};
        static inline constexpr Access BLIT_READ_WRITE = {.stages = PipelineStageFlagBits::BLIT, .type = AccessTypeFlagBits::READ_WRITE};
        static inline constexpr Access CLEAR_READ_WRITE = {.stages = PipelineStageFlagBits::CLEAR, .type = AccessTypeFlagBits::READ_WRITE};
        static inline constexpr Access INDEX_INPUT_READ_WRITE = {.stages = PipelineStageFlagBits::INDEX_INPUT, .type = AccessTypeFlagBits::READ_WRITE};
        static inline constexpr Access PRE_RASTERIZATION_SHADERS_READ_WRITE = {.stages = PipelineStageFlagBits::PRE_RASTERIZATION_SHADERS, .type = AccessTypeFlagBits::READ_WRITE};
    } // namespace AccessConsts

    enum struct SamplerAddressMode
    {
        REPEAT = 0,
        MIRRORED_REPEAT = 1,
        CLAMP_TO_EDGE = 2,
        MIRROR_CLAMP_TO_EDGE = 4,
    };

    enum struct CompareOp
    {
        NEVER = 0,
        LESS = 1,
        EQUAL = 2,
        LESS_OR_EQUAL = 3,
        GREATER = 4,
        NOT_EQUAL = 5,
        GREATER_OR_EQUAL = 6,
        ALWAYS = 7,
    };

    enum struct BlendFactor
    {
        ZERO = 0,
        ONE = 1,
        SRC_COLOR = 2,
        ONE_MINUS_SRC_COLOR = 3,
        DST_COLOR = 4,
        ONE_MINUS_DST_COLOR = 5,
        SRC_ALPHA = 6,
        ONE_MINUS_SRC_ALPHA = 7,
        DST_ALPHA = 8,
        ONE_MINUS_DST_ALPHA = 9,
        CONSTANT_COLOR = 10,
        ONE_MINUS_CONSTANT_COLOR = 11,
        CONSTANT_ALPHA = 12,
        ONE_MINUS_CONSTANT_ALPHA = 13,
        SRC_ALPHA_SATURATE = 14,
        SRC1_COLOR = 15,
        ONE_MINUS_SRC1_COLOR = 16,
        SRC1_ALPHA = 17,
        ONE_MINUS_SRC1_ALPHA = 18,
    };

    enum struct BlendOp
    {
        ADD = 0,
        SUBTRACT = 1,
        REVERSE_SUBTRACT = 2,
        MIN = 3,
        MAX = 4,
    };

    struct ColorComponentFlagsProperties
    {
        using Data = u32;
    };
    using ColorComponentFlags = Flags<ColorComponentFlagsProperties>;
    struct ColorComponentFlagBits
    {
        static inline constexpr ColorComponentFlags NONE = {0x00000000};
        static inline constexpr ColorComponentFlags R = {0x00000001};
        static inline constexpr ColorComponentFlags G = {0x00000002};
        static inline constexpr ColorComponentFlags B = {0x00000004};
        static inline constexpr ColorComponentFlags A = {0x00000008};
    };

    struct BlendInfo
    {
        u32 blend_enable = false;
        BlendFactor src_color_blend_factor = BlendFactor::ONE;
        BlendFactor dst_color_blend_factor = BlendFactor::ZERO;
        BlendOp color_blend_op = BlendOp::ADD;
        BlendFactor src_alpha_blend_factor = BlendFactor::ONE;
        BlendFactor dst_alpha_blend_factor = BlendFactor::ZERO;
        BlendOp alpha_blend_op = BlendOp::ADD;
        ColorComponentFlags color_write_mask = ColorComponentFlagBits::R | ColorComponentFlagBits::G | ColorComponentFlagBits::B | ColorComponentFlagBits::A;

        friend auto operator<=>(BlendInfo const &, BlendInfo const &) = default;
    };

    enum struct PolygonMode
    {
        FILL = 0,
        LINE = 1,
        POINT = 2,
    };

    struct FaceCullFlagsProperties
    {
        using Data = u32;
    };
    using FaceCullFlags = Flags<FaceCullFlagsProperties>;
    struct FaceCullFlagBits
    {
        static inline constexpr FaceCullFlags NONE = {0x00000000};
        static inline constexpr FaceCullFlags FRONT_BIT = {0x00000001};
        static inline constexpr FaceCullFlags BACK_BIT = {0x00000002};
        static inline constexpr FaceCullFlags FRONT_AND_BACK = {0x00000003};
    };

    enum struct AttachmentLoadOp
    {
        LOAD = 0,
        CLEAR = 1,
        DONT_CARE = 2,
    };

    enum struct AttachmentStoreOp
    {
        STORE = 0,
        DONT_CARE = 1,
    };

    struct ViewportInfo
    {
        f32 x = {};
        f32 y = {};
        f32 width = {};
        f32 height = {};
        f32 min_depth = {};
        f32 max_depth = {};
    };

    struct Rect2D
    {
        i32 x = {};
        i32 y = {};
        u32 width = {};
        u32 height = {};
    };
} // namespace daxa
