#pragma once

// Disable msvc warning on alignment padding.
#if defined(_MSC_VER)
#pragma warning(disable : 4324)
#endif

#if !DAXA_BUILT_WITH_UTILS_TASK_GRAPH
#error "[package management error] You must build Daxa with the DAXA_ENABLE_UTILS_TASK_GRAPH CMake option enabled, or request the utils-task-graph feature in vcpkg"
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
        // Read bit: 1
        // Sampled bit: 2
        // Write bit: 3
        NONE = 0,
        CONCURRENT_BIT = 1,
        READ = (1 << 1) | CONCURRENT_BIT,
        SAMPLED = 1 << 2 | CONCURRENT_BIT,
        WRITE = 1 << 3,
        READ_WRITE = (1 << 1) | (1 << 3),
        WRITE_CONCURRENT = WRITE | CONCURRENT_BIT,
        READ_WRITE_CONCURRENT = READ_WRITE | CONCURRENT_BIT,
    };

    auto to_access_type(TaskAccessType taccess) -> AccessTypeFlags;

    enum struct TaskStage : u16
    {
        NONE,
        VERTEX_SHADER,
        TESSELLATION_CONTROL_SHADER,
        TESSELLATION_EVALUATION_SHADER,
        GEOMETRY_SHADER,
        FRAGMENT_SHADER,
        TASK_SHADER,
        MESH_SHADER,
        PRE_RASTERIZATION_SHADERS,
        RASTER_SHADER,
        COMPUTE_SHADER,
        RAY_TRACING_SHADER,
        SHADER,
        COLOR_ATTACHMENT,
        DEPTH_STENCIL_ATTACHMENT,
        RESOLVE,
        PRESENT,
        INDIRECT_COMMAND,
        INDEX_INPUT,
        TRANSFER,
        HOST,
        AS_BUILD,
        ANY_COMMAND,
    };

    auto to_string(TaskStage stage) -> std::string_view;

    auto to_pipeline_stage_flags(TaskStage stage) -> PipelineStageFlags;

    struct alignas(u32) TaskAccess
    {
        TaskStage stage = {};
        TaskAccessType type = {};
        TaskAttachmentType restriction = {};
    };
    static_assert(sizeof(TaskAccess) == sizeof(u32));

    [[nodiscard]] DAXA_EXPORT_CXX auto to_string(TaskAccess const & access) -> std::string_view;

    template <TaskStage STAGE, TaskAttachmentType ATTACHMENT_TYPE_RESTRICTION = TaskAttachmentType::UNDEFINED>
    struct TaskAccessConstsPartial
    {
        static constexpr TaskAccess NONE = TaskAccess{};
        static constexpr TaskAccess READ = TaskAccess{STAGE, TaskAccessType::READ, ATTACHMENT_TYPE_RESTRICTION};
        static constexpr TaskAccess WRITE = TaskAccess{STAGE, TaskAccessType::WRITE, ATTACHMENT_TYPE_RESTRICTION};
        static constexpr TaskAccess WRITE_CONCURRENT = TaskAccess{STAGE, TaskAccessType::WRITE_CONCURRENT, ATTACHMENT_TYPE_RESTRICTION};
        static constexpr TaskAccess READ_WRITE = TaskAccess{STAGE, TaskAccessType::READ_WRITE, ATTACHMENT_TYPE_RESTRICTION};
        static constexpr TaskAccess READ_WRITE_CONCURRENT = TaskAccess{STAGE, TaskAccessType::READ_WRITE_CONCURRENT, ATTACHMENT_TYPE_RESTRICTION};
        static constexpr TaskAccess SAMPLED = TaskAccess{STAGE, TaskAccessType::SAMPLED, TaskAttachmentType::IMAGE};
        static constexpr TaskAccess R = READ;
        static constexpr TaskAccess W = WRITE;
        static constexpr TaskAccess WC = WRITE_CONCURRENT;
        static constexpr TaskAccess RW = READ_WRITE_CONCURRENT;
        static constexpr TaskAccess RWC = READ_WRITE_CONCURRENT;
        static constexpr TaskAccess S = SAMPLED;
    };

    struct TaskAccessConsts
    {
        using VERTEX_SHADER = TaskAccessConstsPartial<TaskStage::VERTEX_SHADER>;
        using VS = VERTEX_SHADER;
        using TESSELLATION_CONTROL_SHADER = TaskAccessConstsPartial<TaskStage::TESSELLATION_CONTROL_SHADER>;
        using TCS = TESSELLATION_CONTROL_SHADER;
        using TESSELLATION_EVALUATION_SHADER = TaskAccessConstsPartial<TaskStage::TESSELLATION_EVALUATION_SHADER>;
        using TES = TESSELLATION_EVALUATION_SHADER;
        using GEOMETRY_SHADER = TaskAccessConstsPartial<TaskStage::GEOMETRY_SHADER>;
        using GS = GEOMETRY_SHADER;
        using FRAGMENT_SHADER = TaskAccessConstsPartial<TaskStage::FRAGMENT_SHADER>;
        using FS = FRAGMENT_SHADER;
        using COMPUTE_SHADER = TaskAccessConstsPartial<TaskStage::COMPUTE_SHADER>;
        using CS = COMPUTE_SHADER;
        using RAY_TRACING_SHADER = TaskAccessConstsPartial<TaskStage::RAY_TRACING_SHADER>;
        using RT = RAY_TRACING_SHADER;
        using TASK_SHADER = TaskAccessConstsPartial<TaskStage::TASK_SHADER>;
        using TS = TASK_SHADER;
        using MESH_SHADER = TaskAccessConstsPartial<TaskStage::MESH_SHADER>;
        using MS = MESH_SHADER;
        using PRE_RASTERIZATION_SHADERS = TaskAccessConstsPartial<TaskStage::PRE_RASTERIZATION_SHADERS>;
        using PRS = PRE_RASTERIZATION_SHADERS;
        using RASTER_SHADER = TaskAccessConstsPartial<TaskStage::RASTER_SHADER>;
        using RS = RASTER_SHADER;
        using SHADER = TaskAccessConstsPartial<TaskStage::SHADER>;
        using S = SHADER;
        using DEPTH_STENCIL_ATTACHMENT = TaskAccessConstsPartial<TaskStage::DEPTH_STENCIL_ATTACHMENT, TaskAttachmentType::IMAGE>;
        using DSA = DEPTH_STENCIL_ATTACHMENT;
        using RESOLVE = TaskAccessConstsPartial<TaskStage::RESOLVE, TaskAttachmentType::IMAGE>;
        using TRANSFER = TaskAccessConstsPartial<TaskStage::TRANSFER>;
        using TF = TRANSFER;
        using HOST = TaskAccessConstsPartial<TaskStage::HOST>;
        using H = HOST;
        using ACCELERATION_STRUCTURE_BUILD = TaskAccessConstsPartial<TaskStage::AS_BUILD>;
        using ASB = ACCELERATION_STRUCTURE_BUILD;
        using ANY_COMMAND = TaskAccessConstsPartial<TaskStage::ANY_COMMAND>;
        using ANY = ANY_COMMAND;
        static constexpr TaskAccess NONE = TaskAccess{TaskStage::NONE, TaskAccessType::NONE};
        static constexpr TaskAccess READ = TaskAccess{TaskStage::NONE, TaskAccessType::READ};
        static constexpr TaskAccess WRITE = TaskAccess{TaskStage::NONE, TaskAccessType::WRITE};
        static constexpr TaskAccess WRITE_CONCURRENT = TaskAccess{TaskStage::NONE, TaskAccessType::WRITE_CONCURRENT};
        static constexpr TaskAccess READ_WRITE = TaskAccess{TaskStage::NONE, TaskAccessType::READ_WRITE};
        static constexpr TaskAccess READ_WRITE_CONCURRENT = TaskAccess{TaskStage::NONE, TaskAccessType::READ_WRITE_CONCURRENT};
        static constexpr TaskAccess SAMPLED = TaskAccess{TaskStage::NONE, TaskAccessType::SAMPLED, TaskAttachmentType::IMAGE};

        static constexpr TaskAccess COLOR_ATTACHMENT = TaskAccess{TaskStage::COLOR_ATTACHMENT, TaskAccessType::READ_WRITE, TaskAttachmentType::IMAGE};
        static constexpr TaskAccess CA = COLOR_ATTACHMENT;
        static constexpr TaskAccess PRESENT = TaskAccess{TaskStage::PRESENT, TaskAccessType::READ, TaskAttachmentType::IMAGE};
        static constexpr TaskAccess INDIRECT_COMMAND_READ = TaskAccess{TaskStage::INDIRECT_COMMAND, TaskAccessType::READ, TaskAttachmentType::BUFFER};
        static constexpr TaskAccess ICR = INDIRECT_COMMAND_READ;
        static constexpr TaskAccess INDEX_INPUT_READ = TaskAccess{TaskStage::INDEX_INPUT, TaskAccessType::READ, TaskAttachmentType::BUFFER};
        static constexpr TaskAccess IDXR = INDEX_INPUT_READ;

        // Backwards Compatibiliy Constants:

        static constexpr TaskAccess GRAPHICS_SHADER_READ = TaskAccessConsts::RS::READ;
        static constexpr TaskAccess GRAPHICS_SHADER_WRITE = TaskAccessConsts::RS::READ;
        static constexpr TaskAccess GRAPHICS_SHADER_READ_WRITE = TaskAccessConsts::RS::READ_WRITE;
        static constexpr TaskAccess GRAPHICS_SHADER_READ_WRITE_CONCURRENT = TaskAccessConsts::RS::READ_WRITE_CONCURRENT;
        static constexpr TaskAccess COMPUTE_SHADER_READ = TaskAccessConsts::COMPUTE_SHADER::READ;
        static constexpr TaskAccess COMPUTE_SHADER_WRITE = TaskAccessConsts::COMPUTE_SHADER::WRITE;
        static constexpr TaskAccess COMPUTE_SHADER_READ_WRITE = TaskAccessConsts::COMPUTE_SHADER::READ_WRITE;
        static constexpr TaskAccess COMPUTE_SHADER_READ_WRITE_CONCURRENT = TaskAccessConsts::COMPUTE_SHADER::READ_WRITE_CONCURRENT;
        static constexpr TaskAccess RAY_TRACING_SHADER_READ = TaskAccessConsts::RAY_TRACING_SHADER::READ;
        static constexpr TaskAccess RAY_TRACING_SHADER_WRITE = TaskAccessConsts::RAY_TRACING_SHADER::WRITE;
        static constexpr TaskAccess RAY_TRACING_SHADER_READ_WRITE = TaskAccessConsts::RAY_TRACING_SHADER::READ_WRITE;
        static constexpr TaskAccess RAY_TRACING_SHADER_READ_WRITE_CONCURRENT = TaskAccessConsts::RAY_TRACING_SHADER::READ_WRITE_CONCURRENT;
        static constexpr TaskAccess TASK_SHADER_READ = TaskAccessConsts::TASK_SHADER::READ;
        static constexpr TaskAccess TASK_SHADER_WRITE = TaskAccessConsts::TASK_SHADER::WRITE;
        static constexpr TaskAccess TASK_SHADER_READ_WRITE = TaskAccessConsts::TASK_SHADER::READ_WRITE;
        static constexpr TaskAccess TASK_SHADER_READ_WRITE_CONCURRENT = TaskAccessConsts::TASK_SHADER::READ_WRITE_CONCURRENT;
        static constexpr TaskAccess MESH_SHADER_READ = TaskAccessConsts::MESH_SHADER::READ;
        static constexpr TaskAccess MESH_SHADER_WRITE = TaskAccessConsts::MESH_SHADER::WRITE;
        static constexpr TaskAccess MESH_SHADER_READ_WRITE = TaskAccessConsts::MESH_SHADER::READ_WRITE;
        static constexpr TaskAccess MESH_SHADER_READ_WRITE_CONCURRENT = TaskAccessConsts::MESH_SHADER::READ_WRITE_CONCURRENT;
        static constexpr TaskAccess VERTEX_SHADER_READ = TaskAccessConsts::VERTEX_SHADER::READ;
        static constexpr TaskAccess VERTEX_SHADER_WRITE = TaskAccessConsts::VERTEX_SHADER::WRITE;
        static constexpr TaskAccess VERTEX_SHADER_READ_WRITE = TaskAccessConsts::VERTEX_SHADER::READ_WRITE;
        static constexpr TaskAccess VERTEX_SHADER_READ_WRITE_CONCURRENT = TaskAccessConsts::VERTEX_SHADER::READ_WRITE_CONCURRENT;
        static constexpr TaskAccess TESSELLATION_CONTROL_SHADER_READ = TaskAccessConsts::TESSELLATION_CONTROL_SHADER::READ;
        static constexpr TaskAccess TESSELLATION_CONTROL_SHADER_WRITE = TaskAccessConsts::TESSELLATION_CONTROL_SHADER::WRITE;
        static constexpr TaskAccess TESSELLATION_CONTROL_SHADER_READ_WRITE = TaskAccessConsts::TESSELLATION_CONTROL_SHADER::READ_WRITE;
        static constexpr TaskAccess TESSELLATION_CONTROL_SHADER_READ_WRITE_CONCURRENT = TaskAccessConsts::TESSELLATION_CONTROL_SHADER::READ_WRITE_CONCURRENT;
        static constexpr TaskAccess TESSELLATION_EVALUATION_SHADER_READ = TaskAccessConsts::TESSELLATION_EVALUATION_SHADER::READ;
        static constexpr TaskAccess TESSELLATION_EVALUATION_SHADER_WRITE = TaskAccessConsts::TESSELLATION_EVALUATION_SHADER::WRITE;
        static constexpr TaskAccess TESSELLATION_EVALUATION_SHADER_READ_WRITE = TaskAccessConsts::TESSELLATION_EVALUATION_SHADER::READ_WRITE;
        static constexpr TaskAccess TESSELLATION_EVALUATION_SHADER_READ_WRITE_CONCURRENT = TaskAccessConsts::TESSELLATION_EVALUATION_SHADER::READ_WRITE_CONCURRENT;
        static constexpr TaskAccess GEOMETRY_SHADER_READ = TaskAccessConsts::GEOMETRY_SHADER::READ;
        static constexpr TaskAccess GEOMETRY_SHADER_WRITE = TaskAccessConsts::GEOMETRY_SHADER::WRITE;
        static constexpr TaskAccess GEOMETRY_SHADER_READ_WRITE = TaskAccessConsts::GEOMETRY_SHADER::READ_WRITE;
        static constexpr TaskAccess GEOMETRY_SHADER_READ_WRITE_CONCURRENT = TaskAccessConsts::GEOMETRY_SHADER::READ_WRITE_CONCURRENT;
        static constexpr TaskAccess FRAGMENT_SHADER_READ = TaskAccessConsts::FRAGMENT_SHADER::READ;
        static constexpr TaskAccess FRAGMENT_SHADER_WRITE = TaskAccessConsts::FRAGMENT_SHADER::WRITE;
        static constexpr TaskAccess FRAGMENT_SHADER_READ_WRITE = TaskAccessConsts::FRAGMENT_SHADER::READ_WRITE;
        static constexpr TaskAccess FRAGMENT_SHADER_READ_WRITE_CONCURRENT = TaskAccessConsts::FRAGMENT_SHADER::READ_WRITE_CONCURRENT;
        static constexpr TaskAccess INDEX_READ = TaskAccessConsts::INDEX_INPUT_READ;
        static constexpr TaskAccess DRAW_INDIRECT_INFO_READ = TaskAccessConsts::INDIRECT_COMMAND_READ;
        static constexpr TaskAccess TRANSFER_READ = TaskAccessConsts::TRANSFER::READ;
        static constexpr TaskAccess TRANSFER_WRITE = TaskAccessConsts::TRANSFER::WRITE;
        static constexpr TaskAccess TRANSFER_READ_WRITE = TaskAccessConsts::TRANSFER::READ_WRITE;
        static constexpr TaskAccess HOST_TRANSFER_READ = TaskAccessConsts::HOST::READ;
        static constexpr TaskAccess HOST_TRANSFER_WRITE = TaskAccessConsts::HOST::WRITE;
        static constexpr TaskAccess HOST_TRANSFER_READ_WRITE = TaskAccessConsts::HOST::READ_WRITE;
        static constexpr TaskAccess ACCELERATION_STRUCTURE_BUILD_READ = TaskAccessConsts::ACCELERATION_STRUCTURE_BUILD::READ;
        static constexpr TaskAccess ACCELERATION_STRUCTURE_BUILD_WRITE = TaskAccessConsts::ACCELERATION_STRUCTURE_BUILD::WRITE;
        static constexpr TaskAccess ACCELERATION_STRUCTURE_BUILD_READ_WRITE = TaskAccessConsts::ACCELERATION_STRUCTURE_BUILD::READ_WRITE;

        static constexpr TaskAccess SHADER_SAMPLED = TaskAccessConsts::SHADER::SAMPLED;
        static constexpr TaskAccess SHADER_STORAGE_WRITE_ONLY = TaskAccessConsts::SHADER::WRITE;
        static constexpr TaskAccess SHADER_STORAGE_READ_ONLY = TaskAccessConsts::SHADER::READ;
        static constexpr TaskAccess SHADER_STORAGE_READ_WRITE = TaskAccessConsts::SHADER::READ_WRITE;
        static constexpr TaskAccess SHADER_STORAGE_READ_WRITE_CONCURRENT = TaskAccessConsts::SHADER::READ_WRITE_CONCURRENT;
        static constexpr TaskAccess GRAPHICS_SHADER_SAMPLED = TaskAccessConsts::RS::SAMPLED;
        static constexpr TaskAccess GRAPHICS_SHADER_STORAGE_WRITE_ONLY = TaskAccessConsts::RS::WRITE;
        static constexpr TaskAccess GRAPHICS_SHADER_STORAGE_READ_ONLY = TaskAccessConsts::RS::READ;
        static constexpr TaskAccess GRAPHICS_SHADER_STORAGE_READ_WRITE = TaskAccessConsts::RS::READ_WRITE;
        static constexpr TaskAccess GRAPHICS_SHADER_STORAGE_READ_WRITE_CONCURRENT = TaskAccessConsts::RS::READ_WRITE_CONCURRENT;
        static constexpr TaskAccess COMPUTE_SHADER_SAMPLED = TaskAccessConsts::CS::SAMPLED;
        static constexpr TaskAccess COMPUTE_SHADER_STORAGE_WRITE_ONLY = TaskAccessConsts::CS::WRITE;
        static constexpr TaskAccess COMPUTE_SHADER_STORAGE_READ_ONLY = TaskAccessConsts::CS::READ;
        static constexpr TaskAccess COMPUTE_SHADER_STORAGE_READ_WRITE = TaskAccessConsts::CS::READ_WRITE;
        static constexpr TaskAccess COMPUTE_SHADER_STORAGE_READ_WRITE_CONCURRENT = TaskAccessConsts::CS::READ_WRITE_CONCURRENT;
        static constexpr TaskAccess RAY_TRACING_SHADER_SAMPLED = TaskAccessConsts::RT::SAMPLED;
        static constexpr TaskAccess RAY_TRACING_SHADER_STORAGE_WRITE_ONLY = TaskAccessConsts::RT::WRITE;
        static constexpr TaskAccess RAY_TRACING_SHADER_STORAGE_READ_ONLY = TaskAccessConsts::RT::READ;
        static constexpr TaskAccess RAY_TRACING_SHADER_STORAGE_READ_WRITE = TaskAccessConsts::RT::READ_WRITE;
        static constexpr TaskAccess RAY_TRACING_SHADER_STORAGE_READ_WRITE_CONCURRENT = TaskAccessConsts::RT::READ_WRITE_CONCURRENT;
        static constexpr TaskAccess TASK_SHADER_SAMPLED = TaskAccessConsts::TS::SAMPLED;
        static constexpr TaskAccess TASK_SHADER_STORAGE_WRITE_ONLY = TaskAccessConsts::TS::WRITE;
        static constexpr TaskAccess TASK_SHADER_STORAGE_READ_ONLY = TaskAccessConsts::TS::READ;
        static constexpr TaskAccess TASK_SHADER_STORAGE_READ_WRITE = TaskAccessConsts::TS::READ_WRITE;
        static constexpr TaskAccess TASK_SHADER_STORAGE_READ_WRITE_CONCURRENT = TaskAccessConsts::TS::READ_WRITE_CONCURRENT;
        static constexpr TaskAccess MESH_SHADER_SAMPLED = TaskAccessConsts::MS::SAMPLED;
        static constexpr TaskAccess MESH_SHADER_STORAGE_WRITE_ONLY = TaskAccessConsts::MS::WRITE;
        static constexpr TaskAccess MESH_SHADER_STORAGE_READ_ONLY = TaskAccessConsts::MS::READ;
        static constexpr TaskAccess MESH_SHADER_STORAGE_READ_WRITE = TaskAccessConsts::MS::READ_WRITE;
        static constexpr TaskAccess MESH_SHADER_STORAGE_READ_WRITE_CONCURRENT = TaskAccessConsts::MS::READ_WRITE_CONCURRENT;
        static constexpr TaskAccess VERTEX_SHADER_SAMPLED = TaskAccessConsts::VS::SAMPLED;
        static constexpr TaskAccess VERTEX_SHADER_STORAGE_WRITE_ONLY = TaskAccessConsts::VS::WRITE;
        static constexpr TaskAccess VERTEX_SHADER_STORAGE_READ_ONLY = TaskAccessConsts::VS::READ;
        static constexpr TaskAccess VERTEX_SHADER_STORAGE_READ_WRITE = TaskAccessConsts::VS::READ_WRITE;
        static constexpr TaskAccess VERTEX_SHADER_STORAGE_READ_WRITE_CONCURRENT = TaskAccessConsts::VS::READ_WRITE_CONCURRENT;
        static constexpr TaskAccess TESSELLATION_CONTROL_SHADER_SAMPLED = TaskAccessConsts::TCS::SAMPLED;
        static constexpr TaskAccess TESSELLATION_CONTROL_SHADER_STORAGE_WRITE_ONLY = TaskAccessConsts::TCS::WRITE;
        static constexpr TaskAccess TESSELLATION_CONTROL_SHADER_STORAGE_READ_ONLY = TaskAccessConsts::TCS::READ;
        static constexpr TaskAccess TESSELLATION_CONTROL_SHADER_STORAGE_READ_WRITE = TaskAccessConsts::TCS::READ_WRITE;
        static constexpr TaskAccess TESSELLATION_CONTROL_SHADER_STORAGE_READ_WRITE_CONCURRENT = TaskAccessConsts::TCS::READ_WRITE_CONCURRENT;
        static constexpr TaskAccess TESSELLATION_EVALUATION_SHADER_SAMPLED = TaskAccessConsts::TES::SAMPLED;
        static constexpr TaskAccess TESSELLATION_EVALUATION_SHADER_STORAGE_WRITE_ONLY = TaskAccessConsts::TES::WRITE;
        static constexpr TaskAccess TESSELLATION_EVALUATION_SHADER_STORAGE_READ_ONLY = TaskAccessConsts::TES::READ;
        static constexpr TaskAccess TESSELLATION_EVALUATION_SHADER_STORAGE_READ_WRITE = TaskAccessConsts::TES::READ_WRITE;
        static constexpr TaskAccess TESSELLATION_EVALUATION_SHADER_STORAGE_READ_WRITE_CONCURRENT = TaskAccessConsts::TES::READ_WRITE_CONCURRENT;
        static constexpr TaskAccess GEOMETRY_SHADER_SAMPLED = TaskAccessConsts::GS::SAMPLED;
        static constexpr TaskAccess GEOMETRY_SHADER_STORAGE_WRITE_ONLY = TaskAccessConsts::GS::WRITE;
        static constexpr TaskAccess GEOMETRY_SHADER_STORAGE_READ_ONLY = TaskAccessConsts::GS::READ;
        static constexpr TaskAccess GEOMETRY_SHADER_STORAGE_READ_WRITE = TaskAccessConsts::GS::READ_WRITE;
        static constexpr TaskAccess GEOMETRY_SHADER_STORAGE_READ_WRITE_CONCURRENT = TaskAccessConsts::GS::READ_WRITE_CONCURRENT;
        static constexpr TaskAccess FRAGMENT_SHADER_SAMPLED = TaskAccessConsts::FS::SAMPLED;
        static constexpr TaskAccess FRAGMENT_SHADER_STORAGE_WRITE_ONLY = TaskAccessConsts::FS::WRITE;
        static constexpr TaskAccess FRAGMENT_SHADER_STORAGE_READ_ONLY = TaskAccessConsts::FS::READ;
        static constexpr TaskAccess FRAGMENT_SHADER_STORAGE_READ_WRITE = TaskAccessConsts::FS::READ_WRITE;
        static constexpr TaskAccess FRAGMENT_SHADER_STORAGE_READ_WRITE_CONCURRENT = TaskAccessConsts::FS::READ_WRITE_CONCURRENT;
        static constexpr TaskAccess DEPTH_ATTACHMENT = TaskAccessConsts::DSA::READ_WRITE;
        static constexpr TaskAccess STENCIL_ATTACHMENT = TaskAccessConsts::DSA::READ_WRITE;
        static constexpr TaskAccess DEPTH_ATTACHMENT_READ = TaskAccessConsts::DSA::SAMPLED;
        static constexpr TaskAccess STENCIL_ATTACHMENT_READ = TaskAccessConsts::DSA::SAMPLED;
        static constexpr TaskAccess DEPTH_STENCIL_ATTACHMENT_READ = TaskAccessConsts::DSA::SAMPLED;
        static constexpr TaskAccess RESOLVE_WRITE = TaskAccessConsts::RESOLVE::READ_WRITE;
    };

    // Backwards Compatibiliy Usings:
    using TaskBufferAccess = TaskAccessConsts;
    using TaskBlasAccess = TaskAccessConsts;
    using TaskTlasAccess = TaskAccessConsts;
    using TaskImageAccess = TaskAccessConsts;

    enum struct TaskType : u16
    {
        UNDEFINED,
        GENERAL,
        RASTER,
        COMPUTE,
        RAY_TRACING,
        TRANSFER
    };

    auto to_string(TaskType task_type) -> std::string_view;

    auto task_type_allowed_stages(TaskType task_type, TaskStage stage) -> bool;

    auto task_type_default_stage(TaskType task_type) -> TaskStage;

    using TaskResourceIndex = u32;

    struct DAXA_EXPORT_CXX TaskGPUResourceView
    {
        TaskResourceIndex task_graph_index = {};
        TaskResourceIndex index = {};

        auto is_empty() const -> bool;
        auto is_persistent() const -> bool;
        auto is_null() const -> bool;

        auto operator<=>(TaskGPUResourceView const & other) const = default;
    };

    [[nodiscard]] DAXA_EXPORT_CXX auto to_string(TaskGPUResourceView const & id) -> std::string;

    struct TaskBufferView : public TaskGPUResourceView
    {
        TaskStage stage_override = {};
        TaskAccessType access_type_override = {};
        auto override_stage(TaskStage stage) const -> TaskBufferView
        {
            auto ret = *this;
            ret.stage_override = stage;
            return ret;
        }
        auto override_access_type(TaskAccessType access_type) const -> TaskBufferView
        {
            auto ret = *this;
            ret.access_type_override = access_type;
            return ret;
        }
        using ID_T = BufferId;
    };

    struct TaskBlasView : public TaskGPUResourceView
    {
        TaskStage stage_override = {};
        TaskAccessType access_type_override = {};
        auto override_stage(TaskStage stage) const -> TaskBlasView
        {
            auto ret = *this;
            ret.stage_override = stage;
            return ret;
        }
        auto override_access_type(TaskAccessType access_type) const -> TaskBlasView
        {
            auto ret = *this;
            ret.access_type_override = access_type;
            return ret;
        }
        using ID_T = BlasId;
    };

    struct TaskTlasView : public TaskGPUResourceView
    {
        TaskStage stage_override = {};
        TaskAccessType access_type_override = {};
        auto override_stage(TaskStage stage) const -> TaskTlasView
        {
            auto ret = *this;
            ret.stage_override = stage;
            return ret;
        }
        auto override_access_type(TaskAccessType access_type) const -> TaskTlasView
        {
            auto ret = *this;
            ret.access_type_override = access_type;
            return ret;
        }
        using ID_T = TlasId;
    };

    struct TaskAttachmentInfo;

    struct TaskImageView : public TaskGPUResourceView
    {
        daxa::ImageMipArraySlice slice = {};
        ImageViewType view_type_override = ImageViewType::MAX_ENUM;
        TaskStage stage_override = {};
        TaskAccessType access_type_override = {};
        auto override_stage(TaskStage stage) const -> TaskImageView
        {
            auto ret = *this;
            ret.stage_override = stage;
            return ret;
        }
        auto override_access_type(TaskAccessType access_type) const -> TaskImageView
        {
            auto ret = *this;
            ret.access_type_override = access_type;
            return ret;
        }
        auto view(daxa::ImageMipArraySlice const & new_slice) const -> TaskImageView
        {
            auto ret = *this;
            ret.slice = new_slice;
            return ret;
        }
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
        auto override_view_type(ImageViewType view_type) const -> TaskImageView
        {
            auto ret = *this;
            ret.view_type_override = view_type;
            return ret;
        }
        auto operator<=>(TaskGPUResourceView const & other) const = delete;
        auto operator<=>(TaskImageView const & other) const = default;
    };

    static constexpr inline TaskBufferView NullTaskBuffer = []()
    {
        TaskBufferView ret = {};
        ret.task_graph_index = std::numeric_limits<u32>::max();
        ret.index = std::numeric_limits<u32>::max();
        return ret;
    }();

    static constexpr inline TaskBlasView NullTaskBlas = {NullTaskBuffer};

    static constexpr inline TaskTlasView NullTaskTlas = {NullTaskBuffer};

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

    enum struct TaskHeadImageArrayType : u8
    {
        RUNTIME_IMAGES,
        MIP_LEVELS,
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

    struct UndefinedAttachment
    {
    };

    struct TaskBufferAttachment
    {
        using INDEX_TYPE = TaskBufferAttachmentIndex;
        static constexpr TaskAttachmentType ATTACHMENT_TYPE = TaskAttachmentType::BUFFER;
        char const * name = {};
        TaskAccess task_access = {};
        Access access = {};
        u8 shader_array_size = {};
        bool shader_as_address = {};
    };

    struct TaskBlasAttachment
    {
        using INDEX_TYPE = TaskBlasAttachmentIndex;
        static constexpr TaskAttachmentType ATTACHMENT_TYPE = TaskAttachmentType::BLAS;
        char const * name = {};
        TaskAccess task_access = {};
        Access access = {};
    };

    struct TaskTlasAttachment
    {
        using INDEX_TYPE = TaskTlasAttachmentIndex;
        static constexpr TaskAttachmentType ATTACHMENT_TYPE = TaskAttachmentType::TLAS;
        char const * name = {};
        TaskAccess task_access = {};
        Access access = {};
        bool shader_as_address = {};
    };

    struct TaskImageAttachment
    {
        using INDEX_TYPE = TaskImageAttachmentIndex;
        static constexpr TaskAttachmentType ATTACHMENT_TYPE = TaskAttachmentType::IMAGE;
        char const * name = {};
        TaskAccess task_access = {};
        Access access = {};
        ImageViewType view_type = ImageViewType::MAX_ENUM;
        u8 shader_array_size = {};
        bool shader_as_index = {};
        TaskHeadImageArrayType shader_array_type = {};
    };

    struct TaskBufferInlineAttachment
    {
        char const * name = {};
        TaskAccess access = {};
        u8 shader_array_size = {};
        bool shader_as_address = {};
        TaskBufferView view = {};
    };

    struct TaskBlasInlineAttachment
    {
        char const * name = {};
        TaskAccess access = {};
        TaskBlasView view = {};
    };

    struct TaskTlasInlineAttachment
    {
        char const * name = {};
        TaskAccess access = {};
        TaskTlasView view = {};
    };

    struct TaskImageInlineAttachment
    {
        char const * name = {};
        TaskAccess access = {};
        ImageViewType view_type = ImageViewType::MAX_ENUM;
        u8 shader_array_size = {};
        TaskHeadImageArrayType shader_array_type = {};
        TaskImageView view = {};
    };

    template <typename T>
    concept IsTaskResourceAttachment =
        std::is_same_v<T, TaskBufferAttachment> ||
        std::is_same_v<T, TaskBlasAttachment> ||
        std::is_same_v<T, TaskTlasAttachment> ||
        std::is_same_v<T, TaskImageAttachment>;

    struct TaskAttachment
    {
        TaskAttachmentType type = TaskAttachmentType::UNDEFINED;
        union Value
        {
            UndefinedAttachment undefined;
            TaskBufferAttachment buffer;
            TaskBlasAttachment blas;
            TaskTlasAttachment tlas;
            TaskImageAttachment image;
        } value = {.undefined = {}};

        constexpr TaskAttachment() = default;

        constexpr TaskAttachment(TaskBufferAttachment const & buffer)
            : type{TaskAttachmentType::BUFFER}, value{.buffer = buffer}
        {
        }

        constexpr TaskAttachment(TaskBlasAttachment const & blas)
            : type{TaskAttachmentType::BLAS}, value{.blas = blas}
        {
        }

        constexpr TaskAttachment(TaskTlasAttachment const & tlas)
            : type{TaskAttachmentType::TLAS}, value{.tlas = tlas}
        {
        }

        constexpr TaskAttachment(TaskImageAttachment const & image)
            : type{TaskAttachmentType::IMAGE}, value{.image = image}
        {
        }

        constexpr auto name() const -> char const *
        {
            switch (type)
            {
            case TaskAttachmentType::BUFFER: return value.buffer.name;
            case TaskAttachmentType::BLAS: return value.blas.name;
            case TaskAttachmentType::TLAS: return value.tlas.name;
            case TaskAttachmentType::IMAGE: return value.image.name;
            default: return "undefined";
            }
        }

        constexpr auto shader_array_size() const -> u32
        {
            switch (type)
            {
            case TaskAttachmentType::BUFFER: return value.buffer.shader_array_size * 8;
            case TaskAttachmentType::BLAS: return 0;
            case TaskAttachmentType::TLAS: return 8;
            case TaskAttachmentType::IMAGE: return value.image.shader_array_size * (value.image.shader_as_index ? 4 : 8);
            default: return 0;
            }
        }

        constexpr auto shader_element_align() const -> u32
        {
            switch (type)
            {
            case TaskAttachmentType::BUFFER: return 8;
            case TaskAttachmentType::BLAS: return 8;
            case TaskAttachmentType::TLAS: return 8;
            case TaskAttachmentType::IMAGE: return value.image.shader_as_index ? 4 : 8;
            default: return 0;
            }
        }
    };

    struct UndefinedAttachmentRuntimeData
    {
    };

    struct TaskBufferAttachmentInfo : TaskBufferAttachment
    {
        TaskBufferView view = {};
        TaskBufferView translated_view = {};
        std::span<BufferId const> ids = {};
    };

    struct TaskBlasAttachmentInfo : TaskBlasAttachment
    {
        TaskBlasView view = {};
        TaskBlasView translated_view = {};
        std::span<BlasId const> ids = {};
    };

    struct TaskTlasAttachmentInfo : TaskTlasAttachment
    {
        TaskTlasView view = {};
        TaskTlasView translated_view = {};
        std::span<TlasId const> ids = {};
    };

    struct TaskImageAttachmentInfo : TaskImageAttachment
    {
        TaskImageView view = {};
        TaskImageView translated_view = {};
        ImageLayout layout = {};
        std::span<ImageId const> ids = {};
        std::span<ImageViewId const> view_ids = {};
    };

    struct TaskAttachmentInfo
    {
        TaskAttachmentType type = TaskAttachmentType::UNDEFINED;
        union Value
        {
            UndefinedAttachment undefined;
            TaskBufferAttachmentInfo buffer;
            TaskBlasAttachmentInfo blas;
            TaskTlasAttachmentInfo tlas;
            TaskImageAttachmentInfo image;
        } value = {.undefined = {}};

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

        constexpr auto name() const -> char const *
        {
            switch (type)
            {
            case TaskAttachmentType::BUFFER: return value.buffer.name;
            case TaskAttachmentType::BLAS: return value.blas.name;
            case TaskAttachmentType::TLAS: return value.tlas.name;
            case TaskAttachmentType::IMAGE: return value.image.name;
            default: return "undefined";
            }
        }

        constexpr auto shader_array_size() const -> u32
        {
            switch (type)
            {
            case TaskAttachmentType::BUFFER: return value.buffer.shader_array_size * 8;
            case TaskAttachmentType::BLAS: return 0;
            case TaskAttachmentType::TLAS: return 8;
            case TaskAttachmentType::IMAGE: return value.image.shader_array_size * (value.image.shader_as_index ? 4 : 8);
            default: return 0;
            }
        }

        constexpr auto shader_element_align() const -> u32
        {
            switch (type)
            {
            case TaskAttachmentType::BUFFER: return 8;
            case TaskAttachmentType::BLAS: return 8;
            case TaskAttachmentType::TLAS: return 8;
            case TaskAttachmentType::IMAGE: return value.image.shader_as_index ? 4 : 8;
            default: return 0;
            }
        }
    };

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

#if !DAXA_REMOVE_DEPRECATED
        [[deprecated("Use AttachmentBlob(std::span<std::byte const>) constructor instead, API:3.0")]] void assign_attachment_shader_blob(std::span<std::byte> arr) const
        {
            std::memcpy(
                arr.data(),
                attachment_shader_blob.data(),
                attachment_shader_blob.size());
        }
#endif

        auto get(TaskBufferAttachmentIndex index) const -> TaskBufferAttachmentInfo const &;
        auto get(TaskBufferView view) const -> TaskBufferAttachmentInfo const &;
        auto get(TaskBlasAttachmentIndex index) const -> TaskBlasAttachmentInfo const &;
        auto get(TaskBlasView view) const -> TaskBlasAttachmentInfo const &;
        auto get(TaskTlasAttachmentIndex index) const -> TaskTlasAttachmentInfo const &;
        auto get(TaskTlasView view) const -> TaskTlasAttachmentInfo const &;
        auto get(TaskImageAttachmentIndex index) const -> TaskImageAttachmentInfo const &;
        auto get(TaskImageView view) const -> TaskImageAttachmentInfo const &;
        auto get(usize index) const -> TaskAttachmentInfo const &;

        auto layout(TaskImageIndexOrView auto timage, [[maybe_unused]] u32 array_index = 0) const -> ImageLayout
        {
            return this->get(timage).layout;
        }
        auto info(TaskIndexOrView auto tresource, u32 array_index = 0) const
        {
            return this->device.info(this->get(tresource).ids[array_index]);
        }
        auto image_view_info(TaskImageIndexOrView auto timage, u32 array_index = 0) const -> Optional<ImageViewInfo>
        {
            return this->device.image_view_info(this->get(timage).view_ids[array_index]);
        }
        auto device_address(TaskBufferBlasOrTlasIndexOrView auto tresource, u32 array_index = 0) const -> Optional<DeviceAddress>
        {
            return this->device.device_address(this->get(tresource).ids[array_index]);
        }
        auto buffer_device_address(TaskBufferBlasOrTlasIndexOrView auto tresource, u32 array_index = 0) const -> Optional<DeviceAddress>
        {
            return this->device.device_address(this->get(tresource).ids[array_index]);
        }
        auto host_address(TaskBufferIndexOrView auto tbuffer, u32 array_index = 0) const -> Optional<std::byte *>
        {
            return this->device.buffer_host_address(this->get(tbuffer).ids[array_index]);
        }
        auto buffer_host_address(TaskBufferIndexOrView auto tbuffer, u32 array_index = 0) const -> Optional<std::byte *>
        {
            return this->device.buffer_host_address(this->get(tbuffer).ids[array_index]);
        }
        auto id(TaskIndexOrView auto tresource, u32 index = 0)
        {
            return this->get(tresource).ids[index];
        }
        auto view(TaskImageIndexOrView auto timg, u32 index = 0)
        {
            return this->get(timg).view_ids[index];
        }
    };

    struct TrackedBuffers
    {
        std::span<BufferId const> buffers = {};
        Access latest_access = {};
    };

    struct TaskBufferInfo
    {
        TrackedBuffers initial_buffers = {};
        std::string name = {};
    };

    struct ImplPersistentTaskBufferBlasTlas;
    struct DAXA_EXPORT_CXX TaskBuffer : ManagedPtr<TaskBuffer, ImplPersistentTaskBufferBlasTlas *>
    {
        TaskBuffer() = default;
        TaskBuffer(TaskBufferInfo const & info);
        TaskBuffer(daxa::Device & device, BufferInfo const & info);

        operator TaskBufferView() const;

        auto view() const -> TaskBufferView;
        /// THREADSAFETY:
        /// * reference MUST NOT be read after the object is destroyed.
        /// @return reference to info of object.
        auto info() const -> TaskBufferInfo const &;
        auto get_state() const -> TrackedBuffers;
        auto is_owning() const -> bool;

        void set_buffers(TrackedBuffers const & buffers);
        void swap_buffers(TaskBuffer & other);

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
    };

    struct TrackedBlas
    {
        std::span<BlasId const> blas = {};
        Access latest_access = {};
    };

    struct TaskBlasInfo
    {
        TrackedBlas initial_blas = {};
        std::string name = {};
    };

    struct DAXA_EXPORT_CXX TaskBlas : ManagedPtr<TaskBlas, ImplPersistentTaskBufferBlasTlas *>
    {
        TaskBlas() = default;
        TaskBlas(TaskBlasInfo const & info);

        operator TaskBlasView() const;

        auto view() const -> TaskBlasView;
        /// THREADSAFETY:
        /// * reference MUST NOT be read after the object is destroyed.
        /// @return reference to info of object.
        auto info() const -> TaskBlasInfo const &;
        auto get_state() const -> TrackedBlas;

        void set_blas(TrackedBlas const & blas);
        void swap_blas(TaskBlas & other);

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
    };

    struct TrackedTlas
    {
        std::span<TlasId const> tlas = {};
        Access latest_access = {};
    };

    struct TaskTlasInfo
    {
        TrackedTlas initial_tlas = {};
        std::string name = {};
    };

    struct DAXA_EXPORT_CXX TaskTlas : ManagedPtr<TaskTlas, ImplPersistentTaskBufferBlasTlas *>
    {
        TaskTlas() = default;
        TaskTlas(TaskTlasInfo const & info);

        operator TaskTlasView() const;

        auto view() const -> TaskTlasView;
        /// THREADSAFETY:
        /// * reference MUST NOT be read after the object is destroyed.
        /// @return reference to info of object.
        auto info() const -> TaskTlasInfo const &;
        auto get_state() const -> TrackedTlas;

        void set_tlas(TrackedTlas const & tlas);
        void swap_tlas(TaskTlas & other);

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
    };

    struct TrackedImages
    {
        std::span<ImageId const> images = {};
        // optional:
        std::span<ImageSliceState const> latest_slice_states = {};
    };

    struct TaskImageInfo
    {
        TrackedImages initial_images = {};
        bool swapchain_image = {};
        std::string name = {};
    };

    struct ImplPersistentTaskImage;
    struct DAXA_EXPORT_CXX TaskImage : ManagedPtr<TaskImage, ImplPersistentTaskImage *>
    {
        TaskImage() = default;
        // TaskImage(TaskImage const & ti) = default;
        TaskImage(TaskImageInfo const & info);

        operator TaskImageView() const;

        auto view() const -> TaskImageView;
        /// THREADSAFETY:
        /// * reference MUST NOT be read after the object is destroyed.
        /// @return reference to info of object.
        auto info() const -> TaskImageInfo const &;
        auto get_state() const -> TrackedImages;

        void set_images(TrackedImages const & images);
        void swap_images(TaskImage & other);

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
    };

    using TaskViewVariant = Variant<
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
        TaskBufferViewOrTaskBuffer<T> || TaskBlasViewOrTaskBlas<T> || TaskTlasViewOrTaskTlas<T> || TaskImageViewOrTaskImage<T> || std::is_same_v<ImageViewType, T>;

    template <typename T>
    concept TaskImageViewOrTaskImageOrImageViewType = std::is_same_v<T, TaskImageView> || std::is_same_v<T, TaskImage> || std::is_same_v<ImageViewType, T>;

    template <typename T>
    concept TaskResourceOrViewOrAccess = TaskResourceViewOrResourceOrImageViewType<T> || std::is_same_v<T, TaskStage>;

    template <typename T>
    concept TaskImageOrViewOrAccess = TaskImageViewOrTaskImageOrImageViewType<T> || std::is_same_v<T, TaskStage>;

    inline namespace detail
    {
        struct AsbSizeAlignment
        {
            u32 size = {};
            u32 alignment = {};
        };
        template <typename T>
        auto constexpr align_up(T value, T align) -> T
        {
            if (value == 0 || align == 0)
                return 0;
            return (value + align - static_cast<T>(1)) / align * align;
        }
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

    template <usize ATTACHMENT_COUNT>
    struct AttachmentViews
    {
        AttachmentViews(std::array<daxa::TaskViewVariant, ATTACHMENT_COUNT> const & index_view_pairs)
        {
            for (TaskViewVariant const & vari : index_view_pairs)
            {
                if (auto * buffer_pair = get_if<std::pair<daxa::TaskBufferAttachmentIndex, daxa::TaskBufferView>>(&vari))
                {
                    views[buffer_pair->first.value] = buffer_pair->second;
                }
                else if (auto * blas_pair = get_if<std::pair<daxa::TaskBlasAttachmentIndex, daxa::TaskBlasView>>(&vari))
                {
                    views[blas_pair->first.value] = blas_pair->second;
                }
                else if (auto * tlas_pair = get_if<std::pair<daxa::TaskTlasAttachmentIndex, daxa::TaskTlasView>>(&vari))
                {
                    views[tlas_pair->first.value] = tlas_pair->second;
                }
                else
                {
                    auto const & img_pair = get<std::pair<daxa::TaskImageAttachmentIndex, daxa::TaskImageView>>(vari);
                    views[img_pair.first.value] = img_pair.second;
                }
            }
        }
        AttachmentViews() = default;
        std::array<Variant<
                       daxa::TaskBufferView,
                       daxa::TaskBlasView,
                       daxa::TaskTlasView,
                       daxa::TaskImageView>,
                   ATTACHMENT_COUNT>
            views = {};
    };

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

    struct TaskAttachmentViewWrapperRaw
    {
        TaskAttachmentType type = TaskAttachmentType::UNDEFINED;
        union Value
        {
            u32 undefined;
            TaskBufferView buffer;
            TaskBlasView blas;
            TaskTlasView tlas;
            TaskImageView image;
        } value = {.undefined = {}};
    };

    template <typename T>
    struct TaskAttachmentViewWrapper
    {
        TaskAttachmentViewWrapperRaw _value = {};

        TaskAttachmentViewWrapper()
        {
            if constexpr (std::is_same_v<T, TaskBufferView>)
            {
                _value = {TaskAttachmentType::BUFFER, {.buffer = TaskBufferView{}}};
            }
            if constexpr (std::is_same_v<T, TaskBlasView>)
            {
                _value = {TaskAttachmentType::BLAS, {.blas = TaskBlasView{}}};
            }
            if constexpr (std::is_same_v<T, TaskTlasView>)
            {
                _value = {TaskAttachmentType::TLAS, {.tlas = TaskTlasView{}}};
            }
            if constexpr (std::is_same_v<T, TaskImageView>)
            {
                _value = {TaskAttachmentType::IMAGE, {.image = TaskImageView{}}};
            }
        }
        TaskAttachmentViewWrapper(TaskBufferViewOrTaskBuffer auto const & v)
            requires(std::is_same_v<T, TaskBufferView>)
            : _value{TaskAttachmentType::BUFFER, {.buffer = v}}
        {
        }
        TaskAttachmentViewWrapper(TaskBlasViewOrTaskBlas auto const & v)
            requires(std::is_same_v<T, TaskBlasView>)
            : _value{TaskAttachmentType::BLAS, {.blas = v}}
        {
        }
        TaskAttachmentViewWrapper(TaskTlasViewOrTaskTlas auto const & v)
            requires(std::is_same_v<T, TaskTlasView>)
            : _value{TaskAttachmentType::TLAS, {.tlas = v}}
        {
        }
        TaskAttachmentViewWrapper(TaskImageViewOrTaskImage auto const & v)
            requires(std::is_same_v<T, TaskImageView>)
            : _value{TaskAttachmentType::IMAGE, {.image = v}}
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
            std::array<daxa::TaskAttachment, ATTACHMENT_COUNT> value = {};
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

        static auto convert(auto const & type) -> daxa::AttachmentViews<ATTACHMENT_COUNT>
        {
            // Binary compatible with TaskHeadStruct for Views type.
            struct Extractor
            {
                u32 dummy = {};
                std::array<TaskAttachmentViewWrapperRaw, ATTACHMENT_COUNT> initializers = {};
            };
            // Compilers collapse here. Usually type traits start to fail so we will just memcpy here until we get c++26 reflection.
            static constexpr u32 SIZEOF_EXTRACTOR = sizeof(Extractor);
            static constexpr u32 SIZEOF_TYPE = sizeof(decltype(type));
            static_assert(SIZEOF_TYPE == SIZEOF_EXTRACTOR, "DAXA_STATIC_ERROR: TaskAttachmentViews Extractor type abi does not match actual views type!");
            Extractor views = {};
            std::memcpy(&views, &type, SIZEOF_EXTRACTOR);
            auto ret = daxa::AttachmentViews<ATTACHMENT_COUNT>{};
            for (daxa::u32 i = 0; i < ATTACHMENT_COUNT; ++i)
            {
                switch (views.initializers[i].type)
                {
                case TaskAttachmentType::BUFFER:
                    ret.views[i] = views.initializers[i].value.buffer;
                    break;
                case TaskAttachmentType::BLAS:
                    ret.views[i] = views.initializers[i].value.blas;
                    break;
                case TaskAttachmentType::TLAS:
                    ret.views[i] = views.initializers[i].value.tlas;
                    break;
                case TaskAttachmentType::IMAGE:
                    ret.views[i] = views.initializers[i].value.image;
                    break;
                default:
                    DAXA_DBG_ASSERT_TRUE_M(false, "Invalid attachment type!");
                }
            }
            return ret;
        }
    };

#define DAXA_DECL_TASK_HEAD_BEGIN_PROTO(HEAD_NAME, HEAD_TYPE)                    \
    namespace HEAD_NAME                                                          \
    {                                                                            \
        static inline constexpr char NAME[] = #HEAD_NAME;                        \
        static inline constexpr daxa::TaskType TYPE = daxa::TaskType::HEAD_TYPE; \
        template <typename TDecl, daxa::usize ATTACHMENT_COUNT>                  \
        struct TaskHeadStruct                                                    \
        {                                                                        \
            typename TDecl::InternalT _internal = {};                            \
            operator daxa::AttachmentViews<ATTACHMENT_COUNT>()                   \
                requires(!TDecl::DECL_ATTACHMENTS)                               \
            {                                                                    \
                return TDecl::convert(*this);                                    \
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
            daxa::TaskBufferAttachment{                                        \
                .name = #NAME,                                                 \
                .task_access = daxa::TaskBufferAccess::TASK_ACCESS,            \
                __VA_ARGS__})};

#define _DAXA_HELPER_TH_BLAS(NAME, TASK_ACCESS)                              \
    typename TDecl::TaskBlasT const NAME =                                   \
        {TDecl::template process_attachment_decl<typename TDecl::TaskBlasT>( \
            _internal,                                                       \
            daxa::TaskBlasAttachment{                                        \
                .name = #NAME,                                               \
                .task_access = daxa::TaskBlasAccess::TASK_ACCESS,            \
            })};

#define _DAXA_HELPER_TH_TLAS(NAME, TASK_ACCESS, ...)                         \
    typename TDecl::TaskTlasT const NAME =                                   \
        {TDecl::template process_attachment_decl<typename TDecl::TaskTlasT>( \
            _internal,                                                       \
            daxa::TaskTlasAttachment{                                        \
                .name = #NAME,                                               \
                .task_access = daxa::TaskTlasAccess::TASK_ACCESS,            \
                __VA_ARGS__})};

#define _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, ...)                         \
    typename TDecl::TaskImageT const NAME =                                   \
        {TDecl::template process_attachment_decl<typename TDecl::TaskImageT>( \
            _internal,                                                        \
            daxa::TaskImageAttachment{                                        \
                .name = #NAME,                                                \
                .task_access = daxa::TaskImageAccess::TASK_ACCESS,            \
                __VA_ARGS__})};
#endif

#define DAXA_DECL_TASK_HEAD_END                                                                                                                \
    }                                                                                                                                          \
    ;                                                                                                                                          \
    static inline constexpr auto ATTACHMENT_COUNT = TaskHeadStruct<daxa::TaskHeadStructSpecializeAttachmentDecls<256>, 256>{}._internal.count; \
    using ATTACHMENTS_T = TaskHeadStruct<daxa::TaskHeadStructSpecializeAttachmentDecls<ATTACHMENT_COUNT>, ATTACHMENT_COUNT>;                   \
    using VIEWS_T = TaskHeadStruct<daxa::TaskHeadStructSpecializeAttachmentViews<ATTACHMENT_COUNT>, ATTACHMENT_COUNT>;                         \
    static inline constexpr auto ATTACHMENTS = ATTACHMENTS_T{};                                                                                \
    static inline constexpr auto const & AT = ATTACHMENTS;                                                                                     \
    struct alignas(daxa::detail::get_asb_size_and_alignment(AT._internal.value).alignment) AttachmentShaderBlob                                \
    {                                                                                                                                          \
        std::array<std::byte, daxa::detail::get_asb_size_and_alignment(AT._internal.value).size> value = {};                                   \
        AttachmentShaderBlob() = default;                                                                                                      \
        AttachmentShaderBlob(std::span<std::byte const> data) { *this = data; }                                                                \
        auto operator=(std::span<std::byte const> data) -> AttachmentShaderBlob &                                                              \
        {                                                                                                                                      \
            DAXA_DBG_ASSERT_TRUE_M(this->value.size() == data.size(), "Blob size missmatch!");                                                 \
            for (daxa::u32 i = 0; i < data.size(); ++i)                                                                                        \
                this->value[i] = data[i];                                                                                                      \
            return *this;                                                                                                                      \
        }                                                                                                                                      \
    };                                                                                                                                         \
    struct Task : public daxa::IPartialTask                                                                                                    \
    {                                                                                                                                          \
        static inline constexpr daxa::TaskType TASK_TYPE = TYPE;                                                                               \
        static inline constexpr char const * TASK_NAME = NAME;                                                                                 \
        using AttachmentViews = daxa::AttachmentViews<ATTACHMENT_COUNT>;                                                                       \
        using Views = VIEWS_T;                                                                                                                 \
        static constexpr auto const & AT = ATTACHMENTS_T{};                                                                                    \
        static constexpr auto ATTACH_COUNT = ATTACHMENT_COUNT;                                                                                 \
    };                                                                                                                                         \
    }                                                                                                                                          \
    ;

