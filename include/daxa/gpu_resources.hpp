#pragma once

#include <daxa/types.hpp>

namespace daxa
{

    enum struct ImageViewType
    {
        REGULAR_1D = 0,
        REGULAR_2D = 1,
        REGULAR_3D = 2,
        CUBE = 3,
        REGULAR_1D_ARRAY = 4,
        REGULAR_2D_ARRAY = 5,
        CUBE_ARRAY = 6,
        MAX_ENUM = 0x7fffffff,
    };

    DAXA_EXPORT_CXX auto to_string(ImageViewType const & type) -> std::string_view;

    struct DAXA_EXPORT_CXX GPUResourceId
    {
        u64 index : 20 = {};
        u64 version : 44 = {};

        auto is_empty() const -> bool;

        constexpr auto operator<=>(GPUResourceId const & other) const
        {
            return std::bit_cast<u64>(*this) <=> std::bit_cast<u64>(other);
        }
        constexpr bool operator==(GPUResourceId const & other) const = default;
        constexpr bool operator!=(GPUResourceId const & other) const = default;
        constexpr bool operator<(GPUResourceId const & other) const = default;
        constexpr bool operator>(GPUResourceId const & other) const = default;
        constexpr bool operator<=(GPUResourceId const & other) const = default;
        constexpr bool operator>=(GPUResourceId const & other) const = default;
    };

    inline namespace types
    {
        struct BufferId : public GPUResourceId
        {
            constexpr operator daxa_BufferId() const { return std::bit_cast<daxa_BufferId>(*this); }
        };

        struct ImageViewId : public GPUResourceId
        {
            constexpr operator daxa_ImageViewId() const { return std::bit_cast<daxa_ImageViewId>(*this); }
        };

        template <ImageViewType VIEW_TYPE>
        struct TypedImageViewId : public ImageViewId
        {
            static constexpr inline auto view_type() -> ImageViewType { return VIEW_TYPE; }
        };

        struct ImageId : public GPUResourceId
        {
            constexpr operator daxa_ImageId() const { return std::bit_cast<daxa_ImageId>(*this); }
            DAXA_EXPORT_CXX auto default_view() const -> ImageViewId;
        };

        struct SamplerId : public GPUResourceId
        {
            constexpr operator daxa_SamplerId() const { return std::bit_cast<daxa_SamplerId>(*this); }
        };

        struct TlasId : public GPUResourceId
        {
            operator daxa_TlasId() const { return std::bit_cast<daxa_TlasId>(*this); }
        };

        struct BlasId : public GPUResourceId
        {
            operator daxa_BlasId() const { return std::bit_cast<daxa_BlasId>(*this); }
        };
    } // namespace types

    DAXA_EXPORT_CXX auto to_string(GPUResourceId const & id) -> std::string;

    struct BufferInfo
    {
        usize size = {};
        // Ignored when allocating with a memory block.
        MemoryFlags allocate_info = {};
        SmallString name = "";
    };

    struct DAXA_EXPORT_CXX ImageCreateFlagsProperties
    {
        using Data = u32;
    };
    using ImageCreateFlags = Flags<ImageCreateFlagsProperties>;
    struct ImageCreateFlagBits
    {
        static inline constexpr ImageCreateFlags NONE = {0x00000000};
        static inline constexpr ImageCreateFlags ALLOW_MUTABLE_FORMAT = {0x00000008};
        static inline constexpr ImageCreateFlags COMPATIBLE_CUBE = {0x00000010};
        static inline constexpr ImageCreateFlags COMPATIBLE_2D_ARRAY = {0x00000020};
        static inline constexpr ImageCreateFlags ALLOW_ALIAS = {0x00000400};
    };

    struct ImageInfo
    {
        ImageCreateFlags flags = ImageCreateFlagBits::NONE;
        u32 dimensions = 2;
        Format format = Format::R8G8B8A8_UNORM;
        Extent3D size = {0, 0, 0};
        u32 mip_level_count = 1;
        u32 array_layer_count = 1;
        u32 sample_count = 1;
        ImageUsageFlags usage = {};
        // Ignored when allocating with a memory block.
        MemoryFlags allocate_info = {};
        SmallString name = "";
    };

    struct ImageViewInfo
    {
        ImageViewType type = ImageViewType::REGULAR_2D;
        Format format = Format::R8G8B8A8_UNORM;
        ImageId image = {};
        ImageMipArraySlice slice = {};
        SmallString name = "";
    };

