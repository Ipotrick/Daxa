#pragma once

// Disable msvc warning on alignment padding.
#if defined(_MSC_VER)
#pragma warning(disable : 4324)
#endif

#if !DAXA_BUILT_WITH_UTILS_TASK_GRAPH
#error "[build error] You must build Daxa with the DAXA_ENABLE_UTILS_TASK_GRAPH CMake option enabled"
#endif

#include <array>
#include <string_view>
#include <cstring>
#include <type_traits>
#include <span>
#include <algorithm>

#include <daxa/core.hpp>
#include <daxa/device.hpp>
#include <daxa/utils/mem.hpp>

namespace daxa
{
    static inline constexpr usize MAX_TASK_ATTACHMENTS = 48;

    enum struct TaskAttachmentType : u8
    {
        UNDEFINED,
        BUFFER,
        BLAS,
        TLAS,
        IMAGE,
    };

    struct TaskBufferAttachmentIndex
    {
        u32 value;
    };

    struct TaskBlasAttachmentIndex
    {
        u32 value;
    };

    struct TaskTlasAttachmentIndex
    {
        u32 value;
    };

    struct TaskImageAttachmentIndex
    {
        u32 value;
    };

    enum struct TaskAccessType : u8
    {
        // Concurrent bit: 0
        // Sampled bit: 1
        // Read bit: 2
        // Write bit: 3
        NONE = 0,
        CONCURRENT_BIT = (1 << 0),
        SAMPLED_BIT = (1 << 1),
        READ_BIT = (1 << 2),
        WRITE_BIT = (1 << 3),
        READ = READ_BIT | CONCURRENT_BIT,
        SAMPLED = READ_BIT | CONCURRENT_BIT | SAMPLED_BIT,
        WRITE = WRITE_BIT,
        READ_WRITE = READ_BIT | WRITE_BIT,
        WRITE_CONCURRENT = WRITE_BIT | CONCURRENT_BIT,
        READ_WRITE_CONCURRENT = READ_BIT | WRITE_BIT | CONCURRENT_BIT,
    };

    inline auto is_access_concurrent(TaskAccessType type) -> bool
    {
        return (static_cast<u8>(type) & static_cast<u8>(TaskAccessType::CONCURRENT_BIT)) != 0;
    }

    inline auto are_accesses_compatible(TaskAccessType a, TaskAccessType b) -> bool
    {
        u8 const a_sampled_ignored = static_cast<u8>(a) & ~(static_cast<u8>(TaskAccessType::SAMPLED_BIT));
        u8 const b_sampled_ignored = static_cast<u8>(b) & ~(static_cast<u8>(TaskAccessType::SAMPLED_BIT));
        return (a_sampled_ignored == b_sampled_ignored) && is_access_concurrent(a) && is_access_concurrent(b);
    }

    [[nodiscard]] DAXA_EXPORT_CXX auto to_string(TaskAccessType taccess) -> std::string_view;

    auto to_access_type(TaskAccessType taccess) -> AccessTypeFlags;

    enum struct TaskStages : u64
    {
        NONE = (PipelineStageFlagBits::NONE).data,
        INDIRECT_COMMAND_READ = (PipelineStageFlagBits::INDIRECT_COMMAND_READ).data,
        VERTEX_SHADER = (PipelineStageFlagBits::VERTEX_SHADER).data,
        TESSELLATION_CONTROL_SHADER = (PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER).data,
        TESSELLATION_EVALUATION_SHADER = (PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER).data,
        GEOMETRY_SHADER = (PipelineStageFlagBits::GEOMETRY_SHADER).data,
        FRAGMENT_SHADER = (PipelineStageFlagBits::FRAGMENT_SHADER).data,
        TASK_SHADER = (PipelineStageFlagBits::TASK_SHADER).data,
        MESH_SHADER = (PipelineStageFlagBits::MESH_SHADER).data,
        PRE_RASTERIZATION_SHADERS = (PipelineStageFlagBits::PipelineStageFlagBits::VERTEX_SHADER |
                                     PipelineStageFlagBits::PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER |
                                     PipelineStageFlagBits::PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER |
                                     PipelineStageFlagBits::PipelineStageFlagBits::GEOMETRY_SHADER |
                                     PipelineStageFlagBits::PipelineStageFlagBits::TASK_SHADER |
                                     PipelineStageFlagBits::PipelineStageFlagBits::MESH_SHADER)
                                        .data,
        RASTER_SHADER = (PipelineStageFlagBits::PipelineStageFlagBits::ALL_RASTER).data,
        COMPUTE_SHADER = (PipelineStageFlagBits::COMPUTE_SHADER).data,
        RAY_TRACING_SHADER = (PipelineStageFlagBits::RAY_TRACING_SHADER).data,
        SHADER = (PipelineStageFlagBits::PipelineStageFlagBits::VERTEX_SHADER |
                  PipelineStageFlagBits::PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER |
                  PipelineStageFlagBits::PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER |
                  PipelineStageFlagBits::PipelineStageFlagBits::GEOMETRY_SHADER |
                  PipelineStageFlagBits::PipelineStageFlagBits::FRAGMENT_SHADER |
                  PipelineStageFlagBits::PipelineStageFlagBits::TASK_SHADER |
                  PipelineStageFlagBits::PipelineStageFlagBits::MESH_SHADER |
                  PipelineStageFlagBits::PipelineStageFlagBits::COMPUTE_SHADER |
                  PipelineStageFlagBits::PipelineStageFlagBits::RAY_TRACING_SHADER)
                     .data,
        COLOR_ATTACHMENT = (PipelineStageFlagBits::COLOR_ATTACHMENT_OUTPUT).data,
        DEPTH_STENCIL_ATTACHMENT = (daxa::PipelineStageFlagBits::LATE_FRAGMENT_TESTS |
                                    daxa::PipelineStageFlagBits::EARLY_FRAGMENT_TESTS |
                                    daxa::PipelineStageFlagBits::COLOR_ATTACHMENT_OUTPUT)
                                       .data,
        RESOLVE = (PipelineStageFlagBits::RESOLVE).data,
        INDEX_INPUT = (PipelineStageFlagBits::INDEX_INPUT).data,
        TRANSFER = (PipelineStageFlagBits::TRANSFER).data,
        HOST = (PipelineStageFlagBits::HOST).data,
        AS_BUILD = (PipelineStageFlagBits::ACCELERATION_STRUCTURE_BUILD).data,
        ANY_COMMAND = (PipelineStageFlagBits::ALL_COMMANDS).data,
        JOKER = 1ull << 63ull,
    };

    constexpr inline TaskStages operator|(TaskStages a, TaskStages b)
    {
        return static_cast<TaskStages>(static_cast<u64>(a) | static_cast<u64>(b));
    }

    constexpr inline TaskStages operator&(TaskStages a, TaskStages b)
    {
        return static_cast<TaskStages>(static_cast<u64>(a) & static_cast<u64>(b));
    }

    constexpr inline TaskStages operator~(TaskStages a)
    {
        return static_cast<TaskStages>(~static_cast<u64>(a));
    }

    [[nodiscard]] DAXA_EXPORT_CXX auto to_string(TaskStages stage) -> std::string;

    auto to_pipeline_stage_flags(TaskStages stage) -> PipelineStageFlags;

    struct TaskAccess
    {
        TaskStages stage = {};
        TaskAccessType type = {};
        TaskAttachmentType restriction = {};
    };

    constexpr inline TaskAccess operator|(TaskAccess a, TaskAccess b)
    {
        TaskAccess ret = {};
        ret.stage = a.stage | b.stage;
        ret.type = static_cast<TaskAccessType>(static_cast<u64>(a.type) | static_cast<u64>(b.type));
        ret.restriction = a.restriction;
        return ret;
    }

    [[nodiscard]] DAXA_EXPORT_CXX auto to_string(TaskAccess const & access) -> std::string;

    template <TaskStages STAGE, TaskAttachmentType ATTACHMENT_TYPE_RESTRICTION = TaskAttachmentType::UNDEFINED>
    struct TaskAccessConstsPartial
    {
        static constexpr TaskAccess NONE = TaskAccess{};
        static constexpr TaskAccess READ = TaskAccess{STAGE, TaskAccessType::READ, ATTACHMENT_TYPE_RESTRICTION};
        static constexpr TaskAccess WRITE = TaskAccess{STAGE, TaskAccessType::WRITE, ATTACHMENT_TYPE_RESTRICTION};
        static constexpr TaskAccess WRITE_CONCURRENT = TaskAccess{STAGE, TaskAccessType::WRITE_CONCURRENT, ATTACHMENT_TYPE_RESTRICTION};
        static constexpr TaskAccess READ_WRITE = TaskAccess{STAGE, TaskAccessType::READ_WRITE, ATTACHMENT_TYPE_RESTRICTION};
        static constexpr TaskAccess READ_WRITE_CONCURRENT = TaskAccess{STAGE, TaskAccessType::READ_WRITE_CONCURRENT, ATTACHMENT_TYPE_RESTRICTION};
        static constexpr TaskAccess SAMPLED = TaskAccess{STAGE, TaskAccessType::SAMPLED, ATTACHMENT_TYPE_RESTRICTION};
        static constexpr TaskAccess R = READ;
        static constexpr TaskAccess W = WRITE;
        static constexpr TaskAccess WC = WRITE_CONCURRENT;
        static constexpr TaskAccess RW = READ_WRITE_CONCURRENT;
        static constexpr TaskAccess RWC = READ_WRITE_CONCURRENT;
        static constexpr TaskAccess S = SAMPLED;
    };

