#pragma once

// Disable msvc warning on alignment padding.
#if defined(_MSC_VER)
#pragma warning(disable : 4324)
#endif

#if !DAXA_BUILT_WITH_UTILS_TASK_GRAPH
#error "[package management error] You must build Daxa with the DAXA_ENABLE_UTILS_TASK_GRAPH CMake option enabled, or request the utils-task-graph feature in vcpkg"
#endif

#include <span>
#include <cstring>

#include <daxa/core.hpp>
#include <daxa/device.hpp>

namespace daxa
{
    enum struct TaskBufferAccess
    {
        NONE,
        GRAPHICS_SHADER_READ,
        GRAPHICS_SHADER_WRITE,
        GRAPHICS_SHADER_READ_WRITE,
        COMPUTE_SHADER_READ,
        COMPUTE_SHADER_WRITE,
        COMPUTE_SHADER_READ_WRITE,
        TASK_SHADER_READ,
        TASK_SHADER_WRITE,
        TASK_SHADER_READ_WRITE,
        MESH_SHADER_READ,
        MESH_SHADER_WRITE,
        MESH_SHADER_READ_WRITE,
        VERTEX_SHADER_READ,
        VERTEX_SHADER_WRITE,
        VERTEX_SHADER_READ_WRITE,
        TESSELLATION_CONTROL_SHADER_READ,
        TESSELLATION_CONTROL_SHADER_WRITE,
        TESSELLATION_CONTROL_SHADER_READ_WRITE,
        TESSELLATION_EVALUATION_SHADER_READ,
        TESSELLATION_EVALUATION_SHADER_WRITE,
        TESSELLATION_EVALUATION_SHADER_READ_WRITE,
        GEOMETRY_SHADER_READ,
        GEOMETRY_SHADER_WRITE,
        GEOMETRY_SHADER_READ_WRITE,
        FRAGMENT_SHADER_READ,
        FRAGMENT_SHADER_WRITE,
        FRAGMENT_SHADER_READ_WRITE,
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
        GRAPHICS_SHADER_SAMPLED,
        GRAPHICS_SHADER_STORAGE_READ_WRITE,
        GRAPHICS_SHADER_STORAGE_WRITE_ONLY,
        GRAPHICS_SHADER_STORAGE_READ_ONLY,
        COMPUTE_SHADER_SAMPLED,
        COMPUTE_SHADER_STORAGE_WRITE_ONLY,
        COMPUTE_SHADER_STORAGE_READ_ONLY,
        COMPUTE_SHADER_STORAGE_READ_WRITE,
        TASK_SHADER_SAMPLED,
        TASK_SHADER_STORAGE_WRITE_ONLY,
        TASK_SHADER_STORAGE_READ_ONLY,
        TASK_SHADER_STORAGE_READ_WRITE,
        MESH_SHADER_SAMPLED,
        MESH_SHADER_STORAGE_WRITE_ONLY,
        MESH_SHADER_STORAGE_READ_ONLY,
        MESH_SHADER_STORAGE_READ_WRITE,
        VERTEX_SHADER_SAMPLED,
        VERTEX_SHADER_STORAGE_WRITE_ONLY,
        VERTEX_SHADER_STORAGE_READ_ONLY,
        VERTEX_SHADER_STORAGE_READ_WRITE,
        TESSELLATION_CONTROL_SHADER_SAMPLED,
        TESSELLATION_CONTROL_SHADER_STORAGE_WRITE_ONLY,
        TESSELLATION_CONTROL_SHADER_STORAGE_READ_ONLY,
        TESSELLATION_CONTROL_SHADER_STORAGE_READ_WRITE,
        TESSELLATION_EVALUATION_SHADER_SAMPLED,
        TESSELLATION_EVALUATION_SHADER_STORAGE_WRITE_ONLY,
        TESSELLATION_EVALUATION_SHADER_STORAGE_READ_ONLY,
        TESSELLATION_EVALUATION_SHADER_STORAGE_READ_WRITE,
        GEOMETRY_SHADER_SAMPLED,
        GEOMETRY_SHADER_STORAGE_WRITE_ONLY,
        GEOMETRY_SHADER_STORAGE_READ_ONLY,
        GEOMETRY_SHADER_STORAGE_READ_WRITE,
        FRAGMENT_SHADER_SAMPLED,
        FRAGMENT_SHADER_STORAGE_WRITE_ONLY,
        FRAGMENT_SHADER_STORAGE_READ_ONLY,
        FRAGMENT_SHADER_STORAGE_READ_WRITE,
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

