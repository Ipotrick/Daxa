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

        static constexpr TaskAccess DEPTH_ATTACHMENT = TaskAccessConsts::DSA::READ_WRITE;
        static constexpr TaskAccess STENCIL_ATTACHMENT = TaskAccessConsts::DSA::READ_WRITE;
        static constexpr TaskAccess DEPTH_ATTACHMENT_READ = TaskAccessConsts::DSA::SAMPLED;
        static constexpr TaskAccess STENCIL_ATTACHMENT_READ = TaskAccessConsts::DSA::SAMPLED;

        static constexpr TaskAccess PRESENT = TaskAccess{TaskStage::PRESENT, TaskAccessType::READ, TaskAttachmentType::IMAGE};
        static constexpr TaskAccess INDIRECT_COMMAND_READ = TaskAccess{TaskStage::INDIRECT_COMMAND, TaskAccessType::READ, TaskAttachmentType::BUFFER};
        static constexpr TaskAccess ICR = INDIRECT_COMMAND_READ;
        static constexpr TaskAccess INDEX_INPUT_READ = TaskAccess{TaskStage::INDEX_INPUT, TaskAccessType::READ, TaskAttachmentType::BUFFER};
        static constexpr TaskAccess IDXR = INDEX_INPUT_READ;

#if !DAXA_REMOVE_DEPRECATED
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess GRAPHICS_SHADER_READ = TaskAccessConsts::RS::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess GRAPHICS_SHADER_WRITE = TaskAccessConsts::RS::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess GRAPHICS_SHADER_READ_WRITE = TaskAccessConsts::RS::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess GRAPHICS_SHADER_READ_WRITE_CONCURRENT = TaskAccessConsts::RS::READ_WRITE_CONCURRENT;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess COMPUTE_SHADER_READ = TaskAccessConsts::COMPUTE_SHADER::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess COMPUTE_SHADER_WRITE = TaskAccessConsts::COMPUTE_SHADER::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess COMPUTE_SHADER_READ_WRITE = TaskAccessConsts::COMPUTE_SHADER::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess COMPUTE_SHADER_READ_WRITE_CONCURRENT = TaskAccessConsts::COMPUTE_SHADER::READ_WRITE_CONCURRENT;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess RAY_TRACING_SHADER_READ = TaskAccessConsts::RAY_TRACING_SHADER::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess RAY_TRACING_SHADER_WRITE = TaskAccessConsts::RAY_TRACING_SHADER::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess RAY_TRACING_SHADER_READ_WRITE = TaskAccessConsts::RAY_TRACING_SHADER::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess RAY_TRACING_SHADER_READ_WRITE_CONCURRENT = TaskAccessConsts::RAY_TRACING_SHADER::READ_WRITE_CONCURRENT;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TASK_SHADER_READ = TaskAccessConsts::TASK_SHADER::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TASK_SHADER_WRITE = TaskAccessConsts::TASK_SHADER::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TASK_SHADER_READ_WRITE = TaskAccessConsts::TASK_SHADER::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TASK_SHADER_READ_WRITE_CONCURRENT = TaskAccessConsts::TASK_SHADER::READ_WRITE_CONCURRENT;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess MESH_SHADER_READ = TaskAccessConsts::MESH_SHADER::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess MESH_SHADER_WRITE = TaskAccessConsts::MESH_SHADER::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess MESH_SHADER_READ_WRITE = TaskAccessConsts::MESH_SHADER::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess MESH_SHADER_READ_WRITE_CONCURRENT = TaskAccessConsts::MESH_SHADER::READ_WRITE_CONCURRENT;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess VERTEX_SHADER_READ = TaskAccessConsts::VERTEX_SHADER::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess VERTEX_SHADER_WRITE = TaskAccessConsts::VERTEX_SHADER::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess VERTEX_SHADER_READ_WRITE = TaskAccessConsts::VERTEX_SHADER::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess VERTEX_SHADER_READ_WRITE_CONCURRENT = TaskAccessConsts::VERTEX_SHADER::READ_WRITE_CONCURRENT;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TESSELLATION_CONTROL_SHADER_READ = TaskAccessConsts::TESSELLATION_CONTROL_SHADER::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TESSELLATION_CONTROL_SHADER_WRITE = TaskAccessConsts::TESSELLATION_CONTROL_SHADER::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TESSELLATION_CONTROL_SHADER_READ_WRITE = TaskAccessConsts::TESSELLATION_CONTROL_SHADER::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TESSELLATION_CONTROL_SHADER_READ_WRITE_CONCURRENT = TaskAccessConsts::TESSELLATION_CONTROL_SHADER::READ_WRITE_CONCURRENT;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TESSELLATION_EVALUATION_SHADER_READ = TaskAccessConsts::TESSELLATION_EVALUATION_SHADER::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TESSELLATION_EVALUATION_SHADER_WRITE = TaskAccessConsts::TESSELLATION_EVALUATION_SHADER::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TESSELLATION_EVALUATION_SHADER_READ_WRITE = TaskAccessConsts::TESSELLATION_EVALUATION_SHADER::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TESSELLATION_EVALUATION_SHADER_READ_WRITE_CONCURRENT = TaskAccessConsts::TESSELLATION_EVALUATION_SHADER::READ_WRITE_CONCURRENT;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess GEOMETRY_SHADER_READ = TaskAccessConsts::GEOMETRY_SHADER::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess GEOMETRY_SHADER_WRITE = TaskAccessConsts::GEOMETRY_SHADER::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess GEOMETRY_SHADER_READ_WRITE = TaskAccessConsts::GEOMETRY_SHADER::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess GEOMETRY_SHADER_READ_WRITE_CONCURRENT = TaskAccessConsts::GEOMETRY_SHADER::READ_WRITE_CONCURRENT;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess FRAGMENT_SHADER_READ = TaskAccessConsts::FRAGMENT_SHADER::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess FRAGMENT_SHADER_WRITE = TaskAccessConsts::FRAGMENT_SHADER::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess FRAGMENT_SHADER_READ_WRITE = TaskAccessConsts::FRAGMENT_SHADER::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess FRAGMENT_SHADER_READ_WRITE_CONCURRENT = TaskAccessConsts::FRAGMENT_SHADER::READ_WRITE_CONCURRENT;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess INDEX_READ = TaskAccessConsts::INDEX_INPUT_READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess DRAW_INDIRECT_INFO_READ = TaskAccessConsts::INDIRECT_COMMAND_READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TRANSFER_READ = TaskAccessConsts::TRANSFER::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TRANSFER_WRITE = TaskAccessConsts::TRANSFER::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TRANSFER_READ_WRITE = TaskAccessConsts::TRANSFER::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess HOST_TRANSFER_READ = TaskAccessConsts::HOST::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess HOST_TRANSFER_WRITE = TaskAccessConsts::HOST::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess HOST_TRANSFER_READ_WRITE = TaskAccessConsts::HOST::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess ACCELERATION_STRUCTURE_BUILD_READ = TaskAccessConsts::ACCELERATION_STRUCTURE_BUILD::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess ACCELERATION_STRUCTURE_BUILD_WRITE = TaskAccessConsts::ACCELERATION_STRUCTURE_BUILD::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess ACCELERATION_STRUCTURE_BUILD_READ_WRITE = TaskAccessConsts::ACCELERATION_STRUCTURE_BUILD::READ_WRITE;

        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess SHADER_SAMPLED = TaskAccessConsts::SHADER::SAMPLED;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess SHADER_STORAGE_WRITE_ONLY = TaskAccessConsts::SHADER::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess SHADER_STORAGE_READ_ONLY = TaskAccessConsts::SHADER::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess SHADER_STORAGE_READ_WRITE = TaskAccessConsts::SHADER::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess SHADER_STORAGE_READ_WRITE_CONCURRENT = TaskAccessConsts::SHADER::READ_WRITE_CONCURRENT;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess GRAPHICS_SHADER_SAMPLED = TaskAccessConsts::RS::SAMPLED;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess GRAPHICS_SHADER_STORAGE_WRITE_ONLY = TaskAccessConsts::RS::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess GRAPHICS_SHADER_STORAGE_READ_ONLY = TaskAccessConsts::RS::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess GRAPHICS_SHADER_STORAGE_READ_WRITE = TaskAccessConsts::RS::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess GRAPHICS_SHADER_STORAGE_READ_WRITE_CONCURRENT = TaskAccessConsts::RS::READ_WRITE_CONCURRENT;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess COMPUTE_SHADER_SAMPLED = TaskAccessConsts::CS::SAMPLED;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess COMPUTE_SHADER_STORAGE_WRITE_ONLY = TaskAccessConsts::CS::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess COMPUTE_SHADER_STORAGE_READ_ONLY = TaskAccessConsts::CS::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess COMPUTE_SHADER_STORAGE_READ_WRITE = TaskAccessConsts::CS::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess COMPUTE_SHADER_STORAGE_READ_WRITE_CONCURRENT = TaskAccessConsts::CS::READ_WRITE_CONCURRENT;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess RAY_TRACING_SHADER_SAMPLED = TaskAccessConsts::RT::SAMPLED;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess RAY_TRACING_SHADER_STORAGE_WRITE_ONLY = TaskAccessConsts::RT::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess RAY_TRACING_SHADER_STORAGE_READ_ONLY = TaskAccessConsts::RT::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess RAY_TRACING_SHADER_STORAGE_READ_WRITE = TaskAccessConsts::RT::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess RAY_TRACING_SHADER_STORAGE_READ_WRITE_CONCURRENT = TaskAccessConsts::RT::READ_WRITE_CONCURRENT;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TASK_SHADER_SAMPLED = TaskAccessConsts::TS::SAMPLED;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TASK_SHADER_STORAGE_WRITE_ONLY = TaskAccessConsts::TS::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TASK_SHADER_STORAGE_READ_ONLY = TaskAccessConsts::TS::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TASK_SHADER_STORAGE_READ_WRITE = TaskAccessConsts::TS::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TASK_SHADER_STORAGE_READ_WRITE_CONCURRENT = TaskAccessConsts::TS::READ_WRITE_CONCURRENT;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess MESH_SHADER_SAMPLED = TaskAccessConsts::MS::SAMPLED;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess MESH_SHADER_STORAGE_WRITE_ONLY = TaskAccessConsts::MS::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess MESH_SHADER_STORAGE_READ_ONLY = TaskAccessConsts::MS::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess MESH_SHADER_STORAGE_READ_WRITE = TaskAccessConsts::MS::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess MESH_SHADER_STORAGE_READ_WRITE_CONCURRENT = TaskAccessConsts::MS::READ_WRITE_CONCURRENT;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess VERTEX_SHADER_SAMPLED = TaskAccessConsts::VS::SAMPLED;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess VERTEX_SHADER_STORAGE_WRITE_ONLY = TaskAccessConsts::VS::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess VERTEX_SHADER_STORAGE_READ_ONLY = TaskAccessConsts::VS::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess VERTEX_SHADER_STORAGE_READ_WRITE = TaskAccessConsts::VS::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess VERTEX_SHADER_STORAGE_READ_WRITE_CONCURRENT = TaskAccessConsts::VS::READ_WRITE_CONCURRENT;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TESSELLATION_CONTROL_SHADER_SAMPLED = TaskAccessConsts::TCS::SAMPLED;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TESSELLATION_CONTROL_SHADER_STORAGE_WRITE_ONLY = TaskAccessConsts::TCS::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TESSELLATION_CONTROL_SHADER_STORAGE_READ_ONLY = TaskAccessConsts::TCS::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TESSELLATION_CONTROL_SHADER_STORAGE_READ_WRITE = TaskAccessConsts::TCS::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TESSELLATION_CONTROL_SHADER_STORAGE_READ_WRITE_CONCURRENT = TaskAccessConsts::TCS::READ_WRITE_CONCURRENT;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TESSELLATION_EVALUATION_SHADER_SAMPLED = TaskAccessConsts::TES::SAMPLED;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TESSELLATION_EVALUATION_SHADER_STORAGE_WRITE_ONLY = TaskAccessConsts::TES::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TESSELLATION_EVALUATION_SHADER_STORAGE_READ_ONLY = TaskAccessConsts::TES::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TESSELLATION_EVALUATION_SHADER_STORAGE_READ_WRITE = TaskAccessConsts::TES::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess TESSELLATION_EVALUATION_SHADER_STORAGE_READ_WRITE_CONCURRENT = TaskAccessConsts::TES::READ_WRITE_CONCURRENT;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess GEOMETRY_SHADER_SAMPLED = TaskAccessConsts::GS::SAMPLED;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess GEOMETRY_SHADER_STORAGE_WRITE_ONLY = TaskAccessConsts::GS::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess GEOMETRY_SHADER_STORAGE_READ_ONLY = TaskAccessConsts::GS::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess GEOMETRY_SHADER_STORAGE_READ_WRITE = TaskAccessConsts::GS::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess GEOMETRY_SHADER_STORAGE_READ_WRITE_CONCURRENT = TaskAccessConsts::GS::READ_WRITE_CONCURRENT;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess FRAGMENT_SHADER_SAMPLED = TaskAccessConsts::FS::SAMPLED;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess FRAGMENT_SHADER_STORAGE_WRITE_ONLY = TaskAccessConsts::FS::WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess FRAGMENT_SHADER_STORAGE_READ_ONLY = TaskAccessConsts::FS::READ;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess FRAGMENT_SHADER_STORAGE_READ_WRITE = TaskAccessConsts::FS::READ_WRITE;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess FRAGMENT_SHADER_STORAGE_READ_WRITE_CONCURRENT = TaskAccessConsts::FS::READ_WRITE_CONCURRENT;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess DEPTH_STENCIL_ATTACHMENT_READ = TaskAccessConsts::DSA::SAMPLED;
        [[deprecated("Use new TaskAccessConsts instead, API:3.1")]] static constexpr TaskAccess RESOLVE_WRITE = TaskAccessConsts::RESOLVE::READ_WRITE;