    namespace TaskAccessConsts
    {
        using VERTEX_SHADER = TaskAccessConstsPartial<TaskStages::VERTEX_SHADER>;
        using VS = VERTEX_SHADER;
        using TESSELLATION_CONTROL_SHADER = TaskAccessConstsPartial<TaskStages::TESSELLATION_CONTROL_SHADER>;
        using TCS = TESSELLATION_CONTROL_SHADER;
        using TESSELLATION_EVALUATION_SHADER = TaskAccessConstsPartial<TaskStages::TESSELLATION_EVALUATION_SHADER>;
        using TES = TESSELLATION_EVALUATION_SHADER;
        using GEOMETRY_SHADER = TaskAccessConstsPartial<TaskStages::GEOMETRY_SHADER>;
        using GS = GEOMETRY_SHADER;
        using FRAGMENT_SHADER = TaskAccessConstsPartial<TaskStages::FRAGMENT_SHADER>;
        using FS = FRAGMENT_SHADER;
        using COMPUTE_SHADER = TaskAccessConstsPartial<TaskStages::COMPUTE_SHADER>;
        using CS = COMPUTE_SHADER;
        using RAY_TRACING_SHADER = TaskAccessConstsPartial<TaskStages::RAY_TRACING_SHADER>;
        using RT = RAY_TRACING_SHADER;
        using TASK_SHADER = TaskAccessConstsPartial<TaskStages::TASK_SHADER>;
        using TS = TASK_SHADER;
        using MESH_SHADER = TaskAccessConstsPartial<TaskStages::MESH_SHADER>;
        using MS = MESH_SHADER;
        using PRE_RASTERIZATION_SHADERS = TaskAccessConstsPartial<TaskStages::PRE_RASTERIZATION_SHADERS>;
        using PRS = PRE_RASTERIZATION_SHADERS;
        using RASTER_SHADER = TaskAccessConstsPartial<TaskStages::RASTER_SHADER>;
        using RS = RASTER_SHADER;
        using SHADER = TaskAccessConstsPartial<TaskStages::SHADER>;
        using S = SHADER;
        using DEPTH_STENCIL_ATTACHMENT = TaskAccessConstsPartial<TaskStages::DEPTH_STENCIL_ATTACHMENT, TaskAttachmentType::IMAGE>;
        using DSA = DEPTH_STENCIL_ATTACHMENT;
        using RESOLVE = TaskAccessConstsPartial<TaskStages::RESOLVE, TaskAttachmentType::IMAGE>;
        using TRANSFER = TaskAccessConstsPartial<TaskStages::TRANSFER>;
        using TF = TRANSFER;
        using HOST = TaskAccessConstsPartial<TaskStages::HOST>;
        using H = HOST;
        using ACCELERATION_STRUCTURE_BUILD = TaskAccessConstsPartial<TaskStages::AS_BUILD>;
        using ASB = ACCELERATION_STRUCTURE_BUILD;
        using ANY_COMMAND = TaskAccessConstsPartial<TaskStages::ANY_COMMAND>;
        using ANY = ANY_COMMAND;
        static constexpr TaskAccess NONE = TaskAccess{TaskStages::NONE, TaskAccessType::NONE};
        static constexpr TaskAccess READ = TaskAccess{TaskStages::JOKER, TaskAccessType::READ};
        static constexpr TaskAccess WRITE = TaskAccess{TaskStages::JOKER, TaskAccessType::WRITE};
        static constexpr TaskAccess WRITE_CONCURRENT = TaskAccess{TaskStages::JOKER, TaskAccessType::WRITE_CONCURRENT};
        static constexpr TaskAccess READ_WRITE = TaskAccess{TaskStages::JOKER, TaskAccessType::READ_WRITE};
        static constexpr TaskAccess READ_WRITE_CONCURRENT = TaskAccess{TaskStages::JOKER, TaskAccessType::READ_WRITE_CONCURRENT};
        static constexpr TaskAccess SAMPLED = TaskAccess{TaskStages::JOKER, TaskAccessType::SAMPLED};

        static constexpr TaskAccess COLOR_ATTACHMENT = TaskAccess{TaskStages::COLOR_ATTACHMENT, TaskAccessType::READ_WRITE, TaskAttachmentType::IMAGE};
        static constexpr TaskAccess CA = COLOR_ATTACHMENT;

        static constexpr TaskAccess DEPTH_ATTACHMENT = TaskAccessConsts::DSA::READ_WRITE;
        static constexpr TaskAccess STENCIL_ATTACHMENT = TaskAccessConsts::DSA::READ_WRITE;
        static constexpr TaskAccess DEPTH_ATTACHMENT_READ = TaskAccessConsts::DSA::READ;
        static constexpr TaskAccess STENCIL_ATTACHMENT_READ = TaskAccessConsts::DSA::READ;

        static constexpr TaskAccess INDIRECT_COMMAND_READ = TaskAccess{TaskStages::INDIRECT_COMMAND_READ, TaskAccessType::READ, TaskAttachmentType::BUFFER};
        static constexpr TaskAccess ICR = INDIRECT_COMMAND_READ;
        static constexpr TaskAccess INDEX_INPUT_READ = TaskAccess{TaskStages::INDEX_INPUT, TaskAccessType::READ, TaskAttachmentType::BUFFER};
        static constexpr TaskAccess IDXR = INDEX_INPUT_READ;
    }; // namespace TaskAccessConsts

    enum struct TaskType : u16
    {
        UNDEFINED,
        GENERAL,
        RASTER,
        COMPUTE,
        RAY_TRACING,
        TRANSFER
    };

    [[nodiscard]] DAXA_EXPORT_CXX auto to_string(TaskType task_type) -> std::string_view;

    DAXA_EXPORT_CXX auto task_type_default_stage(TaskType task_type) -> TaskStages;

    struct DAXA_EXPORT_CXX TaskGPUResourceView
    {
        u32 task_graph_index = {};
        u32 index = {};

        auto is_empty() const -> bool { return index == 0 && task_graph_index == 0; }
        auto is_external() const -> bool { return task_graph_index == std::numeric_limits<u32>::max() && !is_null(); }
        auto is_null() const -> bool { return task_graph_index == std::numeric_limits<u32>::max() && index == std::numeric_limits<u32>::max(); }

        auto operator<=>(TaskGPUResourceView const & other) const = default;
    };
    static_assert(std::is_standard_layout_v<TaskGPUResourceView>);

    [[nodiscard]] DAXA_EXPORT_CXX auto to_string(TaskGPUResourceView const & id) -> std::string;

    struct DAXA_EXPORT_CXX TaskBufferView
    {        
        u32 task_graph_index = {};
        u32 index = {};

        auto is_empty() const -> bool { return index == 0 && task_graph_index == 0; }
        auto is_external() const -> bool { return task_graph_index == std::numeric_limits<u32>::max() && !is_null(); }
        auto is_null() const -> bool { return task_graph_index == std::numeric_limits<u32>::max() && index == std::numeric_limits<u32>::max(); }

        auto operator<=>(TaskGPUResourceView const & other) const = delete;
        auto operator<=>(TaskBufferView const & other) const = default;
    };
    static_assert(std::is_standard_layout_v<TaskBufferView>);
    static_assert(std::is_layout_compatible_v<TaskGPUResourceView, TaskBufferView>);

    struct DAXA_EXPORT_CXX TaskBlasView
    {        
        u32 task_graph_index = {};
        u32 index = {};

        auto is_empty() const -> bool { return index == 0 && task_graph_index == 0; }
        auto is_external() const -> bool { return task_graph_index == std::numeric_limits<u32>::max() && !is_null(); }
        auto is_null() const -> bool { return task_graph_index == std::numeric_limits<u32>::max() && index == std::numeric_limits<u32>::max(); }

        auto operator<=>(TaskGPUResourceView const & other) const = delete;
        auto operator<=>(TaskBlasView const & other) const = default;
    };
    static_assert(std::is_standard_layout_v<TaskBlasView>);
    static_assert(std::is_layout_compatible_v<TaskGPUResourceView, TaskBlasView>);

    struct DAXA_EXPORT_CXX TaskTlasView
    {        
        u32 task_graph_index = {};
        u32 index = {};