    struct SamplerInfo
    {
        Filter magnification_filter = Filter::LINEAR;
        Filter minification_filter = Filter::LINEAR;
        Filter mipmap_filter = Filter::LINEAR;
        ReductionMode reduction_mode = ReductionMode::WEIGHTED_AVERAGE;
        SamplerAddressMode address_mode_u = SamplerAddressMode::CLAMP_TO_EDGE;
        SamplerAddressMode address_mode_v = SamplerAddressMode::CLAMP_TO_EDGE;
        SamplerAddressMode address_mode_w = SamplerAddressMode::CLAMP_TO_EDGE;
        f32 mip_lod_bias = 0.5f;
        bool enable_anisotropy = false;
        f32 max_anisotropy = 0.0f;
        bool enable_compare = false;
        CompareOp compare_op = CompareOp::ALWAYS;
        f32 min_lod = 0.0f;
        f32 max_lod = 1000.0f; // This value is the "VK_LOD_CLAMP_MODE_NONE" value
        BorderColor border_color = BorderColor::FLOAT_TRANSPARENT_BLACK;
        bool enable_unnormalized_coordinates = false;
        SmallString name = "";
    };

    struct GeometryFlagsProperties
    {
        using Data = u32;
    };
    using GeometryFlags = Flags<GeometryFlagsProperties>;
    struct GeometryFlagBits
    {
        static inline constexpr GeometryFlags OPAQUE = {0x1 << 0};
        static inline constexpr GeometryFlags NO_DUPLICATE_ANY_HIT_INVOCATION = {0x1 << 1};
    };

    struct BlasTriangleGeometryInfo
    {
        Format vertex_format = daxa::Format::R32G32B32_SFLOAT;
        DeviceAddress vertex_data = {};
        u64 vertex_stride = 12;
        u32 max_vertex = {};
        IndexType index_type = IndexType::uint32;
        DeviceAddress index_data = {};
        DeviceAddress transform_data = {};
        u32 count = {};
        GeometryFlags flags = GeometryFlagBits::OPAQUE;
    };

    struct BlasAabbGeometryInfo
    {
        DeviceAddress data = {};
        u64 stride = {};
        u32 count = {};
        GeometryFlags flags = GeometryFlagBits::OPAQUE;
    };

    /// Instances are defines as VkAccelerationStructureInstanceKHR;
    struct TlasInstanceInfo
    {
        DeviceAddress data = {};
        u32 count = {};
        daxa_Bool8 is_data_array_of_pointers = {};
        GeometryFlags flags = GeometryFlagBits::OPAQUE;
    };

    struct DAXA_EXPORT_CXX AccelerationStructureBuildFlagsProperties
    {
        using Data = u32;
    };
    using AccelerationStructureBuildFlags = Flags<AccelerationStructureBuildFlagsProperties>;
    struct AccelerationStructureBuildFlagBits
    {
        static inline constexpr AccelerationStructureBuildFlags ALLOW_UPDATE = {0x00000001};
        static inline constexpr AccelerationStructureBuildFlags ALLOW_COMPACTION = {0x00000002};
        static inline constexpr AccelerationStructureBuildFlags PREFER_FAST_TRACE = {0x00000004};
        static inline constexpr AccelerationStructureBuildFlags PREFER_FAST_BUILD = {0x00000008};
        static inline constexpr AccelerationStructureBuildFlags LOW_MEMORY =  {0x00000010};
    };

    struct TlasBuildInfo
    {
        AccelerationStructureBuildFlags flags = daxa::AccelerationStructureBuildFlagBits::PREFER_FAST_TRACE;
        TlasId dst_tlas = {};
        std::span<TlasInstanceInfo const> instances = {};
        DeviceAddress scratch_data = {};
    };

    struct BlasBuildInfo
    {
        AccelerationStructureBuildFlags flags = daxa::AccelerationStructureBuildFlagBits::PREFER_FAST_TRACE;
        BlasId dst_blas = {};
        Variant<
            std::span<BlasTriangleGeometryInfo const>,
            std::span<BlasAabbGeometryInfo const>>
            geometries;
        DeviceAddress scratch_data = {};
    };

    struct TlasInfo
    {
        u64 size = {};
        SmallString name = "";
    };

    struct BlasInfo
    {
        u64 size = {};
        SmallString name = "";
    };
} // namespace daxa
