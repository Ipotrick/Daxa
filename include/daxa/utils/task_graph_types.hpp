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
        RAY_TRACING_SHADER_READ,
        RAY_TRACING_SHADER_WRITE,
        RAY_TRACING_SHADER_READ_WRITE,
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
        RAY_TRACING_SHADER_SAMPLED,
        RAY_TRACING_SHADER_STORAGE_WRITE_ONLY,
        RAY_TRACING_SHADER_STORAGE_READ_ONLY,
        RAY_TRACING_SHADER_STORAGE_READ_WRITE,
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

    /// NOTE: Must be multiple of sizeof(daxa::u64)!
    static inline constexpr size_t TASK_USE_TYPE_SIZE = 128;

    struct GenericTaskResourceUse
    {
        TaskResourceUseType type;
        u16 m_shader_array_size;
        // This is necessary for c++ to properly generate copy and move operators.
        u8 raw[TASK_USE_TYPE_SIZE - sizeof(TaskResourceUseType) - sizeof(u32)];
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

    // Needed to shut up compilers. Its not possible to bitcast spans otherwise.
    template <typename T>
    struct OpaqueSpan
    {
        std::array<u64, 2> data = {};
        auto span() const -> std::span<T> const &
        {
            return *reinterpret_cast<std::span<T> const *>(this);
        }
        auto span() -> std::span<T> &
        {
            return *reinterpret_cast<std::span<T> *>(this);
        }
    };

    template <TaskBufferAccess T_ACCESS = TaskBufferAccess::NONE, u32 T_SHADER_ARRAY_SIZE = 1, bool T_SHADER_AS_ADDRESS = false>
    struct TaskBufferUse
    {
      private:
        friend struct ImplTaskGraph;
        friend struct TaskInterfaceUses;
        friend struct TaskInterface;
        TaskResourceUseType const type = TaskResourceUseType::BUFFER; // + 4      -> 4 bytes
        u32 m_shader_array_size = T_SHADER_ARRAY_SIZE;                // + 4      -> 8 bytes
        OpaqueSpan<BufferId const> buffers = {};                      // + 2 * 8  -> 24 bytes
        TaskBufferAccess m_access = T_ACCESS;                         // + 4      -> 32 (4 padd)
        bool m_shader_as_address = T_SHADER_AS_ADDRESS;               // + 1      -> 32 (3 padd)

      public:
        TaskBufferView handle = {}; // + 8 -> 40 bytes

      private:
        std::array<u64, (TASK_USE_TYPE_SIZE / sizeof(u64)) - 5> padding = {};

      public:
        constexpr TaskBufferUse() = default;

        constexpr TaskBufferUse(TaskBufferView const & a_handle)
            : handle{a_handle}
        {
        }

        constexpr TaskBufferUse(TaskBuffer const & a_handle)
            : handle{a_handle}
        {
        }

        constexpr static auto from(GenericTaskResourceUse const & input) -> TaskBufferUse<> const &
        {
            return *reinterpret_cast<TaskBufferUse<> const *>(&input);
        }

        constexpr operator GenericTaskResourceUse const &() const
        {
            return *reinterpret_cast<GenericTaskResourceUse const *>(this);
        }

        constexpr auto access() const -> TaskBufferAccess
        {
            return m_access;
        }

        constexpr auto buffer(usize index = 0) const -> BufferId
        {
            DAXA_DBG_ASSERT_TRUE_M(buffers.span().size() > 0, "this function is only allowed to be called within a task callback");
            return buffers.span()[index];
        }
    };

    enum struct TaskImageUseArrayType : u16
    {
        RUNTIME_IMAGES,
        MIP_LEVELS,
    };

    template <
        TaskImageAccess T_ACCESS = TaskImageAccess::NONE,
        ImageViewType T_VIEW_TYPE = ImageViewType::MAX_ENUM,
        u16 T_SHADER_ARRAY_SIZE = 1u,
        TaskImageUseArrayType T_SHADER_ARRAY_TYPE = TaskImageUseArrayType::RUNTIME_IMAGES>
    struct TaskImageUse
    {
      private:
        friend struct ImplTaskGraph;
        friend struct TaskGraphPermutation;
        friend struct TaskInterfaceUses;
        friend struct TaskInterface;
        TaskResourceUseType type = TaskResourceUseType::IMAGE;              // +     4 -> 4  bytes
        u32 m_shader_array_size = T_SHADER_ARRAY_SIZE;                      // +     4 -> 8  bytes
        TaskImageAccess m_access = T_ACCESS;                                // +     4 -> 12 bytes
        ImageViewType m_view_type = T_VIEW_TYPE;                            // +     4 -> 16 bytes
        OpaqueSpan<ImageId const> images = {};                              // + 2 * 8 -> 32 bytes
        OpaqueSpan<ImageViewId const> views = {};                           // + 2 * 8 -> 48 bytes
        ImageLayout m_layout = {};                                          // +     4 -> 56 bytes (4 bytes padded)
        TaskImageUseArrayType m_shader_array_type = T_SHADER_ARRAY_TYPE;    // +     4 -> 56 bytes

      public:
        TaskImageView handle = {}; // + 24 -> 80 bytes

      private:
        std::array<u64, (TASK_USE_TYPE_SIZE / sizeof(u64)) - 10> padding = {};

      public:
        constexpr TaskImageUse() = default;

        constexpr TaskImageUse(TaskImageView const & a_handle)
            : handle{a_handle}
        {
        }

        constexpr TaskImageUse(TaskImage const & a_handle)
            : handle{a_handle}
        {
        }

        // Used to up-cast generic image uses to typed image uses.
        constexpr static auto from(GenericTaskResourceUse const & input) -> TaskImageUse<> const &
        {
            return *reinterpret_cast<TaskImageUse<> const *>(&input);
        }

        // Used to cast typed image uses to generic image uses for inline tasks.
        constexpr operator GenericTaskResourceUse const &() const
        {
            return *reinterpret_cast<GenericTaskResourceUse const *>(this);
        }

        /// @brief Each use has an access specified on creation.
        /// @return The access type of the use.
        constexpr auto access() const -> TaskImageAccess
        {
            return m_access;
        }

        /// @brief  The layout of images is controlled and changed by task graph.
        ///         They can change between tasks at any time.
        ///         Within each task callback the image layout for each used image is not changing.
        ///         The stable layout of a used image can be queried with this function.
        /// @return the image layout of the used image at the time of the task.
        constexpr auto layout() const -> ImageLayout
        {
            DAXA_DBG_ASSERT_TRUE_M(images.span().size() > 0, "this function is only allowed to be called within a task callback");
            return m_layout;
        }

        /// @brief  Each image use has an optional image view type.
        ///         If the view type is not the default view daxa will create a new view and cache it.
        ///         If the view type is the default view type, daxa will simply use the default view when the slice fits the default views.
        /// @return View type of use cached image view.
        constexpr auto view_type() const -> ImageViewType
        {
            return m_view_type;
        }

        /// @brief  Each used task image is backed by a real daxa::ImageId at callback-time.
        /// @param index Each image use can be backed by multiple images, the index sets the index into the array of backed images.
        /// @return Backed image at given index
        constexpr auto image(u32 index = 0) const -> ImageId
        {
            DAXA_DBG_ASSERT_TRUE_M(images.span().size() > 0, "this function is only allowed to be called within a task callback");
            return images.span()[index];
        }

        /// @brief  If the use is not the default slice and view type, daxa creates new image views and caches them.
        ///         These image views fit exactly the uses slice and image view type.
        /// @param index Each image use can be backed by multiple images, the index sets the index into the array of backed images.
        /// @return A cached image view that fits the uses slice and view type at the given image index.
        constexpr auto view(u32 index = 0) const -> ImageViewId
        {
            DAXA_DBG_ASSERT_TRUE_M(views.span().size() > 0, "this function is only allowed to be called within a task callback");
            return views.span()[index];
        }
    };

    static inline constexpr size_t TASK_BUFFER_INPUT_SIZE = sizeof(TaskBufferUse<>);
    static inline constexpr size_t TASK_IMAGE_INPUT_SIZE = sizeof(TaskImageUse<>);
    static_assert(TASK_BUFFER_INPUT_SIZE == TASK_IMAGE_INPUT_SIZE, "ABI Size Incompatilibity In Task Uses! Contact Ipotrick!");
    static_assert(TASK_BUFFER_INPUT_SIZE == TASK_USE_TYPE_SIZE, "ABI Size Incompatilibity In Task Uses! Contact Ipotrick!");

    template <typename BufFn, typename ImgFn>
    constexpr void for_each(std::span<GenericTaskResourceUse> uses, BufFn && buf_fn, ImgFn && img_fn)
    {
        for (u32 index = 0; index < uses.size(); ++index)
        {
            auto type = uses[index].type;
            switch (type)
            {
            case TaskResourceUseType::BUFFER:
            {
                auto & arg = *reinterpret_cast<TaskBufferUse<> *>(&uses[index]);
                buf_fn(index, arg);
                break;
            }
            case TaskResourceUseType::IMAGE:
            {
                auto & arg = *reinterpret_cast<TaskImageUse<> *>(&uses[index]);
                img_fn(index, arg);
                break;
            }
            default: break;
            }
        }
    }

    template <typename BufFn, typename ImgFn>
    constexpr void for_each(std::span<GenericTaskResourceUse const> uses, BufFn && buf_fn, ImgFn && img_fn)
    {
        for (u32 index = 0; index < uses.size(); ++index)
        {
            auto type = uses[index].type;
            switch (type)
            {
            case TaskResourceUseType::BUFFER:
            {
                auto & arg = *reinterpret_cast<TaskBufferUse<> const *>(&uses[index]);
                buf_fn(index, arg);
                break;
            }
            case TaskResourceUseType::IMAGE:
            {
                auto & arg = *reinterpret_cast<TaskImageUse<> const *>(&uses[index]);
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
        template <typename T>
        consteval usize task_head_shader_blob_size()
        {
            usize constexpr array_size = sizeof(T) / sizeof(GenericTaskResourceUse);
            auto const generic_uses = std::bit_cast<std::array<GenericTaskResourceUse, array_size>>(T{});
            usize byte_size = 0;
            for (auto const & guse : generic_uses)
            {
                byte_size += 8 * guse.m_shader_array_size;
            }
            return byte_size;
        }
        
        inline usize runtime_task_head_shader_blob_size(std::span<GenericTaskResourceUse const> generic_uses)
        {
            usize byte_size = 0;
            for (auto const & guse : generic_uses)
            {
                byte_size += 8 * guse.m_shader_array_size;
            }
            return byte_size;
        }

        struct BaseTask
        {
            virtual auto get_generic_uses() -> std::span<GenericTaskResourceUse> = 0;
            virtual auto get_generic_uses() const -> std::span<GenericTaskResourceUse const> = 0;
            virtual auto get_task_head_shader_blob_size() const -> u64 = 0;
            virtual auto get_name() const -> std::string = 0;
            virtual void callback(TaskInterface const & ti) = 0;
            virtual ~BaseTask() {}
        };

        template <typename T>
        concept UserUses =
            (sizeof(T) > 0) and ((sizeof(T) % TASK_USE_TYPE_SIZE) == 0) and std::is_trivially_copyable_v<T>;

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
            static constexpr usize USE_COUNT = sizeof(T_USES) / TASK_USE_TYPE_SIZE;

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

            virtual auto get_task_head_shader_blob_size() const -> u64 override
            {
                return task_head_shader_blob_size<T_USES>();
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

            InlineTask(
                std::vector<GenericTaskResourceUse> && a_uses,
                std::function<void(daxa::TaskInterface const &)> && a_callback_lambda,
                std::string && a_name)
                : uses{a_uses}, callback_lambda{a_callback_lambda}, name{a_name}
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

            virtual auto get_task_head_shader_blob_size() const -> u64 override
            {
                return runtime_task_head_shader_blob_size(uses);
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
    } // namespace detail

    template <detail::UserUses T>
    auto generic_uses_cast(T const & uses_struct) -> std::vector<GenericTaskResourceUse>
    {
        std::vector<GenericTaskResourceUse> uses = {};
        uses.resize(sizeof(T) / sizeof(GenericTaskResourceUse), {});
        std::memcpy(uses.data(), &uses_struct, sizeof(T));
        return uses;
    }

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
        using BufferRayTracingShaderRead = daxa::TaskBufferUse<daxa::TaskBufferAccess::RAY_TRACING_SHADER_READ>;
        using BufferRayTracingShaderWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::RAY_TRACING_SHADER_WRITE>;
        using BufferRayTracingShaderReadWrite = daxa::TaskBufferUse<daxa::TaskBufferAccess::RAY_TRACING_SHADER_READ_WRITE>;

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
        using ImageRayTracingShaderSampled = daxa::TaskImageUse<daxa::TaskImageAccess::RAY_TRACING_SHADER_SAMPLED, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageRayTracingShaderStorageWriteOnly = daxa::TaskImageUse<daxa::TaskImageAccess::RAY_TRACING_SHADER_STORAGE_WRITE_ONLY, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageRayTracingShaderStorageReadOnly = daxa::TaskImageUse<daxa::TaskImageAccess::RAY_TRACING_SHADER_STORAGE_READ_ONLY, T_VIEW_TYPE>;
        template <daxa::ImageViewType T_VIEW_TYPE = daxa::ImageViewType::MAX_ENUM>
        using ImageRayTracingShaderStorageReadWrite = daxa::TaskImageUse<daxa::TaskImageAccess::RAY_TRACING_SHADER_STORAGE_READ_WRITE, T_VIEW_TYPE>;
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