        auto is_empty() const -> bool { return index == 0 && task_graph_index == 0; }
        auto is_external() const -> bool { return task_graph_index == std::numeric_limits<u32>::max() && !is_null(); }
        auto is_null() const -> bool { return task_graph_index == std::numeric_limits<u32>::max() && index == std::numeric_limits<u32>::max(); }

        auto operator<=>(TaskGPUResourceView const & other) const = delete;
        auto operator<=>(TaskTlasView const & other) const = default;
    };
    static_assert(std::is_standard_layout_v<TaskTlasView>);
    static_assert(std::is_layout_compatible_v<TaskGPUResourceView, TaskTlasView>);

    struct TaskAttachmentInfo;

    struct DAXA_EXPORT_CXX TaskImageView
    {
        u32 task_graph_index = {};
        u32 index = {};
        ImageMipArraySlice slice = {};

        auto mips(u32 base_mip_level, u32 level_count = 1) const -> TaskImageView
        {
            auto ret = *this;
            ret.slice.base_mip_level = base_mip_level;
            ret.slice.level_count = level_count;
            return ret;
        }
        auto layers(u32 base_array_layer, u32 layer_count = 1) const -> TaskImageView
        {
            auto ret = *this;
            ret.slice.base_array_layer = base_array_layer;
            ret.slice.layer_count = layer_count;
            return ret;
        }
        auto operator<=>(TaskGPUResourceView const & other) const = delete;
        auto operator<=>(TaskImageView const & other) const = default;

        auto is_empty() const -> bool { return index == 0 && task_graph_index == 0; }
        auto is_external() const -> bool { return task_graph_index == std::numeric_limits<u32>::max() && !is_null(); }
        auto is_null() const -> bool { return task_graph_index == std::numeric_limits<u32>::max() && index == std::numeric_limits<u32>::max(); }
    };
    static_assert(std::is_standard_layout_v<TaskImageView>);
    // static_assert(std::is_layout_compatible_v<TaskGPUResourceView, TaskImageView>);

#ifndef __clang__ // MSVC STL does not implement these for clang :/
    // The TaskImageView::operator TaskGPUResourceView const&() const are only valid IF AND ONLY IF:
    // * TaskGPUResourceView and TaskImageView are standard layout
    // * TaskGPUResourceView and TaskImageView share a common initial sequence for all fields in TaskGPUResourceView
    static_assert(std::is_standard_layout_v<TaskGPUResourceView>);
    static_assert(std::is_standard_layout_v<TaskImageView>);
    static_assert(std::is_corresponding_member(&TaskGPUResourceView::index, &TaskImageView::index)); // Tests for common initial sequence up to TaskGPUResourceView::index.
#endif

    static constexpr inline TaskBufferView NullTaskBuffer = []()
    {
        TaskBufferView ret = {};
        ret.task_graph_index = std::numeric_limits<u32>::max();
        ret.index = std::numeric_limits<u32>::max();
        return ret;
    }();

    static constexpr inline TaskBlasView NullTaskBlas = []()
    {
        TaskBlasView ret = {};
        ret.task_graph_index = std::numeric_limits<u32>::max();
        ret.index = std::numeric_limits<u32>::max();
        return ret;
    }();

    static constexpr inline TaskTlasView NullTaskTlas = []()
    {
        TaskTlasView ret = {};
        ret.task_graph_index = std::numeric_limits<u32>::max();
        ret.index = std::numeric_limits<u32>::max();
        return ret;
    }();

    static constexpr inline TaskImageView NullTaskImage = []()
    {
        TaskImageView ret = {};
        ret.task_graph_index = std::numeric_limits<u32>::max();
        ret.index = std::numeric_limits<u32>::max();
        return ret;
    }();

    struct ImageSliceState
    {
        Access latest_access = {};
        ImageLayout latest_layout = {};
        ImageMipArraySlice slice = {};
    };

    template <typename T>
    concept TaskBufferIndexOrView = std::is_same_v<T, TaskBufferAttachmentIndex> || std::is_same_v<T, TaskBufferView>;
    template <typename T>
    concept TaskBlasIndexOrView = std::is_same_v<T, TaskBlasAttachmentIndex> || std::is_same_v<T, TaskBlasView>;
    template <typename T>
    concept TaskTlasIndexOrView = std::is_same_v<T, TaskTlasAttachmentIndex> || std::is_same_v<T, TaskTlasView>;
    template <typename T>
    concept TaskImageIndexOrView = std::is_same_v<T, TaskImageAttachmentIndex> || std::is_same_v<T, TaskImageView>;
    template <typename T>
    concept TaskIndexOrView = TaskBufferIndexOrView<T> || TaskBlasIndexOrView<T> || TaskTlasIndexOrView<T> || TaskImageIndexOrView<T>;
    template <typename T>
    concept TaskBufferBlasOrTlasIndexOrView = TaskBufferIndexOrView<T> || TaskBlasIndexOrView<T> || TaskTlasIndexOrView<T>;

    enum struct TaskBufferShaderAccessType
    {
        NONE,
        ID,
        ADDRESS,
    };

    struct UndefinedAttachmentRuntimeData
    {
    };

    struct TaskBufferAttachmentInfo
    {
        using INDEX_TYPE = TaskBufferAttachmentIndex;
        static constexpr TaskAttachmentType ATTACHMENT_TYPE = TaskAttachmentType::BUFFER;
        char const * name = {};
        TaskAccess task_access = {};
        Access access = {};
        BufferId id = {};

        TaskBufferView view = {};
        TaskBufferView translated_view = {};
        TaskBufferShaderAccessType shader_access_type = {};
    };
    static_assert(std::is_standard_layout_v<TaskBufferAttachmentInfo>);

    struct TaskBlasAttachmentInfo
    {
        using INDEX_TYPE = TaskBlasAttachmentIndex;
        static constexpr TaskAttachmentType ATTACHMENT_TYPE = TaskAttachmentType::BLAS;
        char const * name = {};
        TaskAccess task_access = {};
        Access access = {};
        BlasId id = {};

        TaskBlasView view = {};
        TaskBlasView translated_view = {};
        TaskBufferShaderAccessType shader_access_type = {};
    };
    static_assert(std::is_standard_layout_v<TaskBlasAttachmentInfo>);

    struct TaskTlasAttachmentInfo
    {
        using INDEX_TYPE = TaskTlasAttachmentIndex;
        static constexpr TaskAttachmentType ATTACHMENT_TYPE = TaskAttachmentType::TLAS;
        char const * name = {};
        TaskAccess task_access = {};
        Access access = {};
        TlasId id = {};

        TaskTlasView view = {};
        TaskTlasView translated_view = {};
        TaskBufferShaderAccessType shader_access_type = {};
    };
    static_assert(std::is_standard_layout_v<TaskTlasAttachmentInfo>);

    struct TaskImageAttachmentInfo
    {
        using INDEX_TYPE = TaskImageAttachmentIndex;
        static constexpr TaskAttachmentType ATTACHMENT_TYPE = TaskAttachmentType::IMAGE;
        char const * name = {};
        TaskAccess task_access = {};
        Access access = {};
        ImageId id = {};

        TaskImageView view = {};
        TaskImageView translated_view = {};
        std::span<ImageViewId const> view_ids = {};
        ImageViewType view_type = ImageViewType::MAX_ENUM;
        u8 shader_array_size = {};
        bool shader_as_index = {};
        bool is_mip_array = {};
    };
    static_assert(std::is_standard_layout_v<TaskImageAttachmentInfo>);

    struct TaskCommonAttachmentInfo
    {
        char const * name = {};
        TaskAccess task_access = {};
        Access access = {};

        union {
            BufferId buffer;
            BlasId blas;
            TlasId tlas;
            ImageId image;
        } id = { .buffer = {} };
    };
    static_assert(std::is_standard_layout_v<TaskCommonAttachmentInfo>);

    struct TaskAttachmentInfo
    {
        TaskAttachmentType type = TaskAttachmentType::UNDEFINED;
        union Value
        {
            TaskBufferAttachmentInfo buffer;
            TaskBlasAttachmentInfo blas;
            TaskTlasAttachmentInfo tlas;
            TaskImageAttachmentInfo image;
            TaskCommonAttachmentInfo common;
        } value = {.common = {}};

        constexpr TaskAttachmentInfo() = default;

        constexpr TaskAttachmentInfo(TaskBufferAttachmentInfo const & buffer)
            : type{TaskAttachmentType::BUFFER}, value{.buffer = buffer}
        {
        }

        constexpr TaskAttachmentInfo(TaskBlasAttachmentInfo const & blas)
            : type{TaskAttachmentType::BLAS}, value{.blas = blas}
        {
        }

        constexpr TaskAttachmentInfo(TaskTlasAttachmentInfo const & tlas)
            : type{TaskAttachmentType::TLAS}, value{.tlas = tlas}
        {
        }