    struct DAXA_EXPORT_CXX TaskGPUResourceView
    {
        TaskResourceIndex task_graph_index = {};
        TaskResourceIndex index = {};

        auto is_empty() const -> bool;
        auto is_persistent() const -> bool;

        auto operator<=>(TaskGPUResourceView const & other) const = default;
    };

    auto to_string(TaskGPUResourceView const & id) -> std::string;

    struct DAXA_EXPORT_CXX TaskBufferView : public TaskGPUResourceView
    {
    };

    struct DAXA_EXPORT_CXX TaskImageView : public TaskGPUResourceView
    {
        daxa::ImageMipArraySlice slice = {};
        auto view(daxa::ImageMipArraySlice const & new_slice) const -> TaskImageView
        {
            auto ret = *this;
            ret.slice = new_slice;
            return ret;
        }
        auto operator<=>(TaskGPUResourceView const & other) const = delete;
        auto operator<=>(TaskImageView const & other) const = default;
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
        TaskResourceUseType type;
        // This is necessary for c++ to properly generate copy and move operators.
        u8 raw[TASK_INPUT_FIELD_SIZE - sizeof(TaskResourceUseType)];
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

    struct ImplPersistentTaskBuffer;
    struct DAXA_EXPORT_CXX TaskBuffer : ManagedPtr<TaskBuffer, ImplPersistentTaskBuffer *>
    {
        TaskBuffer() = default;
        // TaskBuffer(TaskBuffer const & tb) = default;
        TaskBuffer(TaskBufferInfo const & info);

        operator TaskBufferView() const;

        auto view() const -> TaskBufferView;
        /// THREADSAFETY:
        /// * reference MUST NOT be read after the object is destroyed.
        /// @return reference to info of object.
        auto info() const -> TaskBufferInfo const &;
        auto get_state() const -> TrackedBuffers;

        void set_buffers(TrackedBuffers const & buffers);
        void swap_buffers(TaskBuffer & other);

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

    template <TaskBufferAccess T_ACCESS = TaskBufferAccess::NONE>
    struct alignas(TASK_INPUT_FIELD_SIZE) TaskBufferUse
    {
      private:
        friend struct ImplTaskGraph;
        TaskResourceUseType const type = TaskResourceUseType::BUFFER;
        std::span<BufferId const> buffers = {};
        TaskBufferAccess m_access = T_ACCESS;

      public:
        TaskBufferView handle = {};

        constexpr TaskBufferUse() = default;

        constexpr TaskBufferUse(TaskBufferView const & a_handle)
            : handle{a_handle}
        {
        }

        inline TaskBufferUse(TaskBuffer const & a_handle)
            : handle{a_handle}
        {
        }

        constexpr TaskBufferUse(TaskBufferView const & a_handle, TaskBufferAccess access)
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
        friend struct ImplTaskGraph;
        friend struct TaskGraphPermutation;
        TaskResourceUseType type = TaskResourceUseType::IMAGE;
        TaskImageAccess m_access = T_ACCESS;
        ImageViewType m_view_type = T_VIEW_TYPE;
        std::span<ImageId const> images = {};
        std::span<ImageViewId const> views = {};
        ImageLayout m_layout = {};

      public:
        TaskImageView handle = {};

        constexpr TaskImageUse() = default;

        constexpr TaskImageUse(TaskImageView const & a_handle)
            : handle{a_handle}
        {
        }

        TaskImageUse(TaskImage const & a_handle)
            : handle{a_handle}
        {
        }

        constexpr TaskImageUse(TaskImageView const & a_handle, TaskImageAccess access, ImageViewType view_type = ImageViewType::MAX_ENUM)
            requires(T_ACCESS == TaskImageAccess::NONE && T_VIEW_TYPE == ImageViewType::MAX_ENUM)
            : handle{a_handle}, m_access{access}, m_view_type{view_type}
        {
        }

