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
        /// @brief  Determines the view type the runtime provides in the TaskInterface<>.
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

    static inline constexpr size_t TASK_INPUT_FIELD_SIZE = 128;

    struct alignas(TASK_INPUT_FIELD_SIZE) GenericTaskInput
    {
        TaskInputType type = TaskInputType::NONE;
        // This is nessecary for c++ to properly generate copy and move operators.
        [[maybe_unused]] u8 raw[TASK_INPUT_FIELD_SIZE - sizeof(TaskInputType)] = {};
    };

    struct alignas(TASK_INPUT_FIELD_SIZE) TaskBufferInput
    {
      private:
        friend struct ImplTaskList;
        [[maybe_unused]] volatile TaskInputType const type = TaskInputType::BUFFER;
        [[maybe_unused]] static constexpr inline TaskInputType INPUT_TYPE = TaskInputType::BUFFER;
        std::span<BufferId const> m_buffers = {};

      public:
        TaskBufferId id = {};
        TaskBufferAccess access = {};

        TaskBufferInput() = default;
        TaskBufferInput(TaskBufferUseInit const & init)
            : id{init.id},
              access{init.access}
        {
        }

        auto buffer(usize index = 0) const -> BufferId
        {
            return m_buffers[index];
        }

        static auto from(GenericTaskInput const & input) -> TaskBufferInput const &
        {
            return *reinterpret_cast<TaskBufferInput const *>(&input);
        }

        static auto from(GenericTaskInput & input) -> TaskBufferInput &
        {
            return *reinterpret_cast<TaskBufferInput *>(&input);
        }

        auto to_generic() const -> GenericTaskInput const &
        {
            return *reinterpret_cast<GenericTaskInput const *>(this);
        }
    };

    struct alignas(TASK_INPUT_FIELD_SIZE) TaskImageInput
    {
      private:
        friend struct ImplTaskList;
        [[maybe_unused]] volatile TaskInputType const type = TaskInputType::IMAGE;
        [[maybe_unused]] static constexpr inline TaskInputType INPUT_TYPE = TaskInputType::IMAGE;
        std::span<ImageId const> m_images = {};
        std::span<ImageViewId const> m_views = {};

      public:
        TaskImageId id = {};
        TaskImageAccess access = {};
        ImageMipArraySlice slice = {};
        /// @brief  Determines the view type the runtime provides in the TaskInterface<>.
        ///         If no type is provided, the runtime images default view type is used.
        std::optional<ImageViewType> view_type = {};

        TaskImageInput() = default;

        TaskImageInput(TaskImageUseInit const & init)
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

        auto view(u32 index = 0) const -> ImageViewId
        {
            return m_views[index];
        }

        static auto from(GenericTaskInput const & input) -> TaskImageInput const &
        {
            return *reinterpret_cast<TaskImageInput const *>(&input);
        }

        static auto from(GenericTaskInput & input) -> TaskImageInput &
        {
            return *reinterpret_cast<TaskImageInput *>(&input);
        }

        auto to_generic() const -> GenericTaskInput const &
        {
            return *reinterpret_cast<GenericTaskInput const *>(this);
        }

        operator GenericTaskInput const &() const
        {
            return to_generic();
        }
    };

    template <typename T>
    struct alignas(TASK_INPUT_FIELD_SIZE) TaskParamInput
    {
      private:
        friend struct ImplTaskList;
        static_assert(sizeof(T) <= (TASK_INPUT_FIELD_SIZE - sizeof(TaskInputType)), "Constant MUST be smaller then 120 bytes!");

        [[maybe_unused]] volatile TaskInputType const type = TaskInputType::CONSTANT;
        [[maybe_unused]] static constexpr inline TaskInputType INPUT_TYPE = TaskInputType::CONSTANT;

      public:
        T value = {};

        TaskParamInput() = default;

        template <typename ANY>
            requires(std::is_convertible_v<ANY, T>)
        TaskParamInput(ANY const & v) : value{static_cast<T>(v)}
        {
        }

        template <typename ANY>
            requires(std::is_convertible_v<ANY, T>)
        TaskParamInput & operator=(ANY const & v)
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

        static auto from(GenericTaskInput const & input) -> TaskParamInput<T> const &
        {
            return *reinterpret_cast<TaskParamInput<T> const *>(&input);
        }

        static auto from(GenericTaskInput & input) -> TaskParamInput<T> &
        {
            return *reinterpret_cast<TaskParamInput<T> *>(&input);
        }

        auto to_generic() const -> GenericTaskInput const &
        {
            return *reinterpret_cast<GenericTaskInput const *>(this);
        }
    };

    static inline constexpr size_t TASK_BUFFER_INPUT_SIZE = sizeof(TaskBufferInput);
    static inline constexpr size_t TASK_IMAGE_INPUT_SIZE = sizeof(TaskImageInput);

    static_assert(TASK_BUFFER_INPUT_SIZE <= TASK_INPUT_FIELD_SIZE, "should be impossible! contact Ipotrick");
    static_assert(TASK_IMAGE_INPUT_SIZE <= TASK_INPUT_FIELD_SIZE, "should be impossible! contact Ipotrick");

    template <typename T>
    struct TaskInputListInfo
    {
        static inline constexpr usize ELEMENT_COUNT = {sizeof(T) / TASK_INPUT_FIELD_SIZE};
        static inline constexpr usize REST = {sizeof(T) % TASK_INPUT_FIELD_SIZE};
        static_assert(REST == 0, "Task Input Struct contains invalid members!");
    };
} // namespace daxa