        constexpr TaskAttachmentInfo(TaskImageAttachmentInfo const & image)
            : type{TaskAttachmentType::IMAGE}, value{.image = image}
        {
        }

        constexpr auto shader_array_size() const -> u32
        {
            switch (type)
            {
            case TaskAttachmentType::BUFFER: return (value.buffer.shader_access_type != TaskBufferShaderAccessType::NONE) ? 8u : 0u;
            case TaskAttachmentType::BLAS: return 0;
            case TaskAttachmentType::TLAS: return (value.tlas.shader_access_type != TaskBufferShaderAccessType::NONE) ? 8u : 0u;
            case TaskAttachmentType::IMAGE: return value.image.shader_array_size * (value.image.shader_as_index ? 4 : 8);
            default: return 0;
            }
        }

        constexpr auto shader_element_align() const -> u32
        {
            switch (type)
            {
            case TaskAttachmentType::BUFFER: return (value.buffer.shader_access_type != TaskBufferShaderAccessType::NONE) ? 8u : 0u;
            case TaskAttachmentType::BLAS: return 8u;
            case TaskAttachmentType::TLAS: return (value.tlas.shader_access_type != TaskBufferShaderAccessType::NONE) ? 8u : 0u;
            case TaskAttachmentType::IMAGE: return value.image.shader_as_index ? 4u : 8u;
            default: return 0;
            }
        }
    };

    static_assert(std::is_standard_layout_v<TaskAttachmentInfo>);

    using TaskAttachmentInfoVariant = Variant<
        TaskBufferAttachmentInfo,
        TaskBlasAttachmentInfo,
        TaskTlasAttachmentInfo,
        TaskImageAttachmentInfo>;

    struct DAXA_EXPORT_CXX TaskInterface
    {
        Device & device;
        CommandRecorder & recorder;
        std::span<TaskAttachmentInfo const> attachment_infos = {};
        // optional:
        TransferMemoryPool * allocator = {};
        std::span<std::byte const> attachment_shader_blob = {};
        std::string_view task_name = {};
        usize task_index = {};
        Queue queue = {};

        auto get(TaskBufferAttachmentIndex index) const -> TaskBufferAttachmentInfo const &;
        auto get(TaskBufferView view) const -> TaskBufferAttachmentInfo const &;
        auto get(TaskBlasAttachmentIndex index) const -> TaskBlasAttachmentInfo const &;
        auto get(TaskBlasView view) const -> TaskBlasAttachmentInfo const &;
        auto get(TaskTlasAttachmentIndex index) const -> TaskTlasAttachmentInfo const &;
        auto get(TaskTlasView view) const -> TaskTlasAttachmentInfo const &;
        auto get(TaskImageAttachmentIndex index) const -> TaskImageAttachmentInfo const &;
        auto get(TaskImageView view) const -> TaskImageAttachmentInfo const &;
        auto get(usize index) const -> TaskAttachmentInfo const &;

        auto info(TaskIndexOrView auto tresource) const
        {
            return this->device.info(this->get(tresource).id);
        }
        auto image_view_info(TaskImageIndexOrView auto timage, u32 mip_index = 0u) const -> Optional<ImageViewInfo>
        {
            return this->device.image_view_info(this->get(timage).view_ids[mip_index]);
        }
        auto device_address(TaskBufferBlasOrTlasIndexOrView auto tresource) const -> Optional<DeviceAddress>
        {
            return this->device.device_address(this->get(tresource).id);
        }
        auto buffer_device_address(TaskBufferBlasOrTlasIndexOrView auto tresource) const -> Optional<DeviceAddress>
        {
            return this->device.device_address(this->get(tresource).id);
        }
        auto host_address(TaskBufferIndexOrView auto tbuffer) const -> Optional<std::byte *>
        {
            return this->device.buffer_host_address(this->get(tbuffer).id);
        }
        auto buffer_host_address(TaskBufferIndexOrView auto tbuffer) const -> Optional<std::byte *>
        {
            return this->device.buffer_host_address(this->get(tbuffer).id);
        }
        auto id(TaskIndexOrView auto tresource)
        {
            return this->get(tresource).id;
        }
        auto view(TaskImageIndexOrView auto timg, u32 mip_index = 0u)
        {
            auto const v = this->get(timg).view_ids[mip_index];
            DAXA_DBG_ASSERT_TRUE_M(
                !v.is_empty(),
                "Failed to return cached image view for image attachment!\n"
                "A likely cause for this error is that no daxa::ImageViewType was specified for the attachment.\n"
                "To specify an image view type for a task attachment you can either:\n"
                "1. add the view type to the attachment within a task head as the second parameter: DAXA_TG_IMAGE(COLOR_ATTACHMENT, REGULAR_2D, image_name), OR\n"
                "2. add the view type when adding the attachment to the task: task.color_attachment.reads_writes(daxa::ImageViewType::REGULAR_2D, image)");
            return v;
        }
    };

    struct TaskBufferInfo
    {
        daxa::BufferId buffer = {};
        std::string_view name = {};
    };

    struct ImplExternalResource;
    using ImplPersistentTaskBufferBlasTlas = ImplExternalResource;

    struct DAXA_EXPORT_CXX TaskBuffer : ManagedPtr<TaskBuffer, ImplPersistentTaskBufferBlasTlas *>
    {
        TaskBuffer() = default;
        TaskBuffer(TaskBufferInfo const & info);

        operator TaskBufferView() const;

        auto view() const -> TaskBufferView;
        auto info() const -> TaskBufferInfo;

        void set_buffer(BufferId buffer);
        void swap_buffers(TaskBuffer & other);
        auto id() const -> BufferId;

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
    };

    struct TaskBlasInfo
    {
        BlasId blas = {};
        std::string_view name = {};
    };

    struct DAXA_EXPORT_CXX TaskBlas : ManagedPtr<TaskBlas, ImplPersistentTaskBufferBlasTlas *>
    {
        TaskBlas() = default;
        TaskBlas(TaskBlasInfo const & info);

        operator TaskBlasView() const;

        auto view() const -> TaskBlasView;
        auto info() const -> TaskBlasInfo;
        auto id() const -> BlasId;
        void set_blas(BlasId blas);
        void swap_blas(TaskBlas & other);

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
    };

    struct TaskTlasInfo
    {
        TlasId tlas = {};
        std::string_view name = {};
    };

    struct DAXA_EXPORT_CXX TaskTlas : ManagedPtr<TaskTlas, ImplPersistentTaskBufferBlasTlas *>
    {
        TaskTlas() = default;
        TaskTlas(TaskTlasInfo const & info);

        operator TaskTlasView() const;

        auto view() const -> TaskTlasView;
        auto info() const -> TaskTlasInfo;
        auto id() const -> TlasId;
        void set_tlas(TlasId tlas);
        void swap_tlas(TaskTlas & other);

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
    };

    struct TaskImageInfo
    {
        ImageId image = {};
        bool is_general_layout = {};
        bool is_swapchain_image = {};
        std::string_view name = {};
    };

    struct ImplExternalResource;
    using ImplPersistentTaskImage = ImplExternalResource;

    struct DAXA_EXPORT_CXX TaskImage : ManagedPtr<TaskImage, ImplPersistentTaskImage *>
    {
        TaskImage() = default;
        TaskImage(TaskImageInfo const & info);

        operator TaskImageView() const;

        auto view() const -> TaskImageView;
        auto info() const -> TaskImageInfo;
        auto id() const -> ImageId;
        void set_image(ImageId image, bool is_general_layout = false);
        void swap_images(TaskImage & other);

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
    };

    using TaskViewIndexVariant = Variant<
        std::pair<daxa::TaskBufferAttachmentIndex, daxa::TaskBufferView>,
        std::pair<daxa::TaskBlasAttachmentIndex, daxa::TaskBlasView>,
        std::pair<daxa::TaskTlasAttachmentIndex, daxa::TaskTlasView>,
        std::pair<daxa::TaskImageAttachmentIndex, daxa::TaskImageView>>;

    template <typename T>
    concept TaskBufferViewOrTaskBuffer = std::is_same_v<T, TaskBufferView> || std::is_same_v<T, TaskBuffer>;

    template <typename T>
    concept TaskBlasViewOrTaskBlas = std::is_same_v<T, TaskBlasView> || std::is_same_v<T, TaskBlas>;

    template <typename T>
    concept TaskTlasViewOrTaskTlas = std::is_same_v<T, TaskTlasView> || std::is_same_v<T, TaskTlas>;

    template <typename T>
    concept TaskImageViewOrTaskImage = std::is_same_v<T, TaskImageView> || std::is_same_v<T, TaskImage>;

    template <typename T>
    concept TaskResourceViewOrResource =
        TaskBufferViewOrTaskBuffer<T> || TaskBlasViewOrTaskBlas<T> || TaskTlasViewOrTaskTlas<T> || TaskImageViewOrTaskImage<T>;

