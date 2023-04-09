#pragma once

#if !DAXA_BUILT_WITH_UTILS_TASK_LIST
#error "[package management error] You must build Daxa with the DAXA_ENABLE_UTILS_TASK_LIST CMake option enabled, or request the utils-task-list feature in vcpkg"
#endif

#include <span>
#include <format>

#include <daxa/core.hpp>
#include <daxa/device.hpp>

namespace daxa
{
    enum struct TaskBufferAccess
    {
        NONE,
        SHADER_READ_ONLY,
        VERTEX_SHADER_READ_ONLY,
        TESSELLATION_CONTROL_SHADER_READ_ONLY,
        TESSELLATION_EVALUATION_SHADER_READ_ONLY,
        GEOMETRY_SHADER_READ_ONLY,
        FRAGMENT_SHADER_READ_ONLY,
        COMPUTE_SHADER_READ_ONLY,
        SHADER_WRITE_ONLY,
        VERTEX_SHADER_WRITE_ONLY,
        TESSELLATION_CONTROL_SHADER_WRITE_ONLY,
        TESSELLATION_EVALUATION_SHADER_WRITE_ONLY,
        GEOMETRY_SHADER_WRITE_ONLY,
        FRAGMENT_SHADER_WRITE_ONLY,
        COMPUTE_SHADER_WRITE_ONLY,
        SHADER_READ_WRITE,
        VERTEX_SHADER_READ_WRITE,
        TESSELLATION_CONTROL_SHADER_READ_WRITE,
        TESSELLATION_EVALUATION_SHADER_READ_WRITE,
        GEOMETRY_SHADER_READ_WRITE,
        FRAGMENT_SHADER_READ_WRITE,
        COMPUTE_SHADER_READ_WRITE,
        INDEX_READ,
        DRAW_INDIRECT_INFO_READ,
        TRANSFER_READ,
        TRANSFER_WRITE,
        HOST_TRANSFER_READ,
        HOST_TRANSFER_WRITE,
    };

    auto to_string(TaskBufferAccess const & usage) -> std::string_view;

    enum struct TaskImageAccess
    {
        NONE,
        SHADER_READ_ONLY,
        VERTEX_SHADER_READ_ONLY,
        TESSELLATION_CONTROL_SHADER_READ_ONLY,
        TESSELLATION_EVALUATION_SHADER_READ_ONLY,
        GEOMETRY_SHADER_READ_ONLY,
        FRAGMENT_SHADER_READ_ONLY,
        COMPUTE_SHADER_READ_ONLY,
        SHADER_WRITE_ONLY,
        VERTEX_SHADER_WRITE_ONLY,
        TESSELLATION_CONTROL_SHADER_WRITE_ONLY,
        TESSELLATION_EVALUATION_SHADER_WRITE_ONLY,
        GEOMETRY_SHADER_WRITE_ONLY,
        FRAGMENT_SHADER_WRITE_ONLY,
        COMPUTE_SHADER_WRITE_ONLY,
        SHADER_READ_WRITE,
        VERTEX_SHADER_READ_WRITE,
        TESSELLATION_CONTROL_SHADER_READ_WRITE,
        TESSELLATION_EVALUATION_SHADER_READ_WRITE,
        GEOMETRY_SHADER_READ_WRITE,
        FRAGMENT_SHADER_READ_WRITE,
        COMPUTE_SHADER_READ_WRITE,
        TRANSFER_READ,
        TRANSFER_WRITE,
        COLOR_ATTACHMENT,
        DEPTH_ATTACHMENT,
        STENCIL_ATTACHMENT,
        DEPTH_STENCIL_ATTACHMENT,
        DEPTH_ATTACHMENT_READ_ONLY,
        STENCIL_ATTACHMENT_READ_ONLY,
        DEPTH_STENCIL_ATTACHMENT_READ_ONLY,
        RESOLVE_WRITE,
        PRESENT,
    };

    auto to_string(TaskImageAccess const & usage) -> std::string_view;

    using TaskResourceIndex = u32;
    
    struct TaskGPUResourceId
    {
        TaskResourceIndex task_list_index = {};
        TaskResourceIndex index = {};

        auto is_empty() const -> bool;
        auto is_persistent() const -> bool;

        auto operator<=>(TaskGPUResourceId const & other) const = default;
    };

    struct TaskBufferId : public TaskGPUResourceId
    {
    };

    struct TaskImageId : public TaskGPUResourceId
    {
    };

    struct TaskBufferUse
    {
        TaskBufferId id = {};
        TaskBufferAccess access = {};
        // Redirects in callback and shader use aliases to this use.
        std::string_view alias = {};
    };

    struct TaskImageUse
    {
        TaskImageId id = {};
        TaskImageAccess access = {};
        ImageMipArraySlice slice = {};
        /// @brief  Determines the view type the runtime provides in the TaskRuntimeInterface.
        ///         If no type is provided, the runtime images default view type is used.
        std::optional<ImageViewType> view_type = {};
        // Redirects in callback and shader use aliases to this use.
        std::string_view alias = {};
    };

    struct ImageSliceState
    {
        Access latest_access = {};
        ImageLayout latest_layout = {};
        ImageMipArraySlice slice = {};
    };

    using UsedTaskBuffers = std::vector<TaskBufferUse>;
    using UsedTaskImages = std::vector<TaskImageUse>;
}