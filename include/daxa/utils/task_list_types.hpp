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

    namespace detail
    {
        template <typename T>
        struct ConstexprCompatibleSpan
        {
            std::array<u8, 16> raw = {};

            auto get() -> std::span<T> &
            {
                return *reinterpret_cast<std::span<T> *>(&raw);
            }

            auto get() const -> std::span<T> const &
            {
                return *reinterpret_cast<std::span<T> const *>(&raw);
            }
        };

        template <typename T>
        struct ConstexprCompatibleOptional
        {
            std::array<u8, sizeof(std::optional<T>)> raw = {};

            auto get() -> std::optional<T> &
            {
                return *reinterpret_cast<std::optional<T> *>(&raw);
            }

            auto get() const -> std::optional<T> const &
            {
                return *reinterpret_cast<std::optional<T> const *>(&raw);
            }
        };
    } // namespace detail

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
        ImageViewType view_type = ImageViewType::MAX_ENUM;
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

    enum class TaskInputType : u32
    {
        NONE = 0,
        BUFFER = 1,
        IMAGE = 2,
        CONSTANT = 3,
    };

    static inline constexpr size_t TASK_INPUT_FIELD_SIZE = 128;

    struct GenericTaskInput
    {
        TaskInputType type = TaskInputType::NONE;
        // This is nessecary for c++ to properly generate copy and move operators.
        [[maybe_unused]] u8 raw[TASK_INPUT_FIELD_SIZE - sizeof(TaskInputType)] = {};
    };

    struct alignas(TASK_INPUT_FIELD_SIZE) TaskBufferInput
    {
      private:
        friend struct ImplTaskList;
        TaskInputType const type = TaskInputType::BUFFER;
        static constexpr inline TaskInputType INPUT_TYPE = TaskInputType::BUFFER;
        std::span<BufferId const> buffers = {};

      public:
        TaskBufferId id = {};
        TaskBufferAccess access = {};

        constexpr TaskBufferInput() = default;

        constexpr TaskBufferInput(TaskBufferUseInit const & init)
            : id{init.id},
              access{init.access}
        {
        }

        auto buffer(usize index = 0) const -> BufferId
        {
            DAXA_DBG_ASSERT_TRUE_M(buffers.size() > 0, "this function is only allowed to be called within a task callback");
            return buffers[index];
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
        TaskInputType type = TaskInputType::IMAGE;
        static constexpr inline TaskInputType INPUT_TYPE = TaskInputType::IMAGE;
        std::span<ImageId const> images = {};
        std::span<ImageViewId const> views = {};

      public:
        TaskImageId id = {};
        TaskImageAccess access = {};
        ImageMipArraySlice slice = {};
        /// @brief  Determines the view type the runtime provides in the TaskInterface<>.
        ///         If no type is provided, the runtime images default view type is used.
        ImageViewType view_type = ImageViewType::MAX_ENUM;

        constexpr TaskImageInput() = default;

        constexpr TaskImageInput(TaskImageUseInit const & init)
            : id{init.id},
              access{init.access},
              slice{init.slice},
              view_type{init.view_type}
        {
        }

        auto image(u32 index = 0) const -> ImageId
        {
            DAXA_DBG_ASSERT_TRUE_M(images.size() > 0, "this function is only allowed to be called within a task callback");
            return images[index];
        }

        auto view(u32 index = 0) const -> ImageViewId
        {
            DAXA_DBG_ASSERT_TRUE_M(views.size() > 0, "this function is only allowed to be called within a task callback");
            return views[index];
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

    static inline constexpr size_t TASK_BUFFER_INPUT_SIZE = sizeof(TaskBufferInput);
    static inline constexpr size_t TASK_IMAGE_INPUT_SIZE = sizeof(TaskImageInput);

    static_assert(TASK_BUFFER_INPUT_SIZE == TASK_IMAGE_INPUT_SIZE, "should be impossible! contact Ipotrick");
    static_assert(TASK_BUFFER_INPUT_SIZE == TASK_INPUT_FIELD_SIZE, "should be impossible! contact Ipotrick");

    struct TaskUseOffsetType
    {
        u32 offset = {};
        TaskInputType type = {};
    };

    template <typename ReflectedT>
    struct TaskUses
    {
        using FIRST_DERIVED = ReflectedT;
        static constexpr usize USE_COUNT = []()
        {
            static_assert(sizeof(ReflectedT) != 0, "TaskUse must be non zero size");
            static_assert(sizeof(ReflectedT) % TASK_INPUT_FIELD_SIZE == 0, "TaskUse struct must only contain task uses!");
            return sizeof(ReflectedT) / TASK_INPUT_FIELD_SIZE;
        }();
    };

    struct GenericTaskArgsContainer
    {
        std::vector<u8> memory = {};
        usize count = {};

        auto span() -> std::span<GenericTaskInput>
        {
            return {reinterpret_cast<GenericTaskInput *>(memory.data()), count};
        }

        auto span() const -> std::span<GenericTaskInput const>
        {
            return {reinterpret_cast<GenericTaskInput const *>(memory.data()), count};
        }

        operator std::span<GenericTaskInput>()
        {
            return span();
        }

        operator std::span<GenericTaskInput const> const()
        {
            return span();
        }
    };
} // namespace daxa