    template <typename T>
    concept TaskBufferBlasTlasViewOrBufferBlasTlas =
        TaskBufferViewOrTaskBuffer<T> || TaskBlasViewOrTaskBlas<T> || TaskTlasViewOrTaskTlas<T>;

    template <typename T>
    concept TaskResourceViewOrResourceOrImageViewType =
        TaskResourceViewOrResource<T> || std::is_same_v<ImageViewType, T>;

    template <typename T>
    concept TaskResourceViewOrResourceOrImageViewTypeOrStage =
        TaskResourceViewOrResourceOrImageViewType<T> || std::is_same_v<T, TaskStages>;

    template <typename T>
    concept TaskImageViewOrTaskImageOrImageViewType = std::is_same_v<T, TaskImageView> || std::is_same_v<T, TaskImage> || std::is_same_v<ImageViewType, T>;

    template <typename T>
    concept TaskResourceOrViewOrAccess = TaskResourceViewOrResourceOrImageViewType<T> || std::is_same_v<T, TaskStages>;

    template <typename T>
    concept TaskImageOrViewOrAccess = TaskImageViewOrTaskImageOrImageViewType<T> || std::is_same_v<T, TaskStages>;

    inline namespace detail
    {
        struct AsbSizeAlignment
        {
            u32 size = {};
            u32 alignment = {};
        };
        constexpr auto get_asb_size_and_alignment(auto const & attachment_array) -> AsbSizeAlignment
        {
            AsbSizeAlignment size_align = {};
            for (auto const & attachment_decl : attachment_array)
            {
                if (attachment_decl.shader_array_size() == 0 || attachment_decl.shader_element_align() == 0)
                    continue;
                size_align.size = align_up(size_align.size, attachment_decl.shader_element_align());
                size_align.alignment = std::max(size_align.alignment, attachment_decl.shader_element_align());
                size_align.size += attachment_decl.shader_array_size();
            }
            size_align.size = align_up(size_align.size, size_align.alignment);
            return size_align;
        }
    } // namespace detail

    template <usize N>
    struct StringLiteral
    {
        constexpr StringLiteral(char const (&str)[N])
        {
            std::copy_n(str, N - 1, value);
        }
        char value[N - 1];
        usize SIZE = N - 1;
    };

    // Used for simpler concept template constraint in add_task.
    struct IPartialTask
    {
    };

    struct TaskViewUndefined
    {
    };

    using TaskViewVariant = Variant<
        TaskViewUndefined,
        daxa::TaskBufferView,
        daxa::TaskBlasView,
        daxa::TaskTlasView,
        daxa::TaskImageView>;