#define DAXA_TH_BLOB(HEAD_NAME, field_name) HEAD_NAME::AttachmentShaderBlob field_name;

#define DAXA_TH_IMAGE(TASK_ACCESS, VIEW_TYPE, NAME) _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, .view_type = daxa::ImageViewType::VIEW_TYPE, .shader_array_size = 0)

#define DAXA_TH_IMAGE_ID(TASK_ACCESS, VIEW_TYPE, NAME) _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, .view_type = daxa::ImageViewType::VIEW_TYPE, .shader_array_size = 1)
#define DAXA_TH_IMAGE_ID_MIP_ARRAY(TASK_ACCESS, VIEW_TYPE, NAME, SIZE) _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, .view_type = daxa::ImageViewType::VIEW_TYPE, .shader_array_size = SIZE, .shader_array_type = daxa::TaskHeadImageArrayType::MIP_LEVELS)

#define DAXA_TH_IMAGE_INDEX(TASK_ACCESS, VIEW_TYPE, NAME) _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, .view_type = daxa::ImageViewType::VIEW_TYPE, .shader_array_size = 1, .shader_as_index = true)
#define DAXA_TH_IMAGE_INDEX_MIP_ARRAY(TASK_ACCESS, VIEW_TYPE, NAME, SIZE) _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, .view_type = daxa::ImageViewType::VIEW_TYPE, .shader_array_size = SIZE, .shader_array_type = daxa::TaskHeadImageArrayType::MIP_LEVELS, .shader_as_index = true)

