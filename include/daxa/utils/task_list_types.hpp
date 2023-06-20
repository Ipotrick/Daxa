#pragma once

// Disable msvc warning on alignment padding.
#pragma warning(disable : 4324)

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
        SHADER_READ,
        VERTEX_SHADER_READ,
        TESSELLATION_CONTROL_SHADER_READ,
        TESSELLATION_EVALUATION_SHADER_READ,
        GEOMETRY_SHADER_READ,
        FRAGMENT_SHADER_READ,
        COMPUTE_SHADER_READ,
        SHADER_WRITE,
        VERTEX_SHADER_WRITE,
        TESSELLATION_CONTROL_SHADER_WRITE,
        TESSELLATION_EVALUATION_SHADER_WRITE,
        GEOMETRY_SHADER_WRITE,
        FRAGMENT_SHADER_WRITE,
        COMPUTE_SHADER_WRITE,
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
        MAX_ENUM = 0x7fffffff,
    };

    auto to_string(TaskBufferAccess const & usage) -> std::string_view;

    enum struct TaskImageAccess
    {
        NONE,
        SHADER_READ,
        VERTEX_SHADER_READ,
        TESSELLATION_CONTROL_SHADER_READ,
        TESSELLATION_EVALUATION_SHADER_READ,
        GEOMETRY_SHADER_READ,
        FRAGMENT_SHADER_READ,
        COMPUTE_SHADER_READ,
        SHADER_WRITE,
        VERTEX_SHADER_WRITE,
        TESSELLATION_CONTROL_SHADER_WRITE,
        TESSELLATION_EVALUATION_SHADER_WRITE,
        GEOMETRY_SHADER_WRITE,
        FRAGMENT_SHADER_WRITE,
        COMPUTE_SHADER_WRITE,
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
        DEPTH_ATTACHMENT_READ,
        STENCIL_ATTACHMENT_READ,
        DEPTH_STENCIL_ATTACHMENT_READ,
        RESOLVE_WRITE,
        PRESENT,
        MAX_ENUM = 0x7fffffff,
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
    } // namespace detail

    using TaskResourceIndex = u32;

    struct TaskGPUResourceHandle
    {
        TaskResourceIndex task_list_index = {};
        TaskResourceIndex index = {};

        auto is_empty() const -> bool;
        auto is_persistent() const -> bool;

        auto operator<=>(TaskGPUResourceHandle const & other) const = default;
    };

    auto to_string(TaskGPUResourceHandle const & id) -> std::string;

    struct TaskBufferHandle : public TaskGPUResourceHandle
    {
    };

    struct TaskImageHandle : public TaskGPUResourceHandle
    {
        daxa::ImageMipArraySlice slice = {};
        auto subslice(daxa::ImageMipArraySlice const & new_slice) const -> TaskImageHandle
        {
            auto ret = *this;
            ret.slice = new_slice;
            return ret;
        }
    };

    struct ImageSliceState
    {
        Access latest_access = {};
        ImageLayout latest_layout = {};
        ImageMipArraySlice slice = {};
    };

    enum struct TaskResourceUseType : u32
    {
        NONE = 0,
        BUFFER = 1,
        IMAGE = 2,
        CONSTANT = 3,
        MAX_ENUM = 0xffffffff,
    };

    static inline constexpr size_t TASK_INPUT_FIELD_SIZE = 128;

    struct GenericTaskResourceUse
    {
        TaskResourceUseType type = TaskResourceUseType::NONE;
        // This is necessary for c++ to properly generate copy and move operators.
        [[maybe_unused]] u8 raw[TASK_INPUT_FIELD_SIZE - sizeof(TaskResourceUseType)] = {};
    };

    template <TaskBufferAccess T_ACCESS = TaskBufferAccess::NONE>
    struct alignas(TASK_INPUT_FIELD_SIZE) TaskBufferUse
    {
      private:
        friend struct ImplTaskList;
        TaskResourceUseType const type = TaskResourceUseType::BUFFER;
        std::span<BufferId const> buffers = {};
        TaskBufferAccess m_access = T_ACCESS;

      public:
        TaskBufferHandle handle = {};

        constexpr TaskBufferUse() = default;

        constexpr TaskBufferUse(TaskBufferHandle const & a_handle)
            : handle{a_handle}
        {
        }

        constexpr TaskBufferUse(TaskBufferHandle const & a_handle, TaskBufferAccess access)
            requires(T_ACCESS == TaskBufferAccess::NONE)
            : handle{a_handle}, m_access{access}
        {
        }

        static auto from(GenericTaskResourceUse const & input) -> TaskBufferUse<> const &
        {
            DAXA_DBG_ASSERT_TRUE_M(input.type == TaskResourceUseType::BUFFER, "invalid TaskResourceUse cast");
            return *reinterpret_cast<TaskBufferUse<> const *>(&input);
        }

        static auto from(GenericTaskResourceUse & input) -> TaskBufferUse<> &
        {
            DAXA_DBG_ASSERT_TRUE_M(input.type == TaskResourceUseType::BUFFER, "invalid TaskResourceUse cast");
            return *reinterpret_cast<TaskBufferUse<> *>(&input);
        }

        auto access() const -> TaskBufferAccess
        {
            return m_access;
        }

        auto buffer(usize index = 0) const -> BufferId
        {
            DAXA_DBG_ASSERT_TRUE_M(buffers.size() > 0, "this function is only allowed to be called within a task callback");
            return buffers[index];
        }

        auto to_generic() const -> GenericTaskResourceUse const &
        {
            return *reinterpret_cast<GenericTaskResourceUse const *>(this);
        }

        operator GenericTaskResourceUse const &() const
        {
            return to_generic();
        }
    };

    template <TaskImageAccess T_ACCESS = TaskImageAccess::NONE, ImageViewType T_VIEW_TYPE = ImageViewType::MAX_ENUM>
    struct alignas(TASK_INPUT_FIELD_SIZE) TaskImageUse
    {
      private:
        friend struct ImplTaskList;
        TaskResourceUseType type = TaskResourceUseType::IMAGE;
        TaskImageAccess m_access = T_ACCESS;
        ImageViewType m_view_type = T_VIEW_TYPE;
        std::span<ImageId const> images = {};
        std::span<ImageViewId const> views = {};

      public:
        TaskImageHandle handle = {};

        constexpr TaskImageUse() = default;

        constexpr TaskImageUse(TaskImageHandle const & a_handle)
            : handle{a_handle}
        {
        }

        constexpr TaskImageUse(TaskImageHandle const & a_handle, TaskImageAccess access, ImageViewType view_type = ImageViewType::MAX_ENUM)
            requires(T_ACCESS == TaskImageAccess::NONE && T_VIEW_TYPE == ImageViewType::MAX_ENUM)
            : handle{a_handle}, m_access{access}, m_view_type{view_type}
        {
        }

        static auto from(GenericTaskResourceUse const & input) -> TaskImageUse<> const &
        {
            DAXA_DBG_ASSERT_TRUE_M(input.type == TaskResourceUseType::IMAGE, "invalid TaskResourceUse cast");
            return *reinterpret_cast<TaskImageUse<> const *>(&input);
        }

        static auto from(GenericTaskResourceUse & input) -> TaskImageUse<> &
        {
            DAXA_DBG_ASSERT_TRUE_M(input.type == TaskResourceUseType::IMAGE, "invalid TaskResourceUse cast");
            return *reinterpret_cast<TaskImageUse<> *>(&input);
        }

        auto access() const -> TaskImageAccess
        {
            return m_access;
        }

        auto view_type() const -> ImageViewType
        {
            return m_view_type;
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

        auto to_generic() const -> GenericTaskResourceUse const &
        {
            return *reinterpret_cast<GenericTaskResourceUse const *>(this);
        }

        operator GenericTaskResourceUse const &() const
        {
            return to_generic();
        }
    };

    static inline constexpr size_t TASK_BUFFER_INPUT_SIZE = sizeof(TaskBufferUse<>);
    static inline constexpr size_t TASK_IMAGE_INPUT_SIZE = sizeof(TaskImageUse<>);

    static_assert(TASK_BUFFER_INPUT_SIZE == TASK_IMAGE_INPUT_SIZE, "should be impossible! contact Ipotrick");
    static_assert(TASK_BUFFER_INPUT_SIZE == TASK_INPUT_FIELD_SIZE, "should be impossible! contact Ipotrick");

    struct TaskUseOffsetType
    {
        u32 offset = {};
        TaskResourceUseType type = {};
    };

    template <typename BufFn, typename ImgFn>
    void for_each(std::span<GenericTaskResourceUse> uses, BufFn && buf_fn, ImgFn && img_fn)
    {
        for (u32 index = 0; index < uses.size(); ++index)
        {
            auto type = uses[index].type;
            switch (type)
            {
            case TaskResourceUseType::BUFFER:
            {
                auto & arg = TaskBufferUse<>::from(uses[index]);
                buf_fn(index, arg);
                break;
            }
            case TaskResourceUseType::IMAGE:
            {
                auto & arg = TaskImageUse<>::from(uses[index]);
                img_fn(index, arg);
                break;
            }
            default: break;
            }
        }
    }

    template <typename BufFn, typename ImgFn>
    void for_each(std::span<GenericTaskResourceUse const> uses, BufFn && buf_fn, ImgFn && img_fn)
    {
        for (u32 index = 0; index < uses.size(); ++index)
        {
            auto type = uses[index].type;
            switch (type)
            {
            case TaskResourceUseType::BUFFER:
            {
                auto const & arg = TaskBufferUse<>::from(uses[index]);
                buf_fn(index, arg);
                break;
            }
            case TaskResourceUseType::IMAGE:
            {
                auto const & arg = TaskImageUse<>::from(uses[index]);
                img_fn(index, arg);
                break;
            }
            default: break;
            }
        }
    }

    struct TaskInterface;

    struct BaseTask
    {
        virtual auto get_generic_uses() -> std::span<GenericTaskResourceUse> = 0;
        virtual auto get_generic_uses() const -> std::span<GenericTaskResourceUse const> = 0;
        virtual auto get_uses_constant_buffer_slot() const -> isize = 0;
        virtual auto get_name() const -> std::string = 0;
        virtual void callback(TaskInterface const & ti) = 0;
        virtual ~BaseTask() {}
    };

    template <typename T>
    concept UserUses =
        (sizeof(T) > 0 and sizeof(T) % TASK_INPUT_FIELD_SIZE == 0);

    template <typename T>
    concept UserTask =
        requires { T{}.uses; } and
        UserUses<decltype(T{}.uses)> and
        requires(TaskInterface interface) { T{}.callback(interface); };

    template <UserTask T_TASK>
    struct PredeclaredTask : public BaseTask
    {
        T_TASK task = {};
        using T_USES = decltype(T_TASK{}.uses);
        static constexpr usize USE_COUNT = sizeof(T_USES) / TASK_INPUT_FIELD_SIZE;

        PredeclaredTask(T_TASK const & task) : task{task} {}

        virtual ~PredeclaredTask() override = default;

        virtual auto get_generic_uses() -> std::span<GenericTaskResourceUse> override
        {
            return std::span{reinterpret_cast<GenericTaskResourceUse *>(&task.uses), USE_COUNT};
        }

        virtual auto get_generic_uses() const -> std::span<GenericTaskResourceUse const> override
        {
            return std::span{reinterpret_cast<GenericTaskResourceUse const *>(&task.uses), USE_COUNT};
        }

        virtual auto get_uses_constant_buffer_slot() const -> isize override
        {
            if constexpr (requires { T_TASK::CONSANT_BUFFER_SLOT; })
            {
                return T_TASK::CONSANT_BUFFER_SLOT;
            }
            else
            {
                return -1;
            }
        }

        virtual auto get_name() const -> std::string
        {
            if constexpr (requires { task.name; })
            {
                return std::string{task.name.data(), task.name.size()};
            }
            else
            {
                return std::string{""};
            }
        }

        virtual void callback(TaskInterface const & ti) override
        {
            task.callback(ti);
        }
    };

    struct InlineTask : public BaseTask
    {
        std::vector<GenericTaskResourceUse> uses = {};
        std::function<void(daxa::TaskInterface const &)> callback_lambda = {};
        std::string name = {};
        isize constant_buffer_slot = -1;

        InlineTask(
            std::vector<GenericTaskResourceUse> && a_uses,
            std::function<void(daxa::TaskInterface const &)> && a_callback_lambda,
            std::string && a_name, isize a_constant_buffer_slot)
            : uses{a_uses}, callback_lambda{a_callback_lambda}, name{a_name}, constant_buffer_slot{a_constant_buffer_slot}
        {
        }

        virtual ~InlineTask() = default;

        virtual auto get_generic_uses() -> std::span<GenericTaskResourceUse> override
        {
            return std::span{uses.data(), uses.size()};
        }

        virtual auto get_generic_uses() const -> std::span<GenericTaskResourceUse const> override
        {
            return std::span{uses.data(), uses.size()};
        }

        virtual auto get_uses_constant_buffer_slot() const -> isize override
        {
            return constant_buffer_slot;
        }

        virtual auto get_name() const -> std::string override
        {
            return name;
        }

        virtual void callback(TaskInterface const & ti) override
        {
            callback_lambda(ti);
        }
    };

    template <UserUses T>
    auto to_generic_uses(T const & uses_struct) -> std::vector<GenericTaskResourceUse>
    {
        std::vector<GenericTaskResourceUse> uses = {};
        uses.resize(sizeof(T) / sizeof(GenericTaskResourceUse), {});
        std::memcpy(uses.data(), &uses_struct, sizeof(T));
        return uses;
    }

    auto get_task_arg_shader_alignment(TaskResourceUseType type) -> u32;

    auto get_task_arg_shader_offsets_size(std::span<GenericTaskResourceUse> args) -> std::pair<std::vector<u32>, u32>;

    inline namespace task_resource_uses
    {
        using BufferShaderRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::SHADER_READ>;
        using BufferVertexShaderRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::VERTEX_SHADER_READ>;
        using BufferTessellationControlShaderRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::TESSELLATION_CONTROL_SHADER_READ>;
        using BufferTessellationEvaluationShaderRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::TESSELLATION_EVALUATION_SHADER_READ>;
        using BufferGeometryShaderRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::GEOMETRY_SHADER_READ>;
        using BufferFragmentShaderRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::FRAGMENT_SHADER_READ>;
        using BufferComputeShaderRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::COMPUTE_SHADER_READ>;
        using BufferShaderWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::SHADER_WRITE>;
        using BufferVertexShaderWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::VERTEX_SHADER_WRITE>;
        using BufferTessellationControlShaderWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::TESSELLATION_CONTROL_SHADER_WRITE>;
        using BufferTessellationEvaluationShaderWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::TESSELLATION_EVALUATION_SHADER_WRITE>;
        using BufferGeometryShaderWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::GEOMETRY_SHADER_WRITE>;
        using BufferFragmentShaderWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::FRAGMENT_SHADER_WRITE>;
        using BufferComputeShaderWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::COMPUTE_SHADER_WRITE>;
        using BufferShaderReadWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::SHADER_READ_WRITE>;
        using BufferVertexShaderReadWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::VERTEX_SHADER_READ_WRITE>;
        using BufferTessellationControlShaderReadWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::TESSELLATION_CONTROL_SHADER_READ_WRITE>;
        using BufferTessellationEvaluationShaderReadWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::TESSELLATION_EVALUATION_SHADER_READ_WRITE>;
        using BufferGeometryShaderReadWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::GEOMETRY_SHADER_READ_WRITE>;
        using BufferFragmentShaderReadWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::FRAGMENT_SHADER_READ_WRITE>;
        using BufferComputeShaderReadWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE>;
        using BufferIndexRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::INDEX_READ>;
        using BufferDrawIndirectInfoRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::DRAW_INDIRECT_INFO_READ>;
        using BufferTransferRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::TRANSFER_READ>;
        using BufferTransferWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::TRANSFER_WRITE>;
        using BufferHostTransferRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::HOST_TRANSFER_READ>;
        using BufferHostTransferWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::HOST_TRANSFER_WRITE>;

        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageShaderRead = daxa::TaskImageUse<daxa::TaskImageAccess::SHADER_READ, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageVertexShaderRead = daxa::TaskImageUse<daxa::TaskImageAccess::VERTEX_SHADER_READ, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageTessellationControlShaderRead = daxa::TaskImageUse<daxa::TaskImageAccess::TESSELLATION_CONTROL_SHADER_READ, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageTessellationEvaluationShaderRead = daxa::TaskImageUse<daxa::TaskImageAccess::TESSELLATION_EVALUATION_SHADER_READ, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageGeometryShaderRead = daxa::TaskImageUse<daxa::TaskImageAccess::GEOMETRY_SHADER_READ, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageFragmentShaderRead = daxa::TaskImageUse<daxa::TaskImageAccess::FRAGMENT_SHADER_READ, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageComputeShaderRead = daxa::TaskImageUse<daxa::TaskImageAccess::COMPUTE_SHADER_READ, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageShaderWrite = daxa::TaskImageUse<daxa::TaskImageAccess::SHADER_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageVertexShaderWrite = daxa::TaskImageUse<daxa::TaskImageAccess::VERTEX_SHADER_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageTessellationControlShaderWrite = daxa::TaskImageUse<daxa::TaskImageAccess::TESSELLATION_CONTROL_SHADER_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageTessellationEvaluationShaderWrite = daxa::TaskImageUse<daxa::TaskImageAccess::TESSELLATION_EVALUATION_SHADER_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageGeometryShaderWrite = daxa::TaskImageUse<daxa::TaskImageAccess::GEOMETRY_SHADER_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageFragmentShaderWrite = daxa::TaskImageUse<daxa::TaskImageAccess::FRAGMENT_SHADER_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageComputeShaderWrite = daxa::TaskImageUse<daxa::TaskImageAccess::COMPUTE_SHADER_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageShaderReadWrite = daxa::TaskImageUse<daxa::TaskImageAccess::SHADER_READ_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageVertexShaderReadWrite = daxa::TaskImageUse<daxa::TaskImageAccess::VERTEX_SHADER_READ_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageTessellationControlShaderReadWrite = daxa::TaskImageUse<daxa::TaskImageAccess::TESSELLATION_CONTROL_SHADER_READ_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageTessellationEvaluationShaderReadWrite = daxa::TaskImageUse<daxa::TaskImageAccess::TESSELLATION_EVALUATION_SHADER_READ_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageGeometryShaderReadWrite = daxa::TaskImageUse<daxa::TaskImageAccess::GEOMETRY_SHADER_READ_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageFragmentShaderReadWrite = daxa::TaskImageUse<daxa::TaskImageAccess::FRAGMENT_SHADER_READ_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageComputeShaderReadWrite = daxa::TaskImageUse<daxa::TaskImageAccess::COMPUTE_SHADER_READ_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageTransferRead = daxa::TaskImageUse<daxa::TaskImageAccess::TRANSFER_READ, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageTransferWrite = daxa::TaskImageUse<daxa::TaskImageAccess::TRANSFER_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageColorAttachment = daxa::TaskImageUse<daxa::TaskImageAccess::COLOR_ATTACHMENT, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageDepthAttachment = daxa::TaskImageUse<daxa::TaskImageAccess::DEPTH_ATTACHMENT, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageStencilAttachment = daxa::TaskImageUse<daxa::TaskImageAccess::STENCIL_ATTACHMENT, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageDepthStencilAttachment = daxa::TaskImageUse<daxa::TaskImageAccess::DEPTH_STENCIL_ATTACHMENT, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageDepthAttachmentRead = daxa::TaskImageUse<daxa::TaskImageAccess::DEPTH_ATTACHMENT_READ, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageStencilAttachmentRead = daxa::TaskImageUse<daxa::TaskImageAccess::STENCIL_ATTACHMENT_READ, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageDepthStencilAttachmentRead = daxa::TaskImageUse<daxa::TaskImageAccess::DEPTH_STENCIL_ATTACHMENT_READ, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageResolveWrite = daxa::TaskImageUse<daxa::TaskImageAccess::RESOLVE_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImagePresent = daxa::TaskImageUse<daxa::TaskImageAccess::PRESENT, T_VIEW_TYPE>;
    } // namespace task_resource_uses
} // namespace daxa