        // Used to up-cast generic image uses to typed image uses.
        static auto from(GenericTaskResourceUse const & input) -> TaskImageUse<> const &
        {
            DAXA_DBG_ASSERT_TRUE_M(input.type == TaskResourceUseType::IMAGE, "invalid TaskResourceUse cast");
            return *reinterpret_cast<TaskImageUse<> const *>(&input);
        }

        // Used to up-cast generic image uses to typed image uses.
        static auto from(GenericTaskResourceUse & input) -> TaskImageUse<> &
        {
            DAXA_DBG_ASSERT_TRUE_M(input.type == TaskResourceUseType::IMAGE, "invalid TaskResourceUse cast");
            return *reinterpret_cast<TaskImageUse<> *>(&input);
        }

        // Used to cast typed image uses to generic image uses for inline tasks.
        auto to_generic() const -> GenericTaskResourceUse const &
        {
            return *reinterpret_cast<GenericTaskResourceUse const *>(this);
        }

        // Used to cast typed image uses to generic image uses for inline tasks.
        operator GenericTaskResourceUse const &() const
        {
            return to_generic();
        }

        /// @brief Each use has an access specified on creation.
        /// @return The access type of the use.
        auto access() const -> TaskImageAccess
        {
            return m_access;
        }

        /// @brief  The layout of images is controlled and changed by task graph.
        ///         They can change between tasks at any time.
        ///         Within each task callback the image layout for each used image is not changing.
        ///         The stable layout of a used image can be queried with this function.
        /// @return the image layout of the used image at the time of the task.
        auto layout() const -> ImageLayout
        {
            return m_layout;
        }

        /// @brief  Each image use has an optional image view type.
        ///         If the view type is not the default view daxa will create a new view and cache it.
        ///         If the view type is the default view type, daxa will simply use the default view when the slice fits the default views.
        /// @return View type of use cached image view.
        auto view_type() const -> ImageViewType
        {
            return m_view_type;
        }

        /// @brief  Each used task image is backed by a real daxa::ImageId at callback-time.
        /// @param index Each image use can be backed by multiple images, the index sets the index into the array of backed images.
        /// @return Backed image at given index
        auto image(u32 index = 0) const -> ImageId
        {
            DAXA_DBG_ASSERT_TRUE_M(images.size() > 0, "this function is only allowed to be called within a task callback");
            return images[index];
        }

        /// @brief  If the use is not the default slice and view type, daxa creates new image views and caches them.
        ///         These image views fit exactly the uses slice and image view type.
        /// @param index Each image use can be backed by multiple images, the index sets the index into the array of backed images.
        /// @return A cached image view that fits the uses slice and view type at the given image index.
        auto view(u32 index = 0) const -> ImageViewId
        {
            DAXA_DBG_ASSERT_TRUE_M(views.size() > 0, "this function is only allowed to be called within a task callback");
            return views[index];
        }
    };

    static inline constexpr size_t TASK_BUFFER_INPUT_SIZE = sizeof(TaskBufferUse<>);
    static inline constexpr size_t TASK_IMAGE_INPUT_SIZE = sizeof(TaskImageUse<>);
    static_assert(TASK_BUFFER_INPUT_SIZE == TASK_IMAGE_INPUT_SIZE, "should be impossible! contact Ipotrick");
    static_assert(TASK_BUFFER_INPUT_SIZE == TASK_INPUT_FIELD_SIZE, "should be impossible! contact Ipotrick");

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

    // Namespace containing implementation details
    namespace detail
    {
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
            (sizeof(T) > 0) and ((sizeof(T) % TASK_INPUT_FIELD_SIZE) == 0) and std::is_trivially_copyable_v<T>;

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

            PredeclaredTask(T_TASK const & a_task) : task{a_task} {}

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
                if constexpr (requires { T_TASK::CONSTANT_BUFFER_SLOT; })
                {
                    return T_TASK::CONSTANT_BUFFER_SLOT;
                }
                else
                {
                    return -1;
                }
            }

            virtual auto get_name() const -> std::string override
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
    } // namespace detail