#define DAXA_TH_IMAGE_TYPED(TASK_ACCESS, VIEW_TYPE, NAME) _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, .view_type = VIEW_TYPE::IMAGE_VIEW_TYPE, .shader_array_size = 1, .shader_as_index = VIEW_TYPE::SHADER_INDEX32)
#define DAXA_TH_IMAGE_TYPED_MIP_ARRAY(TASK_ACCESS, VIEW_TYPE, NAME, SIZE) _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, .view_type = VIEW_TYPE::IMAGE_VIEW_TYPE, .shader_array_size = SIZE, .shader_as_index = VIEW_TYPE::SHADER_INDEX32, .shader_array_type = daxa::TaskHeadImageArrayType::MIP_LEVELS)

#define DAXA_TH_STAGE_VAR(STAGE_VAR) daxa::TaskStage stage = {};

#define DAXA_TH_BUFFER(TASK_ACCESS, NAME) _DAXA_HELPER_TH_BUFFER(NAME, TASK_ACCESS, .shader_array_size = 0)
#define DAXA_TH_BUFFER_ID(TASK_ACCESS, NAME) _DAXA_HELPER_TH_BUFFER(NAME, TASK_ACCESS, .shader_array_size = 1, .shader_as_address = false)
#define DAXA_TH_BUFFER_PTR(TASK_ACCESS, PTR_TYPE, NAME) _DAXA_HELPER_TH_BUFFER(NAME, TASK_ACCESS, .shader_array_size = 1, .shader_as_address = true)
#define DAXA_TH_BLAS(TASK_ACCESS, NAME) _DAXA_HELPER_TH_BLAS(NAME, TASK_ACCESS)
#define DAXA_TH_TLAS(TASK_ACCESS, NAME) _DAXA_HELPER_TH_TLAS(NAME, TASK_ACCESS, .shader_array_size = 0)
#define DAXA_TH_TLAS_PTR(TASK_ACCESS, NAME) _DAXA_HELPER_TH_TLAS(NAME, TASK_ACCESS, .shader_as_address = true)
#define DAXA_TH_TLAS_ID(TASK_ACCESS, NAME) _DAXA_HELPER_TH_TLAS(NAME, TASK_ACCESS, .shader_as_address = false)

    template <typename BufFn, typename ImgFn>
    constexpr void for_each(std::span<TaskAttachmentInfo> attachments, BufFn && buf_fn, ImgFn && img_fn)
    {
        for (u32 index = 0; index < attachments.size(); ++index)
        {
            switch (attachments[index].type)
            {
            case TaskAttachmentType::BUFFER: buf_fn(index, attachments[index].value.buffer); break;
            case TaskAttachmentType::BLAS: buf_fn(index, attachments[index].value.blas); break;
            case TaskAttachmentType::TLAS: buf_fn(index, attachments[index].value.tlas); break;
            case TaskAttachmentType::IMAGE: img_fn(index, attachments[index].value.image); break;
            default: break;
            }
        }
    }

    template <typename BufFn, typename ImgFn>
    constexpr void for_each(std::span<TaskAttachmentInfo const> attachments, BufFn && buf_fn, ImgFn && img_fn)
    {
        for (u32 index = 0; index < attachments.size(); ++index)
        {
            switch (attachments[index].type)
            {
            case TaskAttachmentType::BUFFER: buf_fn(index, attachments[index].value.buffer); break;
            case TaskAttachmentType::BLAS: buf_fn(index, attachments[index].value.blas); break;
            case TaskAttachmentType::TLAS: buf_fn(index, attachments[index].value.tlas); break;
            case TaskAttachmentType::IMAGE: img_fn(index, attachments[index].value.image); break;
            default: break;
            }
        }
    }

    inline auto attachment_view(TaskBufferAttachmentIndex index, TaskBufferView view) -> TaskViewVariant
    {
        return std::pair<daxa::TaskBufferAttachmentIndex, daxa::TaskBufferView>(index, view);
    }

    inline auto attachment_view(TaskBlasAttachmentIndex index, TaskBlasView view) -> TaskViewVariant
    {
        return std::pair<daxa::TaskBlasAttachmentIndex, daxa::TaskBlasView>(index, view);
    }

    inline auto attachment_view(TaskTlasAttachmentIndex index, TaskTlasView view) -> TaskViewVariant
    {
        return std::pair<daxa::TaskTlasAttachmentIndex, daxa::TaskTlasView>(index, view);
    }

    inline auto attachment_view(TaskImageAttachmentIndex index, TaskImageView view) -> TaskViewVariant
    {
        return std::pair<daxa::TaskImageAttachmentIndex, daxa::TaskImageView>(index, view);
    }

    inline auto operator|(TaskBufferAttachmentIndex index, TaskBufferView view) -> TaskViewVariant
    {
        return std::pair<daxa::TaskBufferAttachmentIndex, daxa::TaskBufferView>(index, view);
    }

    inline auto operator|(TaskBlasAttachmentIndex index, TaskBlasView view) -> TaskViewVariant
    {
        return std::pair<daxa::TaskBlasAttachmentIndex, daxa::TaskBlasView>(index, view);
    }

    inline auto operator|(TaskTlasAttachmentIndex index, TaskTlasView view) -> TaskViewVariant
    {
        return std::pair<daxa::TaskTlasAttachmentIndex, daxa::TaskTlasView>(index, view);
    }

    inline auto operator|(TaskImageAttachmentIndex index, TaskImageView view) -> TaskViewVariant
    {
        return std::pair<daxa::TaskImageAttachmentIndex, daxa::TaskImageView>(index, view);
    }

    inline auto inl_attachment(TaskAccess access, TaskBufferView view) -> TaskAttachmentInfo
    {
        TaskBufferAttachmentInfo buf = {};
        buf.name = "inline attachment";
        buf.task_access = access;
        buf.shader_array_size = 0;
        buf.shader_as_address = false;
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
        tlas.shader_as_address = false;
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