#endif // #if !DAXA_REMOVE_DEPRECATED
    };

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

    DAXA_EXPORT_CXX auto task_type_default_stage(TaskType task_type) -> TaskStage;

    using TaskResourceIndex = u32;

    struct DAXA_EXPORT_CXX TaskGPUResourceView
    {
        TaskResourceIndex task_graph_index = {};
        TaskResourceIndex index = {};

        auto is_empty() const -> bool;
        auto is_external() const -> bool;
        auto is_null() const -> bool;

        auto operator<=>(TaskGPUResourceView const & other) const = default;
    };

    [[nodiscard]] DAXA_EXPORT_CXX auto to_string(TaskGPUResourceView const & id) -> std::string;

    struct TaskBufferView : public TaskGPUResourceView
    {
        using ID_T = BufferId;
    };

    struct TaskBlasView : public TaskGPUResourceView
    {
        using ID_T = BlasId;
    };

    struct TaskTlasView : public TaskGPUResourceView
    {
        using ID_T = TlasId;
    };

    struct TaskAttachmentInfo;

    struct TaskImageView
    {
        TaskResourceIndex task_graph_index = {};
        TaskResourceIndex index = {};
        ImageMipArraySlice slice = {};
#if !DAXA_REMOVE_DEPRECATED
        [[deprecated("Use .mips and .layers instead, API:3.1")]] auto view(daxa::ImageMipArraySlice const & new_slice) const -> TaskImageView
        {
            auto ret = *this;
            ret.slice = new_slice;
            return ret;
        }
#endif // #if !DAXA_REMOVE_DEPRECATED
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

        operator TaskGPUResourceView &()
        {
            return *reinterpret_cast<TaskGPUResourceView *>(this);
        }
        operator TaskGPUResourceView const &() const
        {
            return *reinterpret_cast<TaskGPUResourceView const *>(this);
        }

        auto is_empty() const -> bool { return operator TaskGPUResourceView const &().is_empty(); }
        auto is_external() const -> bool { return operator TaskGPUResourceView const &().is_external(); }
        auto is_null() const -> bool { return operator TaskGPUResourceView const &().is_null(); }
    };

    struct Test
    {
        TaskResourceIndex task_graph_index = {};
        TaskResourceIndex index = {};
        ImageMipArraySlice slice = {};
    };

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

    struct TaskBufferAttachmentInfo
    {
        using INDEX_TYPE = TaskBufferAttachmentIndex;
        static constexpr TaskAttachmentType ATTACHMENT_TYPE = TaskAttachmentType::BUFFER;
        char const * name = {};
        TaskAccess task_access = {};
        Access access = {};
        u8 shader_array_size = {};
        bool shader_as_address = {};

        TaskBufferView view = {};
        TaskBufferView translated_view = {};
        std::span<BufferId const> ids = {};
    };

    static_assert(std::is_standard_layout_v<TaskBufferAttachmentInfo>);

    struct TaskBlasAttachmentInfo
    {
        using INDEX_TYPE = TaskBlasAttachmentIndex;
        static constexpr TaskAttachmentType ATTACHMENT_TYPE = TaskAttachmentType::BLAS;
        char const * name = {};
        TaskAccess task_access = {};
        Access access = {};

        TaskBlasView view = {};
        TaskBlasView translated_view = {};
        std::span<BlasId const> ids = {};
    };

    static_assert(std::is_standard_layout_v<TaskBlasAttachmentInfo>);

    struct TaskTlasAttachmentInfo
    {
        using INDEX_TYPE = TaskTlasAttachmentIndex;
        static constexpr TaskAttachmentType ATTACHMENT_TYPE = TaskAttachmentType::TLAS;
        char const * name = {};
        TaskAccess task_access = {};
        Access access = {};
        bool shader_as_address = {};

        TaskTlasView view = {};
        TaskTlasView translated_view = {};
        std::span<TlasId const> ids = {};
    };

    static_assert(std::is_standard_layout_v<TaskTlasAttachmentInfo>);

    struct TaskImageAttachmentInfo
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

        TaskImageView view = {};
        TaskImageView translated_view = {};
        [[deprecated("Parameter Ignored, always layout general; API:3.2")]] ImageLayout layout = {};
        std::span<ImageId const> ids = {};
        std::span<ImageViewId const> view_ids = {};
    };

    static_assert(std::is_standard_layout_v<TaskImageAttachmentInfo>);

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

        [[deprecated("Layout is guaranteed to always be general, stop using this function; API:3.2")]] 
        auto layout(TaskImageIndexOrView auto) const -> ImageLayout
        {
            return daxa::ImageLayout::GENERAL;
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
            auto const v = this->get(timg).view_ids[index];
            DAXA_DBG_ASSERT_TRUE_M(
                !v.is_empty(), 
                "Failed to return cached image view for image attachment!\n"
                "A likely cause for this error is that no daxa::ImageViewType was specified for the attachment.\n"
                "To specify an image view type for a task attachment you can either:\n"
                "1. add the view type to the attachment within a task head as the second parameter: DAXA_TG_IMAGE(COLOR_ATTACHMENT, REGULAR_2D, image_name), OR\n"
                "2. add the view type when adding the attachment to the task: task.color_attachment.reads_writes(daxa::ImageViewType::REGULAR_2D, image)"
            );
            return v;
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
        TaskResourceViewOrResourceOrImageViewType<T> || std::is_same_v<T, TaskStage>;

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
            constexpr auto at(daxa::usize idx) const -> daxa::TaskAttachment const &             \
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
            daxa::TaskBufferAttachment{                                        \
                .name = #NAME,                                                 \
                .task_access = daxa::TaskAccessConsts::TASK_ACCESS,            \
                __VA_ARGS__})};

