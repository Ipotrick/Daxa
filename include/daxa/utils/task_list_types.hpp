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

    auto to_string(TaskGPUResourceId const & id) -> std::string;

    struct TaskBufferId : public TaskGPUResourceId
    {
    };

    struct TaskImageId : public TaskGPUResourceId
    {
    };

    struct TaskBufferUseInit
    {
        TaskBufferId id = {};
        TaskBufferAccess access = {};
        // Redirects in callback and shader use aliases to this use.
        std::string_view name = {};
    };

    struct TaskImageUseInit
    {
        TaskImageId id = {};
        TaskImageAccess access = {};
        ImageMipArraySlice slice = {};
        /// @brief  Determines the view type the runtime provides in the GenericTaskInterface.
        ///         If no type is provided, the runtime images default view type is used.
        std::optional<ImageViewType> view_type = {};
        // Redirects in callback and shader use aliases to this use.
        std::string_view name = {};
    };

    struct ImageSliceState
    {
        Access latest_access = {};
        ImageLayout latest_layout = {};
        ImageMipArraySlice slice = {};
    };

    using UsedTaskBuffers = std::vector<TaskBufferUseInit>;
    using UsedTaskImages = std::vector<TaskImageUseInit>;

    enum class TaskInputType : u64
    {
        NONE = 0,
        BUFFER = 1,
        IMAGE = 2,
        CONSTANT = 3,
    };

    static inline constexpr usize TASK_INPUT_FIELD_SIZE = 128;

    struct alignas(TASK_INPUT_FIELD_SIZE) TaskInputBuffer
    {
      private:
        [[maybe_unused]] volatile TaskInputType const type = TaskInputType::BUFFER;

      public:
        std::span<BufferId const> m_buffers = {};
        TaskBufferId id = {};
        TaskBufferAccess access = {};

        TaskInputBuffer() = default;
        TaskInputBuffer(TaskBufferUseInit const & init)
            : id{init.id},
              access{init.access}
        {
        }

        auto buffer(usize index = 0) const -> BufferId
        {
            return m_buffers[index];
        }

        auto buffers() const -> std::span<BufferId const>
        {
            return m_buffers;
        }
    };

    struct alignas(TASK_INPUT_FIELD_SIZE) TaskInputImage
    {
      private:
        [[maybe_unused]] volatile TaskInputType const type = TaskInputType::IMAGE;

      public:
        std::span<ImageId const> m_images = {};
        std::span<ImageViewId const> m_views = {};
        TaskImageId id = {};
        TaskImageAccess access = {};
        ImageMipArraySlice slice = {};
        /// @brief  Determines the view type the runtime provides in the GenericTaskInterface.
        ///         If no type is provided, the runtime images default view type is used.
        std::optional<ImageViewType> view_type = {};

        TaskInputImage() = default;

        TaskInputImage(TaskImageUseInit const & init)
            : id{init.id},
              access{init.access},
              slice{init.slice},
              view_type{init.view_type}
        {
        }

        auto image(u32 index = 0) const -> ImageId
        {
            return m_images[index];
        }

        auto images() const -> std::span<ImageId const>
        {
            return m_images;
        }

        auto view(u32 index = 0) const -> ImageViewId
        {
            return m_views[index];
        }

        auto views() const -> std::span<ImageViewId const>
        {
            return m_views;
        }
    };

    template <typename T>
    struct alignas(TASK_INPUT_FIELD_SIZE) TaskParam
    {
        static_assert(sizeof(T) <= (TASK_INPUT_FIELD_SIZE - sizeof(TaskInputType)), "Constant MUST be smaller then 120 bytes!");

        [[maybe_unused]] volatile TaskInputType const type = TaskInputType::CONSTANT;
        T value = {};

        TaskParam() = default;

        template <typename ANY>
            requires(std::is_convertible_v<ANY, T>)
        TaskParam(ANY const & v) : value{static_cast<T>(v)}
        {
        }

        template <typename ANY>
            requires(std::is_convertible_v<ANY, T>)
        TaskParam & operator=(ANY const & v)
        {
            value = static_cast<T>(v);
            return *this;
        }

        operator T &()
        {
            return value;
        }

        operator T const &() const
        {
            return value;
        }

        auto operator&() -> T *
        {
            return &value;
        }

        auto operator&() const -> T const *
        {
            return &value;
        }
    };

    static_assert(sizeof(TaskInputBuffer) <= TASK_INPUT_FIELD_SIZE, "should be impossible! contact Ipotrick");
    static_assert(sizeof(TaskInputImage) <= TASK_INPUT_FIELD_SIZE, "should be impossible! contact Ipotrick");

    struct alignas(TASK_INPUT_FIELD_SIZE) GenericTaskInput
    {
        TaskInputType type = TaskInputType::NONE;

        auto as_buffer_use() -> TaskInputBuffer &
        {
            return *reinterpret_cast<TaskInputBuffer *>(this);
        }

        auto as_image_use() -> TaskInputImage &
        {
            return *reinterpret_cast<TaskInputImage *>(this);
        }

        auto as_buffer_use() const -> TaskInputBuffer const &
        {
            return *reinterpret_cast<TaskInputBuffer const *>(this);
        }

        auto as_image_use() const -> TaskInputImage const &
        {
            return *reinterpret_cast<TaskInputImage const *>(this);
        }
    };

    template <typename T>
    struct TaskInputListInfo
    {
        static inline constexpr usize ELEMENT_COUNT = {sizeof(T) / TASK_INPUT_FIELD_SIZE};
        static inline constexpr usize REST = {sizeof(T) % TASK_INPUT_FIELD_SIZE};
        static_assert(REST == 0, "Task Input Struct contains invalid members!");
    };
} // namespace daxa