    /*
    ⠀⠀⢀⣀⣄⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣴⠾⠛⠛⠷⣦⡀⠀⠀⠀⠀⠀⠀
    ⢠⣶⠛⠋⠉⡙⢷⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣾⣿⠐⡡⢂⠢⠈⠻⣦⡀⠀⠀⠀⠀
    ⣾⠃⠠⡀⠥⡐⡙⣧⣰⣤⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢿⡹⡜⢄⠣⢤⣩⣦⣸⣧⠀⠀⠀⠀
    ⣿⡀⢢⠑⠢⣵⡿⠛⠉⠉⠉⣷⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢿⣜⢬⣿⠛⠉⠉⠉⠻⣧⡀⠀⠀
    ⣹⣇⠢⣉⣾⡏⠀⠠⠀⢆⠡⣘⣷⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⡾⠟⠋⢉⠛⢷⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⣿⡆⠱⡈⠔⠠⠄⠈⢷⡄⠀
    ⠀⢿⣦⢡⣿⠀⠌⡐⠩⡄⢊⢵⣇⣠⣀⣀⡀⠀⠀⠀⠀⣼⠟⢁⢀⠂⠆⡌⢢⢿⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣤⡀⠀⠀⠀⠀⣾⣅⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⡏⢷⡣⠜⣈⠆⡡⠂⠌⣷⠀
    ⠀⠀⢹⡞⣧⠈⡆⢡⠃⣼⣾⡟⠛⠉⠉⠉⠛⣷⡄⠀⢸⡏⠐⢨⡄⡍⠒⣬⢡⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣴⡟⠁⠀⠀⠀⠀⠑⢻⣶⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⣴⣾⡖⣶⣦⠀⠀⠀⠀⠀⠀⠀⢸⡏⡜⣷⢱⢨⡆⢱⠈⡆⣿⠀
    ⠀⠀⠀⠽⣇⠎⡰⣩⡼⡟⠁⠄⡀⠠⠀⠀⠀⠈⢿⡄⡿⢄⢃⠖⡰⣉⠖⣡⡿⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣷⡿⠁⣀⣠⣀⣤⣤⣤⣼⣿⣷⣦⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣰⢯⠋⡀⢀⠀⠉⢿⣆⠀⠀⠀⠀⠀⣸⢗⢡⣿⣂⣖⣨⡱⢊⡔⣿⠀
    ⠀⠀⠀⠀⣯⠒⠥⡾⢇⠰⡉⠔⡠⠃⡌⢐⠡⠀⣼⡟⡓⢌⢒⢪⠑⣌⡾⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣴⣿⣿⣿⣻⠭⠿⠛⠒⠓⠚⠛⠛⠿⣿⣿⣿⣿⣳⡶⢦⣄⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣾⡚⠤⣁⠢⠐⣀⠀⣿⡀⠀⠀⠀⢈⡟⣸⢟⠉⠁⠀⠉⠙⢷⣴⠇⠀
    ⠀⠀⠀⠀⢿⣩⢲⣟⢌⡒⡱⢊⠴⢡⢘⣄⣢⣽⠞⡑⢌⠂⢎⠤⢋⡞⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⣶⣿⣿⣿⠿⠋⡀⢀⠠⠀⠄⠠⠂⠄⠄⡠⠀⠄⡈⠉⠛⠛⠛⡙⠺⣭⡗⣦⣄⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢿⣰⠂⡅⢣⠐⡠⢽⡇⠀⠀⢀⡾⡅⣯⢄⠊⠤⢁⠂⠄⠀⠙⣧⡀
    ⠀⠀⠀⠀⠺⣇⢾⢭⢢⠱⣡⠋⣔⣷⠋⡍⠰⢀⠊⠰⢈⠜⡠⢊⣽⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⡶⣿⡿⠛⠛⡉⡁⢄⠂⡔⢠⠂⡅⢊⢡⠘⡐⢌⣠⡑⠢⢐⠡⢊⠔⡡⢂⠅⣂⠙⡳⣎⡟⣶⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠹⣯⠰⢃⡜⢠⢺⡇⠀⢰⡾⠅⠃⢿⣜⠌⡒⢄⢊⡐⡁⢂⠘⣧
    ⠀⠀⠀⠀⠀⢻⣺⡇⢎⡱⢄⡓⣾⠄⢣⠈⠅⡂⠡⠑⡈⢢⠑⢢⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣴⡾⣫⠗⡅⣢⣥⣧⢽⠶⠟⣶⢶⣿⠆⡜⡐⠦⠱⢌⠢⡜⢏⣿⠛⣛⠳⢾⣤⡣⡜⣠⠓⡤⢩⢳⡎⡝⡷⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⡟⡰⢈⠆⡹⣇⣰⠿⡀⢌⠒⠤⠙⢷⣼⡠⢆⡔⢡⠂⠔⣻
    ⠀⠀⠀⠀⠀⠐⢻⣏⠦⣑⢊⠔⣿⠈⢆⡑⠂⡌⢠⠑⡈⠤⡉⢼⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣴⢻⢣⠞⣣⢵⣾⡿⢋⠃⢆⠬⣹⠗⡬⡑⢎⠴⣉⠎⣕⢪⡑⢎⠲⡸⢯⣅⡚⠤⡘⡙⠿⣶⣍⡒⠧⢎⠼⣑⢣⢏⢷⣆⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡿⡐⠥⠚⡄⡙⠓⠤⡑⢌⡘⠤⡉⣼⢌⣷⠢⠜⢢⠉⢆⣿
    ⠀⠀⠀⠀⠀⠀⠐⣯⣚⠤⡋⡜⢫⠩⢄⠢⡑⡠⢃⠰⡁⢆⠱⣈⡧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣾⠏⣎⢣⣾⣿⡋⢍⡰⢌⡚⣌⣾⢋⠳⡰⣉⢎⠲⣡⠚⡤⠣⡜⣌⢣⠱⣩⠙⠷⣧⠵⡨⠜⡨⠻⣿⣇⠮⣑⢎⢣⠞⣬⡙⣯⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣸⢻⡌⡱⢃⡜⣨⡕⢢⠑⣢⠘⢤⣹⢏⡜⣠⢣⠙⢦⡙⢦⠇
    ⠀⠀⠀⠀⠀⠀⠀⠽⣎⠖⡱⢌⠥⢊⠖⠓⠒⠿⣮⡔⡡⢎⠰⢂⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣼⡛⣆⡛⣴⣿⡟⠰⣌⠲⡌⢶⢞⡋⢦⡉⠖⣑⣢⣮⣵⣶⣷⣶⣷⣶⣶⣥⣧⣢⡙⢢⡑⢎⡡⡙⠴⣛⠛⡦⢓⢬⠚⣌⡓⢦⡹⢜⡻⣆⠀⠀⠀⠀⠀⠀⠀⠀⢰⣿⢜⡢⢱⣡⡿⠛⠛⢒⠳⢤⢋⠴⣛⠣⡔⢢⢎⡙⢦⣱⠟⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⢻⣝⡰⣉⠖⣡⠚⣈⠁⠄⠈⢻⣶⡨⢡⢃⡿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡾⢳⠍⣦⠱⣊⠏⡽⣉⢆⠳⢌⠣⢆⡙⣤⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣼⡠⢃⠝⡢⢅⠫⡔⡍⢦⠹⢤⡙⢦⠱⣋⡜⡻⣆⠀⠀⠀⠀⠀⠀⠀⠈⢻⣜⠲⣱⣿⠀⠂⡍⠰⣈⠦⡉⢖⡡⢓⡌⢣⠎⣜⣶⠏⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠠⢻⣖⡡⠞⣄⠓⡄⠣⢐⠀⠀⢻⣿⣥⡾⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡾⣍⢧⢫⠔⡫⠴⣉⠖⡡⢎⡱⢊⡱⣼⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣎⡑⢎⡱⡘⡜⢢⠝⣢⡙⣌⢳⡑⢮⠱⣹⣆⠀⠀⠀⠀⠀⠀⠀⠈⢿⡱⣿⣿⠀⢃⠌⡱⢠⢒⡉⢦⡑⢣⡜⢣⣾⠞⠁⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠻⣼⠱⣌⠓⡬⠑⡌⠠⠁⢸⣿⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣾⢑⠎⡖⣩⢎⡱⢣⠜⡬⡑⢎⠔⣣⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣮⡢⡑⡜⢌⡱⢪⠔⡱⢊⢦⠹⣌⠏⣄⢻⡆⠀⠀⠀⠀⠀⠀⠀⠁⠙⢿⣿⡌⡐⢌⠰⠡⢎⠜⣢⠙⣦⡽⠟⠁⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠻⣦⣝⡰⢩⢌⠱⣈⣾⠏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣸⠇⣎⠹⡬⣑⠎⣔⠣⣍⠒⡭⢌⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⡕⡘⠦⣡⢃⢎⡱⣉⢦⢋⡜⡎⢥⠊⣿⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠹⣿⣔⣈⠒⣍⣢⣽⡴⠟⠉⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠙⠛⠚⠛⠉⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⡿⠐⣌⢓⠲⣉⠞⡤⢓⠬⡑⢆⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣎⢒⡡⠎⢦⠱⡌⠦⡍⠖⣭⠒⡌⢸⣇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠙⠛⠋⠉⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢼⡇⢢⠙⡜⢦⢋⡴⢡⠚⡤⢓⣼⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡌⢣⠔⡌⢦⠱⢢⠕⡲⣉⠖⣁⠚⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣾⠁⢢⠹⣌⠳⣌⠲⣡⢋⠴⣹⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡐⢎⡜⢢⡙⢆⢫⠱⣌⠳⣀⠂⣿⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠠⣿⠐⢂⠳⣌⠳⣌⠳⣄⢋⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡰⢌⠣⡜⣌⠣⡝⢤⠳⢄⠂⢹⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⠀⢣⡙⣔⠣⡜⠲⡌⢦⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣧⢊⠵⡘⢤⡓⢬⢣⡙⠢⠌⢸⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠸⣿⠈⠴⡱⢌⡳⢌⠳⡘⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣎⠲⣉⠦⡙⣆⢣⠚⡅⠊⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⣿⡈⡱⣘⢣⠜⡬⢣⢹⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡰⡡⢎⡱⡌⡖⣍⠒⡡⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⡇⡒⣍⠮⡜⢆⢣⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠟⡋⠍⡠⠄⡠⢀⠂⡍⢙⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣧⡑⢮⠰⡱⢜⡢⠍⢤⣿⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣺⣇⣱⢎⢲⣉⠮⢼⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⢟⡋⠔⣡⠘⠤⡑⢨⠐⡁⢎⠠⢃⠌⡐⡙⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡜⣌⢣⠕⣎⡱⣩⢘⡿⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠩⣷⡐⣏⠦⣃⠞⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡟⡑⠢⠜⡰⢠⢉⠒⡌⢄⠣⡘⠄⠣⢌⠢⡑⢌⠢⡙⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣯⠔⣣⢚⡴⣑⡃⢾⠇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣷⠸⣜⡰⢻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠋⡴⢉⠜⢢⠑⠢⢌⠒⡌⢢⠑⠤⣉⠲⡈⢆⠱⢌⠢⢡⠃⡽⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⢜⢢⠣⢖⡱⢌⡿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢻⣷⢆⡇⣻⣿⣿⣿⣿⣿⣿⣿⣿⡿⣁⠳⢨⠜⡨⢆⣉⠣⣊⠜⣈⠆⣙⡐⢢⠡⣑⢊⠒⡌⢣⢑⡊⡔⣊⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡎⡖⣹⢊⡵⣿⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⣿⢼⡘⣽⣿⣿⣿⣿⣿⣿⣿⢏⠦⣡⢋⠲⢌⡑⠦⢌⡱⡐⠎⡤⠩⢔⠨⡅⢃⠲⢌⡱⢌⡱⢢⠱⡘⢤⣉⠻⣿⣿⣿⣿⣿⣿⣿⣿⡗⣍⠖⣯⣼⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⢺⡱⣚⣿⣿⣿⣿⣿⠟⡕⢎⠲⡡⢎⡱⢊⡜⡘⢆⠲⢡⠓⣌⠓⣌⠓⣌⢃⠳⣈⠲⢌⡒⣡⢣⡙⠦⣌⠣⡍⢿⣿⣿⣿⣿⣿⣿⡹⡰⣋⢿⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢿⡟⡠⢇⠼⣻⠿⣟⢣⠟⡸⢜⢣⠣⡜⡠⢇⡸⢣⠜⢣⠇⡛⣄⢛⡀⢟⡀⠟⡤⢃⠻⡄⢣⢄⢣⡘⢇⡄⢧⠛⣤⢘⡿⣿⣿⣿⢟⡣⢣⠇⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠚⣧⡑⡌⠒⡡⠚⢤⣳⡾⣱⠪⣅⠳⢬⠱⣊⠴⣃⠮⡑⢎⡱⢌⠦⣙⢢⡙⡜⡰⣉⠖⣩⠲⡌⢦⡙⠦⡜⢢⠛⡤⢓⡜⣆⠳⡜⢪⡑⢣⠋⣾⢁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⣷⣄⣁⣠⣽⡟⢧⡱⢆⡳⣌⢓⡎⡱⣌⠳⣌⢲⣉⠖⡱⢊⠖⣡⢒⡱⣌⡱⡘⣜⢢⢓⡜⣢⡙⣜⡘⣣⢝⡸⣛⢾⣌⡳⢈⠥⠘⠠⢡⡿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠙⠛⠋⠉⠛⢧⡝⣎⠵⣌⠧⡜⡱⣌⠳⣌⠶⡌⢞⡡⢏⠼⣡⢎⡱⢢⠵⡱⣌⢎⠦⣱⢡⠞⣤⠛⡴⢪⠵⣩⢞⣼⠿⢶⣤⣥⣤⡿⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⠳⣽⡘⢮⡱⢳⣌⠳⣜⢢⡝⣢⢝⡸⢲⢡⠞⣰⠣⡞⡱⣌⢎⢞⡰⢣⠞⣔⡫⣜⢣⣿⠶⠋⠀⠀⠀⠈⠉⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠓⠯⣧⣎⠳⣬⢓⡬⡱⢎⡵⣋⡬⣛⠴⣋⠶⡱⢎⡞⡬⢳⡍⣞⣦⠷⠛⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢈⡙⠓⠻⠶⠽⢮⣶⣥⣷⣭⣾⣥⣯⡵⠯⠼⠗⠛⠋⣉⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    */

    /// ======================================= BRACE FOR IMPACT =======================================
    /// ============================== HEAVY TEMPLATE METAPROGRAMMING AHEAD ============================
    /// ========================================= DO NOT PANIC =========================================

    template <typename T>
    struct TaskAttachmentViewWrapper
    {
        TaskViewVariant _value;
        TaskAttachmentViewWrapper()
        {
            _value = TaskViewUndefined{};
        }
        TaskAttachmentViewWrapper(T const & v)
            : _value{v}
        {
        }
    };