#define _DAXA_HELPER_TH_BLAS(NAME, TASK_ACCESS)                              \
    typename TDecl::TaskBlasT const NAME =                                   \
        {TDecl::template process_attachment_decl<typename TDecl::TaskBlasT>( \
            _internal,                                                       \
            daxa::TaskBlasAttachment{                                        \
                .name = #NAME,                                               \
                .task_access = daxa::TaskAccessConsts::TASK_ACCESS,          \
            })};

#define _DAXA_HELPER_TH_TLAS(NAME, TASK_ACCESS, ...)                         \
    typename TDecl::TaskTlasT const NAME =                                   \
        {TDecl::template process_attachment_decl<typename TDecl::TaskTlasT>( \
            _internal,                                                       \
            daxa::TaskTlasAttachment{                                        \
                .name = #NAME,                                               \
                .task_access = daxa::TaskAccessConsts::TASK_ACCESS,          \
                __VA_ARGS__})};

#define _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, ...)                         \
    typename TDecl::TaskImageT const NAME =                                   \
        {TDecl::template process_attachment_decl<typename TDecl::TaskImageT>( \
            _internal,                                                        \
            daxa::TaskImageAttachment{                                        \
                .name = #NAME,                                                \
                .task_access = daxa::TaskAccessConsts::TASK_ACCESS,           \
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
    /*deprecated("Use Head::Info instead; API:3.1")*/ using Views = Info::Views;                                                                        \
    /*deprecated("Use Head::Info instead; API:3.1")*/ using AttachmentViews = Info::Views;                                                              \
    /*deprecated("Use Head::Info instead; API:3.1")*/ using AttachmentShaderBlob = Info::AttachmentShaderBlob;                                          \
    [[deprecated("Use Head::Info instead; API:3.1")]] static inline constexpr daxa::TaskType TYPE = Info::TYPE;                                         \
    [[deprecated("Use Head::Info instead; API:3.1")]] static inline constexpr char const * NAME = Info::NAME;                                           \
    [[deprecated("Use Head::Info instead; API:3.1")]] static constexpr decltype(Info::ATTACHMENT_COUNT) ATTACHMENT_COUNT = Info::ATTACHMENT_COUNT;      \
    [[deprecated("Use Head::Info instead; API:3.1")]] static constexpr auto const & ATTACHMENTS = Info::ATTACHMENTS;                                    \
    [[deprecated("Use Head::Info instead; API:3.1")]] static constexpr auto const & AT = Info::ATTACHMENTS;                                             \
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

#if !DAXA_REMOVE_DEPRECATED
    [[deprecated("Task::Views asignment instead, API:3.1")]] inline auto attachment_view(TaskBufferAttachmentIndex index, TaskBufferView view) -> TaskViewIndexVariant
    {
        return std::pair<daxa::TaskBufferAttachmentIndex, daxa::TaskBufferView>(index, view);
    }

    [[deprecated("Task::Views asignment instead, API:3.1")]] inline auto attachment_view(TaskBlasAttachmentIndex index, TaskBlasView view) -> TaskViewIndexVariant
    {
        return std::pair<daxa::TaskBlasAttachmentIndex, daxa::TaskBlasView>(index, view);
    }

    [[deprecated("Task::Views asignment instead, API:3.1")]] inline auto attachment_view(TaskTlasAttachmentIndex index, TaskTlasView view) -> TaskViewIndexVariant
    {
        return std::pair<daxa::TaskTlasAttachmentIndex, daxa::TaskTlasView>(index, view);
    }

    [[deprecated("Task::Views asignment instead, API:3.1")]] inline auto attachment_view(TaskImageAttachmentIndex index, TaskImageView view) -> TaskViewIndexVariant
    {
        return std::pair<daxa::TaskImageAttachmentIndex, daxa::TaskImageView>(index, view);
    }

    [[deprecated("Task::Views asignment instead, API:3.1")]] inline auto operator|(TaskBufferAttachmentIndex index, TaskBufferView view) -> TaskViewIndexVariant
    {
        return std::pair<daxa::TaskBufferAttachmentIndex, daxa::TaskBufferView>(index, view);
    }

    [[deprecated("Task::Views asignment instead, API:3.1")]] inline auto operator|(TaskBlasAttachmentIndex index, TaskBlasView view) -> TaskViewIndexVariant
    {
        return std::pair<daxa::TaskBlasAttachmentIndex, daxa::TaskBlasView>(index, view);
    }

    [[deprecated("Task::Views asignment instead, API:3.1")]] inline auto operator|(TaskTlasAttachmentIndex index, TaskTlasView view) -> TaskViewIndexVariant
    {
        return std::pair<daxa::TaskTlasAttachmentIndex, daxa::TaskTlasView>(index, view);
    }

    [[deprecated("Task::Views asignment instead, API:3.1")]] inline auto operator|(TaskImageAttachmentIndex index, TaskImageView view) -> TaskViewIndexVariant
    {
        return std::pair<daxa::TaskImageAttachmentIndex, daxa::TaskImageView>(index, view);
    }

#endif // !DAXA_REMOVE_DEPRECATED

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