    inline namespace task_resource_uses
    {
        using BufferGraphicsShaderRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::GRAPHICS_SHADER_READ>;
        using BufferGraphicsShaderWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::GRAPHICS_SHADER_WRITE>;
        using BufferGraphicsShaderReadWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::GRAPHICS_SHADER_READ_WRITE>;
        using BufferComputeShaderRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::COMPUTE_SHADER_READ>;
        using BufferComputeShaderWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::COMPUTE_SHADER_WRITE>;
        using BufferComputeShaderReadWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::COMPUTE_SHADER_READ_WRITE>;
        using BufferVertexShaderRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::VERTEX_SHADER_READ>;
        using BufferVertexShaderWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::VERTEX_SHADER_WRITE>;
        using BufferVertexShaderReadWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::VERTEX_SHADER_READ_WRITE>;
        using BufferTaskShaderRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::TASK_SHADER_READ>;
        using BufferTaskShaderWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::TASK_SHADER_WRITE>;
        using BufferTaskShaderReadWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::TASK_SHADER_READ_WRITE>;
        using BufferMeshShaderRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::MESH_SHADER_READ>;
        using BufferMeshShaderWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::MESH_SHADER_WRITE>;
        using BufferMeshShaderReadWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::MESH_SHADER_READ_WRITE>;
        using BufferTessellationControlShaderRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::TESSELLATION_CONTROL_SHADER_READ>;
        using BufferTessellationControlShaderWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::TESSELLATION_CONTROL_SHADER_WRITE>;
        using BufferTessellationControlShaderReadWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::TESSELLATION_CONTROL_SHADER_READ_WRITE>;
        using BufferTessellationEvaluationShaderRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::TESSELLATION_EVALUATION_SHADER_READ>;
        using BufferTessellationEvaluationShaderWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::TESSELLATION_EVALUATION_SHADER_WRITE>;
        using BufferTessellationEvaluationShaderReadWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::TESSELLATION_EVALUATION_SHADER_READ_WRITE>;
        using BufferGeometryShaderRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::GEOMETRY_SHADER_READ>;
        using BufferGeometryShaderWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::GEOMETRY_SHADER_WRITE>;
        using BufferGeometryShaderReadWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::GEOMETRY_SHADER_READ_WRITE>;
        using BufferFragmentShaderRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::FRAGMENT_SHADER_READ>;
        using BufferFragmentShaderWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::FRAGMENT_SHADER_WRITE>;
        using BufferFragmentShaderReadWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::FRAGMENT_SHADER_READ_WRITE>;
        using BufferIndexRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::INDEX_READ>;
        using BufferDrawIndirectInfoRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::DRAW_INDIRECT_INFO_READ>;
        using BufferTransferRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::TRANSFER_READ>;
        using BufferTransferWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::TRANSFER_WRITE>;
        using BufferHostTransferRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::HOST_TRANSFER_READ>;
        using BufferHostTransferWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::HOST_TRANSFER_WRITE>;

        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageGraphicsShaderSampled = daxa::TaskImageUse<daxa::TaskImageAccess::GRAPHICS_SHADER_SAMPLED, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageGraphicsShaderStorageWriteOnly = daxa::TaskImageUse<daxa::TaskImageAccess::GRAPHICS_SHADER_STORAGE_WRITE_ONLY, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageGraphicsShaderStorageReadOnly = daxa::TaskImageUse<daxa::TaskImageAccess::GRAPHICS_SHADER_STORAGE_READ_ONLY, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageGraphicsShaderStorageReadWrite = daxa::TaskImageUse<daxa::TaskImageAccess::GRAPHICS_SHADER_STORAGE_READ_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageComputeShaderSampled = daxa::TaskImageUse<daxa::TaskImageAccess::COMPUTE_SHADER_SAMPLED, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageComputeShaderStorageWriteOnly = daxa::TaskImageUse<daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageComputeShaderStorageReadOnly = daxa::TaskImageUse<daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageComputeShaderStorageReadWrite = daxa::TaskImageUse<daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageTessellationControlShaderSampled = daxa::TaskImageUse<daxa::TaskImageAccess::TESSELLATION_CONTROL_SHADER_SAMPLED, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageTessellationControlShaderStorageWriteOnly = daxa::TaskImageUse<daxa::TaskImageAccess::TESSELLATION_CONTROL_SHADER_STORAGE_WRITE_ONLY, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageTessellationControlShaderStorageReadOnly = daxa::TaskImageUse<daxa::TaskImageAccess::TESSELLATION_CONTROL_SHADER_STORAGE_READ_ONLY, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageTessellationControlShaderStorageReadWrite = daxa::TaskImageUse<daxa::TaskImageAccess::TESSELLATION_CONTROL_SHADER_STORAGE_READ_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageTessellationEvaluationShaderSampled = daxa::TaskImageUse<daxa::TaskImageAccess::TESSELLATION_EVALUATION_SHADER_SAMPLED, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageTessellationEvaluationShaderStorageWriteOnly = daxa::TaskImageUse<daxa::TaskImageAccess::TESSELLATION_EVALUATION_SHADER_STORAGE_WRITE_ONLY, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageTessellationEvaluationShaderStorageReadOnly = daxa::TaskImageUse<daxa::TaskImageAccess::TESSELLATION_EVALUATION_SHADER_STORAGE_READ_ONLY, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageTessellationEvaluationShaderStorageReadWrite = daxa::TaskImageUse<daxa::TaskImageAccess::TESSELLATION_EVALUATION_SHADER_STORAGE_READ_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageGeometryShaderSampled = daxa::TaskImageUse<daxa::TaskImageAccess::GEOMETRY_SHADER_SAMPLED, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageGeometryShaderStorageWriteOnly = daxa::TaskImageUse<daxa::TaskImageAccess::GEOMETRY_SHADER_STORAGE_WRITE_ONLY, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageGeometryShaderStorageReadOnly = daxa::TaskImageUse<daxa::TaskImageAccess::GEOMETRY_SHADER_STORAGE_READ_ONLY, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageGeometryShaderStorageReadWrite = daxa::TaskImageUse<daxa::TaskImageAccess::GEOMETRY_SHADER_STORAGE_READ_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageFragmentShaderSampled = daxa::TaskImageUse<daxa::TaskImageAccess::FRAGMENT_SHADER_SAMPLED, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageFragmentShaderStorageWriteOnly = daxa::TaskImageUse<daxa::TaskImageAccess::FRAGMENT_SHADER_STORAGE_WRITE_ONLY, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageFragmentShaderStorageReadOnly = daxa::TaskImageUse<daxa::TaskImageAccess::FRAGMENT_SHADER_STORAGE_READ_ONLY, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageFragmentShaderStorageReadWrite = daxa::TaskImageUse<daxa::TaskImageAccess::FRAGMENT_SHADER_STORAGE_READ_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageVertexShaderSampled = daxa::TaskImageUse<daxa::TaskImageAccess::VERTEX_SHADER_SAMPLED, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageVertexShaderStorageWriteOnly = daxa::TaskImageUse<daxa::TaskImageAccess::VERTEX_SHADER_STORAGE_WRITE_ONLY, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageVertexShaderStorageReadOnly = daxa::TaskImageUse<daxa::TaskImageAccess::VERTEX_SHADER_STORAGE_READ_ONLY, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageVertexShaderStorageReadWrite = daxa::TaskImageUse<daxa::TaskImageAccess::VERTEX_SHADER_STORAGE_READ_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageTaskShaderSampled = daxa::TaskImageUse<daxa::TaskImageAccess::TASK_SHADER_SAMPLED, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageTaskShaderStorageWriteOnly = daxa::TaskImageUse<daxa::TaskImageAccess::TASK_SHADER_STORAGE_WRITE_ONLY, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageTaskShaderStorageReadOnly = daxa::TaskImageUse<daxa::TaskImageAccess::TASK_SHADER_STORAGE_READ_ONLY, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageTaskShaderStorageReadWrite = daxa::TaskImageUse<daxa::TaskImageAccess::TASK_SHADER_STORAGE_READ_WRITE, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageMeshShaderSampled = daxa::TaskImageUse<daxa::TaskImageAccess::MESH_SHADER_SAMPLED, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageMeshShaderStorageWriteOnly = daxa::TaskImageUse<daxa::TaskImageAccess::MESH_SHADER_STORAGE_WRITE_ONLY, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageMeshShaderStorageReadOnly = daxa::TaskImageUse<daxa::TaskImageAccess::MESH_SHADER_STORAGE_READ_ONLY, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageMeshShaderStorageReadWrite = daxa::TaskImageUse<daxa::TaskImageAccess::MESH_SHADER_STORAGE_READ_WRITE, T_VIEW_TYPE>;
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