    // Used to specialize the TaskHeadStruct.
    // Specialized struct will contain an array of all declared attachments,
    // As well as a named field for each declared attachment with the index of that declaration as a value.
    template <daxa::usize ATTACHMENT_COUNT>
    struct TaskHeadStructSpecializeAttachmentDecls
    {
        struct DeclaredAttachments
        {
            std::array<daxa::TaskAttachmentInfo, ATTACHMENT_COUNT> value = {};
            u32 count = {};
        };
        using InternalT = DeclaredAttachments;
        using TaskBufferT = daxa::TaskBufferAttachmentIndex const;
        using TaskBlasT = daxa::TaskBlasAttachmentIndex const;
        using TaskTlasT = daxa::TaskTlasAttachmentIndex const;
        using TaskImageT = daxa::TaskImageAttachmentIndex const;
        constexpr static usize DECL_ATTACHMENTS = true;
        template <typename TaskResourceT>
        static constexpr auto process_attachment_decl(DeclaredAttachments & _internal, auto const & initalizer)
        {
            _internal.value.at(_internal.count) = initalizer;
            return _internal.count++;
        }
    };

    // Used to specialize the TaskHeadStruct.
    // Specialized struct will contain a named field for each declared attachment typed as a view for that attachment.
    template <daxa::usize ATTACHMENT_COUNT>
    struct TaskHeadStructSpecializeAttachmentViews
    {
        using InternalT = u32;
        using TaskBufferT = daxa::TaskAttachmentViewWrapper<TaskBufferView>;
        using TaskBlasT = daxa::TaskAttachmentViewWrapper<TaskBlasView>;
        using TaskTlasT = daxa::TaskAttachmentViewWrapper<TaskTlasView>;
        using TaskImageT = daxa::TaskAttachmentViewWrapper<TaskImageView>;
        constexpr static usize DECL_ATTACHMENTS = false;
        template <typename TaskResourceT>
        static constexpr auto process_attachment_decl(InternalT &, auto const &)
        {
            return TaskResourceT{};
        }

        static auto convert_to_array(auto const & type) -> std::array<TaskViewVariant, ATTACHMENT_COUNT>
        {
            // Binary compatible with TaskHeadStruct for Views type.
            struct Extractor
            {
                u32 dummy = {};
                std::array<TaskViewVariant, ATTACHMENT_COUNT> initializers = {};
            };
            // Compilers collapse here. Usually type traits start to fail so we will just memcpy here until we get c++26 reflection.
            static constexpr u32 SIZEOF_EXTRACTOR = sizeof(Extractor);
            static constexpr u32 SIZEOF_TYPE = sizeof(decltype(type));
            static_assert(SIZEOF_TYPE == SIZEOF_EXTRACTOR, "DAXA_STATIC_ERROR: TaskAttachmentViews Extractor type abi does not match actual views type!");
            Extractor views = {};
            std::memcpy(&views, &type, SIZEOF_EXTRACTOR);
            auto ret = std::array<TaskViewVariant, ATTACHMENT_COUNT>{};
            for (daxa::u32 i = 0; i < ATTACHMENT_COUNT; ++i)
            {
                ret[i] = views.initializers[i];
            }
            return ret;
        }
    };

#define DAXA_DECL_TASK_HEAD_BEGIN_PROTO(HEAD_NAME, HEAD_TYPE)                                    \
    namespace HEAD_NAME                                                                          \
    {                                                                                            \
        static inline constexpr char _NAME[] = #HEAD_NAME;                                       \
        static inline constexpr daxa::TaskType _TYPE = daxa::TaskType::HEAD_TYPE;                \
        template <typename TDecl, daxa::usize ATTACHMENT_COUNT>                                  \
        struct TaskHeadStruct                                                                    \
        {                                                                                        \
            typename TDecl::InternalT _internal = {};                                            \
                                                                                                 \
            auto convert_to_array() const -> std::array<daxa::TaskViewVariant, ATTACHMENT_COUNT> \
                requires(!TDecl::DECL_ATTACHMENTS)                                               \
            {                                                                                    \
                return TDecl::convert_to_array(*this);                                           \
            }                                                                                    \
            constexpr auto span() const                                                          \
                requires(!!TDecl::DECL_ATTACHMENTS)                                              \
            {                                                                                    \
                return std::span{_internal.value};                                               \
            }                                                                                    \
            constexpr auto at(daxa::usize idx) const -> daxa::TaskAttachmentInfo const &             \
                requires(!!TDecl::DECL_ATTACHMENTS)                                              \
            {                                                                                    \
                return _internal.value.at(idx);                                                  \
            }

#define DAXA_DECL_TASK_HEAD_BEGIN(HEAD_NAME) DAXA_DECL_TASK_HEAD_BEGIN_PROTO(HEAD_NAME, GENERAL)
#define DAXA_DECL_RASTER_TASK_HEAD_BEGIN(HEAD_NAME) DAXA_DECL_TASK_HEAD_BEGIN_PROTO(HEAD_NAME, RASTER)
#define DAXA_DECL_COMPUTE_TASK_HEAD_BEGIN(HEAD_NAME) DAXA_DECL_TASK_HEAD_BEGIN_PROTO(HEAD_NAME, COMPUTE)
#define DAXA_DECL_RAY_TRACING_TASK_HEAD_BEGIN(HEAD_NAME) DAXA_DECL_TASK_HEAD_BEGIN_PROTO(HEAD_NAME, RAY_TRACING)
#define DAXA_DECL_TRANSFER_TASK_HEAD_BEGIN(HEAD_NAME) DAXA_DECL_TASK_HEAD_BEGIN_PROTO(HEAD_NAME, TRANSFER)

// Intellisense has trouble processing the real attachment declarations.
#if defined(__INTELLISENSE__)
#define _DAXA_HELPER_TH_BUFFER(NAME, TASK_ACCESS, ...) typename TDecl::TaskBufferT NAME = {};
#define _DAXA_HELPER_TH_BLAS(NAME, TASK_ACCESS) typename TDecl::TaskBlasT const NAME = {};
#define _DAXA_HELPER_TH_TLAS(NAME, TASK_ACCESS, ...) typename TDecl::TaskTlasT const NAME = {};
#define _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, ...) typename TDecl::TaskImageT const NAME = {};
#else
#define _DAXA_HELPER_TH_BUFFER(NAME, TASK_ACCESS, ...)                         \
    typename TDecl::TaskBufferT NAME =                                         \
        {TDecl::template process_attachment_decl<typename TDecl::TaskBufferT>( \
            _internal,                                                         \
            daxa::TaskBufferAttachmentInfo{                                        \
                .name = #NAME,                                                 \
                .task_access = []() { using namespace daxa::TaskAccessConsts; return TASK_ACCESS; }(),                                     \
                __VA_ARGS__})};

#define _DAXA_HELPER_TH_BLAS(NAME, TASK_ACCESS)                              \
    typename TDecl::TaskBlasT const NAME =                                   \
        {TDecl::template process_attachment_decl<typename TDecl::TaskBlasT>( \
            _internal,                                                       \
            daxa::TaskBlasAttachmentInfo{                                        \
                .name = #NAME,                                               \
                .task_access = []() { using namespace daxa::TaskAccessConsts; return TASK_ACCESS; }(),                                   \
            })};

#define _DAXA_HELPER_TH_TLAS(NAME, TASK_ACCESS, ...)                         \
    typename TDecl::TaskTlasT const NAME =                                   \
        {TDecl::template process_attachment_decl<typename TDecl::TaskTlasT>( \
            _internal,                                                       \
            daxa::TaskTlasAttachmentInfo{                                        \
                .name = #NAME,                                               \
                .task_access = []() { using namespace daxa::TaskAccessConsts; return TASK_ACCESS; }(),                                   \
                __VA_ARGS__})};

#define _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, ...)                         \
    typename TDecl::TaskImageT const NAME =                                   \
        {TDecl::template process_attachment_decl<typename TDecl::TaskImageT>( \
            _internal,                                                        \
            daxa::TaskImageAttachmentInfo{                                        \
                .name = #NAME,                                                \
                .task_access = []() { using namespace daxa::TaskAccessConsts; return TASK_ACCESS; }(),                                    \
                __VA_ARGS__})};
#endif

#define DAXA_DECL_TASK_HEAD_END                                                                                                                         \
    }                                                                                                                                                   \
    ;                                                                                                                                                   \
    struct Info                                                                                                                                         \
    {                                                                                                                                                   \
        static inline constexpr daxa::TaskType TYPE = _TYPE;                                                                                            \
        static inline constexpr char const * NAME = _NAME;                                                                                              \
        static inline constexpr auto ATTACHMENT_COUNT =                                                                                                 \
            TaskHeadStruct<daxa::TaskHeadStructSpecializeAttachmentDecls<daxa::MAX_TASK_ATTACHMENTS>, daxa::MAX_TASK_ATTACHMENTS>{}                     \
                ._internal.count;                                                                                                                       \
        using Views = TaskHeadStruct<daxa::TaskHeadStructSpecializeAttachmentViews<ATTACHMENT_COUNT>, ATTACHMENT_COUNT>;                                \
        using AttachmentViews = Views;                                                                                                                  \
        static inline constexpr auto ATTACHMENTS = TaskHeadStruct<daxa::TaskHeadStructSpecializeAttachmentDecls<ATTACHMENT_COUNT>, ATTACHMENT_COUNT>{}; \
        static inline constexpr auto AT = ATTACHMENTS;                                                                                                  \
        struct alignas(daxa::detail::get_asb_size_and_alignment(ATTACHMENTS.span()).alignment) AttachmentShaderBlob                                     \
        {                                                                                                                                               \
            std::array<std::byte, daxa::detail::get_asb_size_and_alignment(ATTACHMENTS.span()).size> value = {};                                        \
            AttachmentShaderBlob() = default;                                                                                                           \
            AttachmentShaderBlob(std::span<std::byte const> data) { *this = data; }                                                                     \
            auto operator=(std::span<std::byte const> data) -> AttachmentShaderBlob &                                                                   \
            {                                                                                                                                           \
                DAXA_DBG_ASSERT_TRUE_M(this->value.size() == data.size(), "Blob size missmatch!");                                                      \
                for (daxa::u32 i = 0; i < data.size(); ++i)                                                                                             \
                    this->value[i] = data[i];                                                                                                           \
                return *this;                                                                                                                           \
            }                                                                                                                                           \
        };                                                                                                                                              \
    };                                                                                                                                                  \
    using Views = Info::Views;                                                                                                                          \
    using AttachmentViews = Info::Views;                                                                                                                \
    using AttachmentShaderBlob = Info::AttachmentShaderBlob;                                                                                            \
    static inline constexpr daxa::TaskType TYPE = Info::TYPE;                                                                                           \
    static inline constexpr char const * NAME = Info::NAME;                                                                                             \
    static constexpr decltype(Info::ATTACHMENT_COUNT) ATTACHMENT_COUNT = Info::ATTACHMENT_COUNT;                                                        \
    static constexpr auto const & ATTACHMENTS = Info::ATTACHMENTS;                                                                                      \
    static constexpr auto const & AT = Info::ATTACHMENTS;                                                                                               \
    struct Task : public daxa::IPartialTask                                                                                                             \
    {                                                                                                                                                   \
        using Info = Info;                                                                                                                              \
        static constexpr auto const & ATTACHMENTS = Info::ATTACHMENTS;                                                                                  \
        static constexpr auto const & AT = Info::ATTACHMENTS;                                                                                           \
        using Views = Info::Views;                                                                                                                      \
        using AttachmentViews = Info::Views;                                                                                                            \
    };                                                                                                                                                  \
    }                                                                                                                                                   \
    ;

#define DAXA_TH_BLOB(HEAD_NAME, field_name) HEAD_NAME::AttachmentShaderBlob field_name;

#define DAXA_TH_IMAGE(TASK_ACCESS, VIEW_TYPE, NAME) _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, .view_type = daxa::ImageViewType::VIEW_TYPE, .shader_array_size = 0)

#define DAXA_TH_IMAGE_ID(TASK_ACCESS, VIEW_TYPE, NAME) _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, .view_type = daxa::ImageViewType::VIEW_TYPE, .shader_array_size = 1)
#define DAXA_TH_IMAGE_ID_MIP_ARRAY(TASK_ACCESS, VIEW_TYPE, NAME, SIZE) _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, .view_type = daxa::ImageViewType::VIEW_TYPE, .shader_array_size = SIZE, .is_mip_array = true)

#define DAXA_TH_IMAGE_INDEX(TASK_ACCESS, VIEW_TYPE, NAME) _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, .view_type = daxa::ImageViewType::VIEW_TYPE, .shader_array_size = 1, .shader_as_index = true)
#define DAXA_TH_IMAGE_INDEX_MIP_ARRAY(TASK_ACCESS, VIEW_TYPE, NAME, SIZE) _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, .view_type = daxa::ImageViewType::VIEW_TYPE, .shader_array_size = SIZE, .is_mip_array = true, .shader_as_index = true)

#define DAXA_TH_IMAGE_TYPED(TASK_ACCESS, VIEW_TYPE, NAME) _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, .view_type = VIEW_TYPE::IMAGE_VIEW_TYPE, .shader_array_size = 1, .shader_as_index = VIEW_TYPE::SHADER_INDEX32)
#define DAXA_TH_IMAGE_TYPED_MIP_ARRAY(TASK_ACCESS, VIEW_TYPE, NAME, SIZE) _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, .view_type = VIEW_TYPE::IMAGE_VIEW_TYPE, .shader_array_size = SIZE, .shader_as_index = VIEW_TYPE::SHADER_INDEX32, .is_mip_array = true)

#define DAXA_TH_STAGE_VAR(STAGE_VAR) daxa::TaskStages stage = {};

#define DAXA_TH_BUFFER(TASK_ACCESS, NAME) _DAXA_HELPER_TH_BUFFER(NAME, TASK_ACCESS)
#define DAXA_TH_BUFFER_ID(TASK_ACCESS, NAME) _DAXA_HELPER_TH_BUFFER(NAME, TASK_ACCESS, .shader_access_type = daxa::TaskBufferShaderAccessType::ID)
#define DAXA_TH_BUFFER_PTR(TASK_ACCESS, PTR_TYPE, NAME) _DAXA_HELPER_TH_BUFFER(NAME, TASK_ACCESS, .shader_access_type = daxa::TaskBufferShaderAccessType::ADDRESS)
#define DAXA_TH_BLAS(TASK_ACCESS, NAME) _DAXA_HELPER_TH_BLAS(NAME, TASK_ACCESS)
#define DAXA_TH_TLAS(TASK_ACCESS, NAME) _DAXA_HELPER_TH_TLAS(NAME, TASK_ACCESS)
#define DAXA_TH_TLAS_ID(TASK_ACCESS, NAME) _DAXA_HELPER_TH_TLAS(NAME, TASK_ACCESS, .shader_access_type = daxa::TaskBufferShaderAccessType::ID)
#define DAXA_TH_TLAS_PTR(TASK_ACCESS, NAME) _DAXA_HELPER_TH_TLAS(NAME, TASK_ACCESS, .shader_access_type = daxa::TaskBufferShaderAccessType::ADDRESS)

    inline auto inl_attachment(TaskAccess access, TaskBufferView view) -> TaskAttachmentInfo
    {
        TaskBufferAttachmentInfo buf = {};
        buf.name = "inline attachment";
        buf.task_access = access;
        buf.shader_access_type = TaskBufferShaderAccessType::NONE;
        buf.view = view;
        TaskAttachmentInfo info = {};
        info.type = daxa::TaskAttachmentType::BUFFER;
        info.value.buffer = buf;
        return info;
    }

    inline auto inl_attachment(TaskAccess access, TaskBlasView view) -> TaskAttachmentInfo
    {
        TaskBlasAttachmentInfo blas = {};
        blas.name = "inline attachment";
        blas.task_access = access;
        blas.view = view;
        blas.shader_access_type = TaskBufferShaderAccessType::NONE;
        TaskAttachmentInfo info = {};
        info.type = daxa::TaskAttachmentType::BLAS;
        info.value.blas = blas;
        return info;
    }

    inline auto inl_attachment(TaskAccess access, TaskTlasView view) -> TaskAttachmentInfo
    {
        TaskTlasAttachmentInfo tlas = {};
        tlas.name = "inline attachment";
        tlas.task_access = access;
        tlas.view = view;
        tlas.shader_access_type = TaskBufferShaderAccessType::NONE;
        TaskAttachmentInfo info = {};
        info.type = daxa::TaskAttachmentType::TLAS;
        info.value.tlas = tlas;
        return info;
    }

    inline auto inl_attachment(TaskAccess access, TaskImageView view, ImageViewType view_type = daxa::ImageViewType::MAX_ENUM) -> TaskAttachmentInfo
    {
        TaskImageAttachmentInfo img = {};
        img.name = "inline attachment";
        img.task_access = access;
        img.view_type = view_type;
        img.shader_array_size = 0;
        img.view = view;
        TaskAttachmentInfo info = {};
        info.value.image = img;
        info.type = daxa::TaskAttachmentType::IMAGE;
        return info;
    }

    inline auto inl_attachment(TaskAccess access, ImageViewType view_type, TaskImageView view) -> TaskAttachmentInfo
    {
        TaskImageAttachmentInfo img = {};
        img.name = "inline attachment";
        img.task_access = access;
        img.view_type = view_type;
        img.shader_array_size = 0;
        img.view = view;
        TaskAttachmentInfo info = {};
        info.type = daxa::TaskAttachmentType::IMAGE;
        info.value.image = img;
        return info;
    }
} // namespace daxa