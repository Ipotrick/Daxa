#pragma once

#include <daxa/daxa.hpp>
#include <functional>
#include <memory>

#include "task_graph_types.hpp"

#if !DAXA_BUILT_WITH_UTILS_TASK_GRAPH
#error "[package management error] You must build Daxa with the DAXA_ENABLE_UTILS_TASK_GRAPH CMake option enabled, or request the utils-task-graph feature in vcpkg"
#endif

namespace daxa
{
    struct TaskGraph;
    struct Device;
    struct CommandSubmitInfo;
    struct PresentInfo;

    struct TaskTransientBufferInfo
    {
        u32 size = {};
        std::string_view name = {};
    };

    using TaskTransientTlasInfo = TaskTransientBufferInfo;

    struct TaskmanagedImageInfo
    {
        bool temporal = false;
        u32 dimensions = 2;
        Format format = Format::R8G8B8A8_UNORM;
        Extent3D size = {0, 0, 0};
        u32 mip_level_count = 1;
        u32 array_layer_count = 1;
        u32 sample_count = 1;
        std::string_view name = {};
    };

    using TaskTransientImageInfo = TaskmanagedImageInfo;

    struct TaskGraphInfo
    {
        Device device = {};
        /// @brief  Optionally the user can provide a swapchain. This enables the use of present.
        std::optional<Swapchain> swapchain = {};
        /// @brief  Task reordering can drastically improve performance,
        ///         yet is it also nice to have sequential callback execution.
        bool reorder_tasks = true;
        /// @brief  Allows task graph to alias transient resources memory (ofc only when that wont break the program)
        bool alias_transients = {};
        /// @brief  Some drivers have bad implementations for split barriers.
        ///         If that is the case for you, you can turn off all use of split barriers.
        ///         Daxa will use pipeline barriers instead if this is set.
        bool use_split_barriers = true;
        /// @brief  Each condition doubled the number of permutations.
        ///         For a low number of permutations its is preferable to precompile all permutations.
        ///         For a large number of permutations it might be preferable to only create the permutations actually used on the fly just before they are needed.
        ///         The second option is enabled by using jit (just in time) compilation.
        bool jit_compile_permutations = {};
        /// @brief  Task graph can branch the execution based on conditionals. All conditionals must be set before execution and stay constant while executing.
        ///         This is useful to create permutations of a task graph without having to create a separate task graph.
        ///         Another benefit is that task graph can generate synch between executions of permutations while it can not generate synch between two separate task graphs.
        usize permutation_condition_count = {};
        /// @brief  Task graph will put performance markers that are used by profilers like nsight around each tasks execution by default.
        bool enable_command_labels = true;
        std::array<f32, 4> task_graph_label_color = {0.463f, 0.333f, 0.671f, 1.0f};
        std::array<f32, 4> task_batch_label_color = {0.563f, 0.433f, 0.771f, 1.0f};
        std::array<f32, 4> task_label_color = {0.663f, 0.533f, 0.871f, 1.0f};
        /// @brief  Records debug information about the execution if enabled. This string is retrievable with the function get_debug_string.
        bool record_debug_information = {};
        /// @brief  Sets the size of the linear allocator of device local, host visible memory used by the linear staging allocator.
        ///         This memory is used internally as well as by tasks via the TaskInterface::get_allocator().
        ///         Setting the size to 0, disables a few task list features but also eliminates the memory allocation.
        u32 staging_memory_pool_size = 1u << 17u; // 128kib
        /// @brief  CPU Memory allocated for task data
        u32 task_memory_pool_size = 1u << 19u; // 512kib
        // Useful for debugging tools that are invisible to the graph.
        ImageUsageFlags additional_transient_image_usage_flags = {};
        // Useful for reflection/ debugging.
        std::function<void(TaskInterface)> pre_task_callback = {};
        std::function<void(TaskInterface)> post_task_callback = {};
        Queue default_queue = QUEUE_MAIN;
        std::string_view name = {};
    };

    struct TaskSubmitInfo
    {
        PipelineStageFlags * additional_src_stages = {};
        std::span<ExecutableCommandList> * additional_command_lists = {};
        std::span<BinarySemaphore> * additional_wait_binary_semaphores = {};
        std::span<BinarySemaphore> * additional_signal_binary_semaphores = {};
        std::span<std::pair<TimelineSemaphore, u64>> * additional_wait_timeline_semaphores = {};
        std::span<std::pair<TimelineSemaphore, u64>> * additional_signal_timeline_semaphores = {};
    };

    struct TaskPresentInfo
    {
        Queue queue = QUEUE_MAIN;
        std::span<BinarySemaphore> * additional_binary_semaphores = {};
    };

    struct TaskCompleteInfo
    {
    };

    struct TaskImageLastUse
    {
        ImageMipArraySlice slice = {};
        ImageLayout layout = {};
        Access access = {};
    };

    struct TaskGraphConditionalInfo
    {
        u32 condition_index = {};
        std::function<void()> when_true = {};
        std::function<void()> when_false = {};
    };

    struct ExecutionInfo
    {
        std::span<bool> permutation_condition_values = {};
        bool record_debug_string = {};
    };

    /*
         __/\__
    . _  \\''//
    -( )-/_||_\
     .'. \_()_/
      |  | . \
      |  | .  \
    . '. \_____\.
    */    

    static constexpr inline daxa::usize MAX_TASK_CALLBACK_DATA_SIZE = 512ull - sizeof(daxa::u64); 

    template<typename T>
    concept TaskCallbackLambda = std::is_copy_constructible_v<T> && sizeof(T) <= MAX_TASK_CALLBACK_DATA_SIZE && alignof(T) <= alignof(daxa::u64);

    struct TaskCallback
    {
        std::array<daxa::u64, MAX_TASK_CALLBACK_DATA_SIZE> data = {};
        void (*callback)(daxa::TaskInterface ti, void*) = nullptr;
        usize size = {};

        template<TaskCallbackLambda TFunc>
        void store(TFunc const & f)
        {
            std::memcpy(data.data(), &f, sizeof(f));
            size = sizeof(f);
            callback = [](daxa::TaskInterface ti, void* v)
            {
                TFunc & f_local = *reinterpret_cast<TFunc*>(v);
                f_local(ti);
            };
        }

        void execute(daxa::TaskInterface ti)
        {
            if (callback)
            {
                callback(ti, data.data());
            }
        }

        void operator()(daxa::TaskInterface ti)
        {
            execute(ti);
        }

        auto is_empty() const -> bool
        {
            return callback == nullptr;
        }

        operator bool() const
        {
            return !is_empty();
        }

        void clear() 
        {
            callback = {};
            size = {};
        }
    };

#if !DAXA_REMOVE_DEPRECATED
    struct [[deprecated("Use struct InlineTask constructors instead, API:3.1")]] InlineTaskInfo
    {
        TaskType task_type = TaskType::GENERAL;
        FixedList<TaskAttachmentInfo, MAX_TASK_ATTACHMENTS> attachments = {};
        TaskCallback task = {};
        std::string_view name = "unnamed";
    };
#endif

    inline namespace detail
    {
        template <typename TaskHeadAttachmentDeclT>
        auto convert_to_task_attachment_info(TaskHeadAttachmentDeclT const & attachment_decl, TaskViewVariant const & view, TaskStage default_stage = TaskStage::NONE) -> TaskAttachmentInfo
        {
            TaskAttachmentInfo ret = {};
            switch (attachment_decl.type)
            {
            case TaskAttachmentType::BUFFER:
            {
                TaskBufferAttachmentInfo info;
                info.name = attachment_decl.value.buffer.name;
                info.task_access = attachment_decl.value.buffer.task_access;
                if (info.task_access.stage == TaskStage::NONE && default_stage != TaskStage::NONE)
                {
                    info.task_access.stage = default_stage;
                }
                info.shader_array_size = attachment_decl.value.buffer.shader_array_size;
                info.shader_as_address = attachment_decl.value.buffer.shader_as_address;
                if (auto * ptr = get_if<TaskBufferView>(&view))
                {
                    info.view = *ptr;
                }
                else
                {
                    info.view = {};
                }
                ret = info;
            }
            break;
            case TaskAttachmentType::TLAS:
            {
                TaskTlasAttachmentInfo info;
                info.name = attachment_decl.value.tlas.name;
                info.task_access = attachment_decl.value.tlas.task_access;
                if (info.task_access.stage == TaskStage::NONE && default_stage != TaskStage::NONE)
                {
                    info.task_access.stage = default_stage;
                }
                info.shader_as_address = attachment_decl.value.tlas.shader_as_address;
                if (auto * ptr = get_if<TaskTlasView>(&view))
                {
                    info.view = *ptr;
                }
                else
                {
                    info.view = {};
                }
                ret = info;
            }
            break;
            case TaskAttachmentType::BLAS:
            {
                TaskBlasAttachmentInfo info;
                info.name = attachment_decl.value.blas.name;
                info.task_access = attachment_decl.value.blas.task_access;
                if (info.task_access.stage == TaskStage::NONE && default_stage != TaskStage::NONE)
                {
                    info.task_access.stage = default_stage;
                }
                if (auto * ptr = get_if<TaskBlasView>(&view))
                {
                    info.view = *ptr;
                }
                else
                {
                    info.view = {};
                }
                ret = info;
            }
            break;
            case TaskAttachmentType::IMAGE:
            {
                TaskImageAttachmentInfo info;
                info.name = attachment_decl.value.image.name;
                info.task_access = attachment_decl.value.image.task_access;
                if (info.task_access.stage == TaskStage::NONE && default_stage != TaskStage::NONE)
                {
                    info.task_access.stage = default_stage;
                }
                info.view_type = attachment_decl.value.image.view_type;
                info.shader_array_size = attachment_decl.value.image.shader_array_size;
                info.shader_array_type = attachment_decl.value.image.shader_array_type;
                info.shader_as_index = attachment_decl.value.image.shader_as_index;
                if (auto * ptr = get_if<TaskImageView>(&view))
                {
                    info.view = *ptr;
                }
                else
                {
                    info.view = {};
                }
                ret = info;
            }
            break;
            default:
            {
                DAXA_DBG_ASSERT_TRUE_M(false, "Declared attachment count does not match actually declared attachment count");
            }
            break;
            }
            return ret;
        }
    } // namespace detail

    struct NoTaskHeadStruct
    {
        struct Views
        {
        };

        static constexpr inline usize ATTACHMENT_COUNT = 0;
    };

    DAXA_EXPORT_CXX auto error_message_unassigned_buffer_view(std::string_view task_name, std::string_view attachment_name) -> std::string;
    DAXA_EXPORT_CXX auto error_message_unassigned_image_view(std::string_view task_name, std::string_view attachment_name) -> std::string;
    DAXA_EXPORT_CXX auto error_message_unassigned_tlas_view(std::string_view task_name, std::string_view attachment_name) -> std::string;
    DAXA_EXPORT_CXX auto error_message_unassigned_blas_view(std::string_view task_name, std::string_view attachment_name) -> std::string;

    template <typename TaskHeadT>
    struct TInlineTask
    {
        // construction interface:

        TInlineTask()
        {
            value = {}; // Prevents ICE
        }
#if !DAXA_REMOVE_DEPRECATED
        [[deprecated("Use other InlineTask constructors instead, API:3.1")]] TInlineTask(InlineTaskInfo const & info)
        {
            value = {}; // Prevents ICE
            value._internal._task_type = info.task_type;
            value._internal._default_stage = task_type_default_stage(info.task_type);
            value._internal._attachments = info.attachments;
            value._internal._callback = info.task;
            value._internal._name = info.name;
        }
#endif
        TInlineTask(std::string_view name, TaskType task_type = TaskType::GENERAL)
        {
            value._internal._name = name;
            value._internal._task_type = task_type;
            value._internal._default_stage = task_type_default_stage(task_type);

            if constexpr(!std::is_same_v<TaskHeadT, NoTaskHeadStruct>)
            {
                *this = uses_head<TaskHeadT>();
            }
        }
        static auto Raster(std::string_view name) -> TInlineTask
            requires std::is_same_v<TaskHeadT, NoTaskHeadStruct>
        {
            return TInlineTask(name, TaskType::RASTER);
        }
        static auto Compute(std::string_view name) -> TInlineTask
            requires std::is_same_v<TaskHeadT, NoTaskHeadStruct>
        {
            return TInlineTask(name, TaskType::COMPUTE);
        }
        static auto RayTracing(std::string_view name) -> TInlineTask
            requires std::is_same_v<TaskHeadT, NoTaskHeadStruct>
        {
            return TInlineTask(name, TaskType::RAY_TRACING);
        }
        static auto Transfer(std::string_view name) -> TInlineTask
            requires std::is_same_v<TaskHeadT, NoTaskHeadStruct>
        {
            return TInlineTask(name, TaskType::TRANSFER);
        }

        // c++ rule of 5 or whatever:

        ~TInlineTask()
        {
            value.~InternalValue<Allow::NONE, TaskStage::NONE>();
        }
        TInlineTask(TInlineTask && other)
        {
            this->value = {}; // Prevents ICE
            this->value._internal._attachments = std::move(other.value._internal._attachments);
            this->value._internal._callback = std::move(other.value._internal._callback);
            this->value._internal._name = std::move(other.value._internal._name);
            this->value._internal._task_type = std::move(other.value._internal._task_type);
            this->value._internal._default_stage = std::move(other.value._internal._default_stage);
            this->value._internal._queue = std::move(other.value._internal._queue);
            other.value._internal._attachments = {};
            other.value._internal._callback = {};
            other.value._internal._name = {};
            other.value._internal._task_type = {};
            other.value._internal._default_stage = {};
            other.value._internal._queue = {};
        }
        TInlineTask(TInlineTask const & other)
        {
            this->value = {}; // Prevents ICE
            this->value._internal._attachments = other.value._internal._attachments;
            this->value._internal._callback = other.value._internal._callback;
            this->value._internal._name = other.value._internal._name;
            this->value._internal._task_type = other.value._internal._task_type;
            this->value._internal._default_stage = other.value._internal._default_stage;
            this->value._internal._queue = other.value._internal._queue;
        }

      private:
        // Implementation details:

        enum Allow
        {
            NONE = 0,
            READ = (1 << 0),
            WRITE = (1 << 1),
            READ_WRITE = (1 << 2),
            SAMPLED = (1 << 5),
        };

        struct Internal
        {
            FixedList<TaskAttachmentInfo, MAX_TASK_ATTACHMENTS> _attachments = {};
            TaskCallback _callback = {};
            std::string_view _name = {};
            TaskType _task_type = TaskType::GENERAL;
            TaskStage _default_stage = TaskStage::ANY_COMMAND;
            Queue _queue = QUEUE_MAIN;

            void _process_parameter(TaskStage, TaskAccessType, ImageViewType &, TaskStage) {}
            void _process_parameter(TaskStage stage, TaskAccessType type, ImageViewType &, TaskBufferBlasTlasViewOrBufferBlasTlas auto param)
            {
                _attachments.push_back(inl_attachment(TaskAccess{stage, type}, param));
            }
            void _process_parameter(TaskStage stage, TaskAccessType type, ImageViewType & view_override, TaskImageViewOrTaskImage auto param)
            {
                _attachments.push_back(inl_attachment(TaskAccess{stage, type}, param, view_override));
            }
            void _process_parameter(TaskStage, TaskAccessType, ImageViewType & view_override, ImageViewType param)
            {
                view_override = param;
            }
        };
        template <Allow ALLOWED_ACCESS, TaskStage STAGE = TaskStage::NONE>
        struct InternalValue
        {
            Internal _internal = {};

            // inline attachment interface:

            template <TaskResourceViewOrResourceOrImageViewType... TParams>
            auto _process_parameters(TaskAccessType access, TaskStage stage, TParams... v) -> TInlineTask &
            {
                ImageViewType view_override = ImageViewType::MAX_ENUM;
                (_internal._process_parameter(stage, access, view_override, v), ...);
                return *reinterpret_cast<TInlineTask *>(this);
            }

            template <TaskResourceViewOrResourceOrImageViewType... TParams>
            auto reads(TParams... v) -> TInlineTask &
                requires((ALLOWED_ACCESS & Allow::READ) != 0)
            {
                return _process_parameters(TaskAccessType::READ, STAGE, v...);
            }

            template <TaskResourceViewOrResourceOrImageViewType... TParams>
            auto writes(TParams... v) -> TInlineTask &
                requires((ALLOWED_ACCESS & Allow::WRITE) != 0)
            {
                return _process_parameters(TaskAccessType::WRITE, STAGE, v...);
            }

            template <TaskResourceViewOrResourceOrImageViewType... TParams>
            auto writes_concurrent(TParams... v) -> TInlineTask &
                requires((ALLOWED_ACCESS & Allow::WRITE) != 0)
            {
                return _process_parameters(TaskAccessType::WRITE_CONCURRENT, STAGE, v...);
            }

            template <TaskResourceViewOrResourceOrImageViewType... TParams>
            auto reads_writes(TParams... v) -> TInlineTask &
                requires((ALLOWED_ACCESS & Allow::READ_WRITE) != 0)
            {
                return _process_parameters(TaskAccessType::READ_WRITE, STAGE, v...);
            }

            template <TaskResourceViewOrResourceOrImageViewType... TParams>
            auto reads_writes_concurrent(TParams... v) -> TInlineTask &
                requires((ALLOWED_ACCESS & Allow::READ_WRITE) != 0)
            {
                return _process_parameters(TaskAccessType::READ_WRITE_CONCURRENT, STAGE, v...);
            }

            template <TaskImageViewOrTaskImageOrImageViewType... TParams>
            auto samples(TParams... v) -> TInlineTask &
                requires((ALLOWED_ACCESS & Allow::SAMPLED) != 0)
            {
                return _process_parameters(TaskAccessType::SAMPLED, STAGE, v...);
            }

            // task head attachment interface:

            static inline constexpr bool HAS_HEAD = !std::is_same_v<TaskHeadT, NoTaskHeadStruct>;

            auto _process_th_views(TaskHeadT::Views const & views, TaskAccessType access = TaskAccessType::NONE, TaskStage stage = TaskStage::NONE, bool keep_access = false) -> TInlineTask &
            {
                constexpr bool IS_GENERIC_CALL = STAGE == TaskStage::NONE;
                if constexpr (!IS_GENERIC_CALL)
                {
                    stage = STAGE;
                }

                if (stage == TaskStage::NONE)
                {
                    stage = task_type_default_stage(_internal._task_type);
                }

                auto av = views.convert_to_array(); // converts views to flat array
                for (u32 i = 0; i < TaskHeadT::ATTACHMENT_COUNT; ++i)
                {
                    TaskAttachmentInfo & attach = _internal._attachments[i];
                    if (daxa::holds_alternative<TaskViewUndefined>(av[i])) // skip unassigned views
                    {
                    }
                    else if (TaskBufferView * buffer_ptr = daxa::get_if<TaskBufferView>(&av[i]); buffer_ptr != nullptr)
                    {
                        DAXA_DBG_ASSERT_TRUE_M(!buffer_ptr->is_empty(), error_message_unassigned_buffer_view(attach.value.buffer.name, _internal._name));
                        if (!keep_access)
                        {
                            attach.value.buffer.task_access.stage = stage;
                            attach.value.buffer.task_access.type = access;
                        }
                        attach.value.buffer.view = *buffer_ptr;
                    }
                    else if (TaskImageView * image_ptr = daxa::get_if<TaskImageView>(&av[i]); image_ptr != nullptr)
                    {
                        DAXA_DBG_ASSERT_TRUE_M(!image_ptr->is_empty(), error_message_unassigned_image_view(attach.value.image.name, _internal._name));
                        if (!keep_access)
                        {
                            attach.value.image.task_access.stage = stage;
                            attach.value.image.task_access.type = access;
                        }
                        attach.value.image.view = *image_ptr;
                    }
                    else if (TaskBlasView * blas_ptr = daxa::get_if<TaskBlasView>(&av[i]); blas_ptr != nullptr)
                    {
                        DAXA_DBG_ASSERT_TRUE_M(!blas_ptr->is_empty(), error_message_unassigned_tlas_view(attach.value.blas.name, _internal._name));
                        if (!keep_access)
                        {
                            attach.value.blas.task_access.stage = stage;
                            attach.value.blas.task_access.type = access;
                        }
                        attach.value.blas.view = *blas_ptr;
                    }
                    else if (TaskTlasView * tlas_ptr = daxa::get_if<TaskTlasView>(&av[i]); tlas_ptr != nullptr)
                    {
                        DAXA_DBG_ASSERT_TRUE_M(!tlas_ptr->is_empty(), error_message_unassigned_blas_view(attach.value.tlas.name, _internal._name));
                        if (!keep_access)
                        {
                            attach.value.tlas.task_access.stage = stage;
                            attach.value.tlas.task_access.type = access;
                        }
                        attach.value.tlas.view = *tlas_ptr;
                    }
                }
                return *reinterpret_cast<TInlineTask *>(this);
            }

            auto reads(TaskHeadT::Views const & views) -> TInlineTask &
                requires(HAS_HEAD)
            {
                return _process_th_views(views, TaskAccessType::READ, STAGE);
            }

            auto writes(TaskHeadT::Views const & views) -> TInlineTask &
                requires(HAS_HEAD)
            {
                return _process_th_views(views, TaskAccessType::WRITE, STAGE);
            }

            auto writes_concurrent(TaskHeadT::Views const & views) -> TInlineTask &
                requires(HAS_HEAD)
            {
                return _process_th_views(views, TaskAccessType::WRITE_CONCURRENT, STAGE);
            }

            auto reads_writes(TaskHeadT::Views const & views) -> TInlineTask &
                requires(HAS_HEAD)
            {
                return _process_th_views(views, TaskAccessType::READ_WRITE, STAGE);
            }

            auto reads_writes_concurrent(TaskHeadT::Views const & views) -> TInlineTask &
                requires(HAS_HEAD)
            {
                return _process_th_views(views, TaskAccessType::READ_WRITE_CONCURRENT, STAGE);
            }

            auto samples(TaskHeadT::Views const & views) -> TInlineTask &
                requires(HAS_HEAD)
            {
                return _process_th_views(views, TaskAccessType::SAMPLED, STAGE);
            }
        };

      public:
        // typed inline attachment interface:

        union
        {
            InternalValue<Allow::NONE, TaskStage::NONE> value = {};
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::VERTEX_SHADER> vertex_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::TESSELLATION_CONTROL_SHADER> tesselation_control_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::TESSELLATION_EVALUATION_SHADER> tesselation_evaluation_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::GEOMETRY_SHADER> geometry_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::FRAGMENT_SHADER> fragment_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::COMPUTE_SHADER> compute_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::RAY_TRACING_SHADER> ray_tracing_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::TASK_SHADER> task_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::MESH_SHADER> mesh_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::PRE_RASTERIZATION_SHADERS> pre_rasterization_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::RASTER_SHADER> raster_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::SHADER> shader;
            InternalValue<Allow(Allow::WRITE | Allow::READ_WRITE), TaskStage::COLOR_ATTACHMENT> color_attachment;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::DEPTH_STENCIL_ATTACHMENT> depth_stencil_attachment;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE), TaskStage::RESOLVE> resolve;
            InternalValue<Allow::READ, TaskStage::PRESENT> present;
            InternalValue<Allow::READ, TaskStage::INDIRECT_COMMAND> indirect_cmd;
            InternalValue<Allow::READ, TaskStage::INDEX_INPUT> index_input;
            InternalValue<Allow(Allow::READ | Allow::WRITE), TaskStage::TRANSFER> transfer;
            InternalValue<Allow(Allow::READ | Allow::WRITE), TaskStage::HOST> host;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE), TaskStage::AS_BUILD> acceleration_structure_build;
        };

      private:
#ifndef __clang__ // MSVC STL does not implement these for clang :/
        // Per c++ spec, it is only legal to access multiple union members at the same time IF AND ONLY IF:
        // * all types in the union are standard layout
        // * all types in the union share a common initial sequence for all members accessed
        //   * or all types within the union are layout compatible
        using TEST_TYPE_A = InternalValue<Allow::NONE, TaskStage::NONE>;
        using TEST_TYPE_B = InternalValue<Allow::NONE, TaskStage::NONE>;
        static constexpr inline bool UNION_MEMBERS_ARE_STANDART_LAYOUT = std::is_standard_layout_v<TEST_TYPE_A> && std::is_standard_layout_v<TEST_TYPE_B>;
        static constexpr inline bool UNION_MEMBERS_ARE_LAYOUT_COMPATIBLE = std::is_layout_compatible_v<TEST_TYPE_A, TEST_TYPE_B>;
        static_assert(UNION_MEMBERS_ARE_STANDART_LAYOUT && UNION_MEMBERS_ARE_LAYOUT_COMPATIBLE);
#endif

      public:
        // untyped inline attachments interface:

        template <TaskResourceViewOrResourceOrImageViewTypeOrStage... TParams>
        auto reads(TParams... v) -> TInlineTask & { return value._process_parameters(TaskAccessType::READ, value._internal._default_stage, v...); }

        template <TaskResourceViewOrResourceOrImageViewTypeOrStage... TParams>
        auto writes(TParams... v) -> TInlineTask & { return value._process_parameters(TaskAccessType::WRITE, value._internal._default_stage, v...); }

        template <TaskResourceViewOrResourceOrImageViewTypeOrStage... TParams>
        auto writes_concurrent(TParams... v) -> TInlineTask & { return value._process_parameters(TaskAccessType::WRITE_CONCURRENT, value._internal._default_stage, v...); }

        template <TaskResourceViewOrResourceOrImageViewTypeOrStage... TParams>
        auto reads_writes(TParams... v) -> TInlineTask & { return value._process_parameters(TaskAccessType::READ_WRITE, value._internal._default_stage, v...); }

        template <TaskResourceViewOrResourceOrImageViewTypeOrStage... TParams>
        auto reads_writes_concurrent(TParams... v) -> TInlineTask & { return value._process_parameters(TaskAccessType::READ_WRITE_CONCURRENT, value._internal._default_stage, v...); }

        template <TaskResourceViewOrResourceOrImageViewTypeOrStage... TParams>
        auto samples(TParams... v) -> TInlineTask & { return value._process_parameters(TaskAccessType::SAMPLED, value._internal._default_stage, v...); }
        

        template <TaskResourceViewOrResourceOrImageViewTypeOrStage... TParams>
        auto reads(TaskStage stage, TParams... v) -> TInlineTask & { return value._process_parameters(TaskAccessType::READ, stage, v...); }

        template <TaskResourceViewOrResourceOrImageViewTypeOrStage... TParams>
        auto writes(TaskStage stage, TParams... v) -> TInlineTask & { return value._process_parameters(TaskAccessType::WRITE, stage, v...); }

        template <TaskResourceViewOrResourceOrImageViewTypeOrStage... TParams>
        auto writes_concurrent(TaskStage stage, TParams... v) -> TInlineTask & { return value._process_parameters(TaskAccessType::WRITE_CONCURRENT, stage, v...); }

        template <TaskResourceViewOrResourceOrImageViewTypeOrStage... TParams>
        auto reads_writes(TaskStage stage, TParams... v) -> TInlineTask & { return value._process_parameters(TaskAccessType::READ_WRITE, stage, v...); }

        template <TaskResourceViewOrResourceOrImageViewTypeOrStage... TParams>
        auto reads_writes_concurrent(TaskStage stage, TParams... v) -> TInlineTask & { return value._process_parameters(TaskAccessType::READ_WRITE_CONCURRENT, stage, v...); }

        template <TaskResourceViewOrResourceOrImageViewTypeOrStage... TParams>
        auto samples(TaskStage stage, TParams... v) -> TInlineTask & { return value._process_parameters(TaskAccessType::SAMPLED, stage, v...); }


        template <TaskResourceViewOrResource... TParams>
        auto uses(TaskAccess access, TParams... v) -> TInlineTask & { return value._process_parameters(access.type, access.stage, v...); }

        auto uses(TaskAttachmentInfo const & inl_attachment) -> TInlineTask &
        {
            value._internal._attachments.push_back(inl_attachment);
            return *this;
        }

        // task head interface:

        static inline constexpr bool HAS_HEAD = !std::is_same_v<TaskHeadT, NoTaskHeadStruct>;

        template <typename NewTaskHeadInfo>
        auto uses_head() -> TInlineTask<NewTaskHeadInfo>
            requires(!HAS_HEAD)
        {
            DAXA_DBG_ASSERT_TRUE_M(value._internal._attachments.size() == 0, "Detected invalid task head attachment use! Task heads must be added to tasks BEFORE ANY non-head attachments are added to a task!");
            auto head_default_stage = task_type_default_stage(NewTaskHeadInfo::TYPE);
            if (value._internal._name.size() == 0)
            {
                value._internal._name = NewTaskHeadInfo::NAME;
            }
            TInlineTask<NewTaskHeadInfo> ret = {};
            for (u32 i = 0; i < NewTaskHeadInfo::ATTACHMENT_COUNT; ++i)
            {
                ret.value._internal._attachments.push_back(detail::convert_to_task_attachment_info(NewTaskHeadInfo::AT._internal.value[i], {}, head_default_stage));
            }
            ret.value._internal._callback = this->value._internal._callback;
            ret.value._internal._name = this->value._internal._name;
            ret.value._internal._task_type = this->value._internal._task_type;
            return ret;
        }

        auto reads(TaskHeadT::Views const & views) -> TInlineTask &
            requires(HAS_HEAD)
        {
            return value._process_th_views(views, TaskAccessType::READ, value._internal._default_stage);
        }

        auto writes(TaskHeadT::Views const & views) -> TInlineTask &
            requires(HAS_HEAD)
        {
            return value._process_th_views(views, TaskAccessType::WRITE, value._internal._default_stage);
        }

        auto writes_concurrent(TaskHeadT::Views const & views) -> TInlineTask &
            requires(HAS_HEAD)
        {
            return value._process_th_views(views, TaskAccessType::WRITE_CONCURRENT, value._internal._default_stage);
        }

        auto reads_writes(TaskHeadT::Views const & views) -> TInlineTask &
            requires(HAS_HEAD)
        {
            return value._process_th_views(views, TaskAccessType::READ_WRITE, value._internal._default_stage);
        }

        auto reads_writes_concurrent(TaskHeadT::Views const & views) -> TInlineTask &
            requires(HAS_HEAD)
        {
            return value._process_th_views(views, TaskAccessType::READ_WRITE_CONCURRENT, value._internal._default_stage);
        }

        auto samples(TaskHeadT::Views const & views) -> TInlineTask &
            requires(HAS_HEAD)
        {
            return value._process_th_views(views, TaskAccessType::SAMPLED, value._internal._default_stage);
        }

        auto head_views(TaskHeadT::Views const & views) -> TInlineTask &
            requires(HAS_HEAD)
        {
            return value._process_th_views(views, TaskAccessType::NONE, TaskStage::NONE, true);
        }

        // overloads for stage:

        auto reads(TaskStage stage, TaskHeadT::Views const & views) -> TInlineTask &
            requires(HAS_HEAD)
        {
            return value._process_th_views(views, TaskAccessType::READ, stage);
        }

        auto writes(TaskStage stage, TaskHeadT::Views const & views) -> TInlineTask &
            requires(HAS_HEAD)
        {
            return value._process_th_views(views, TaskAccessType::WRITE, stage);
        }

        auto writes_concurrent(TaskStage stage, TaskHeadT::Views const & views) -> TInlineTask &
            requires(HAS_HEAD)
        {
            return value._process_th_views(views, TaskAccessType::WRITE_CONCURRENT, stage);
        }

        auto reads_writes(TaskStage stage, TaskHeadT::Views const & views) -> TInlineTask &
            requires(HAS_HEAD)
        {
            return value._process_th_views(views, TaskAccessType::READ_WRITE, stage);
        }

        auto reads_writes_concurrent(TaskStage stage, TaskHeadT::Views const & views) -> TInlineTask &
            requires(HAS_HEAD)
        {
            return value._process_th_views(views, TaskAccessType::READ_WRITE_CONCURRENT, stage);
        }

        auto samples(TaskStage stage, TaskHeadT::Views const & views) -> TInlineTask &
            requires(HAS_HEAD)
        {
            return value._process_th_views(views, TaskAccessType::SAMPLED, stage);
        }

        auto uses(TaskStage stage, TaskHeadT::Views const & views) -> TInlineTask &
            requires(std::is_same_v<TaskHeadT, NoTaskHeadStruct>)
        {
            return value._process_th_views(views, TaskAccessType::NONE, stage);
        }

        // callback Interface:

        template <TaskCallbackLambda CallbackT>
        auto executes(CallbackT const & callback) -> TInlineTask &
        {
            this->value._internal._callback.store(callback);
            return *this;
        }

        template <typename... CallbackParamsT>
        auto executes(void (*callback)(TaskInterface, CallbackParamsT...), std::remove_reference_t<std::remove_const_t<CallbackParamsT>>... params) -> TInlineTask &
        {
            this->value._internal._callback.store([=](TaskInterface ti)
            {
                callback(ti, params...);
            });
            return *this;
        }

        operator TInlineTask<NoTaskHeadStruct>() const
            requires(HAS_HEAD)
        {
            return *reinterpret_cast<TInlineTask<NoTaskHeadStruct> const *>(this);
        }

        auto uses_queue(Queue queue) -> TInlineTask &
        {
            this->value._internal._queue = queue;
            return *this;
        }

      private:
        TInlineTask(Internal && internal)
        {
            this->value._internal = std::move(internal);
        }
    };

    using InlineTask = TInlineTask<NoTaskHeadStruct>;
    using Task = TInlineTask<NoTaskHeadStruct>;
    
    template <typename HeadInfoT>
    auto HeadTask(std::string_view name = {}) -> TInlineTask<HeadInfoT>
    {
        return daxa::Task(name, HeadInfoT::TYPE).uses_head<HeadInfoT>();
    }

    inline auto RasterTask(std::string_view name) -> Task
    {
        return Task::Raster(name);
    }

    inline auto ComputeTask(std::string_view name) -> Task
    {
        return Task::Compute(name);
    }

    inline auto RayTracingTask(std::string_view name) -> Task
    {
        return Task::RayTracing(name);
    }

    inline auto TransferTask(std::string_view name) -> Task
    {
        return Task::Transfer(name);
    }

    struct TaskBufferClearInfo
    {
        TaskBufferView buffer = {};
        u64 offset = {};
        u64 size = ~0ull; // default clears all
        u32 clear_value = {};
        Queue queue = QUEUE_MAIN;
        std::string_view name = {};
    };

    struct TaskImageClearInfo
    {
        TaskImageView view = {};
        ClearValue clear_value = std::array{0u, 0u, 0u, 0u};
        Queue queue = QUEUE_MAIN;
        std::string_view name = {};
    };

    struct TaskBufferCopyInfo
    {
        TaskBufferView src = {};
        TaskBufferView dst = {};
        Queue queue = QUEUE_MAIN;
        std::string_view name = {};
    };

    struct TaskImageCopyInfo
    {
        TaskImageView src = {};
        TaskImageView dst = {};
        Queue queue = QUEUE_MAIN;
        std::string_view name = {};
    };

    struct ImplTaskGraph;

    using OpaqueTaskPtr = std::unique_ptr<void, void (*)(void *)>;
    using OpaqueTaskCallback = void (*)(void *, TaskInterface &);

    struct TaskGraph : ManagedPtr<TaskGraph, ImplTaskGraph *>
    {
        TaskGraph() = default;

        DAXA_EXPORT_CXX TaskGraph(TaskGraphInfo const & info);
        DAXA_EXPORT_CXX ~TaskGraph();

        DAXA_EXPORT_CXX void use_persistent_buffer(TaskBuffer const & buffer);
        DAXA_EXPORT_CXX void use_persistent_blas(TaskBlas const & blas);
        DAXA_EXPORT_CXX void use_persistent_tlas(TaskTlas const & tlas);
        DAXA_EXPORT_CXX void use_persistent_image(TaskImage const & image);

        DAXA_EXPORT_CXX auto create_transient_buffer(TaskTransientBufferInfo info) -> TaskBufferView;
        DAXA_EXPORT_CXX auto create_transient_tlas(TaskTransientTlasInfo info) -> TaskTlasView;
        DAXA_EXPORT_CXX auto create_transient_image(TaskTransientImageInfo info) -> TaskImageView;

        DAXA_EXPORT_CXX auto transient_buffer_info(TaskBufferView const & transient) -> TaskTransientBufferInfo const &;
        DAXA_EXPORT_CXX auto transient_tlas_info(TaskTlasView const & transient) -> TaskTransientTlasInfo const &;
        DAXA_EXPORT_CXX auto transient_image_info(TaskImageView const & transient) -> TaskTransientImageInfo const &;

        DAXA_EXPORT_CXX void clear_buffer(TaskBufferClearInfo const & info);
        DAXA_EXPORT_CXX void clear_image(TaskImageClearInfo const & info);

        DAXA_EXPORT_CXX void copy_buffer_to_buffer(TaskBufferCopyInfo const & info);
        DAXA_EXPORT_CXX void copy_image_to_image(TaskImageCopyInfo const & info);

        template <typename TTask>
            requires std::is_base_of_v<IPartialTask, TTask> && std::is_trivially_copy_constructible_v<TTask>
        void add_task(TTask const & task)
        {
            using NoRefTTask = std::remove_reference_t<TTask>;
            static constexpr auto const & ATTACHMENTS = NoRefTTask::AT._internal.value;

            // Convert, allocate and copy attachments
            std::array<TaskAttachmentInfo, NoRefTTask::Info::ATTACHMENT_COUNT> converted_attachments = {};
            auto default_stage = task_type_default_stage(NoRefTTask::Info::TYPE);
            auto view_array = task.views.convert_to_array();
            for (u32 i = 0; i < NoRefTTask::Info::ATTACHMENT_COUNT; ++i)
            {
                converted_attachments[i] = detail::convert_to_task_attachment_info(ATTACHMENTS[i], view_array[i], default_stage);
            }
            auto attachment_memory = std::span{ 
                reinterpret_cast<TaskAttachmentInfo *>(
                    allocate_task_memory(sizeof(TaskAttachmentInfo) * NoRefTTask::Info::ATTACHMENT_COUNT, alignof(TaskAttachmentInfo))
                ), 
                NoRefTTask::Info::ATTACHMENT_COUNT,
            };
            std::memcpy(attachment_memory.data(), converted_attachments.data(), sizeof(TaskAttachmentInfo) * NoRefTTask::Info::ATTACHMENT_COUNT);

            // Allocate and assign task callback memory
            auto task_callback_memory = reinterpret_cast<u64*>(allocate_task_memory(sizeof(NoRefTTask), alignof(NoRefTTask)));
            std::memcpy(task_callback_memory, &task, sizeof(NoRefTTask));

            // Create callback adapter
            auto task_callback = [](daxa::TaskInterface ti, void * v)
            {
                reinterpret_cast<NoRefTTask*>(v)->callback(ti);
            };

            u32 asb_size = detail::get_asb_size_and_alignment(converted_attachments).size;
            u32 asb_align = detail::get_asb_size_and_alignment(converted_attachments).alignment;
            TaskType task_type = TTask::Info::TYPE;
            std::string_view name = TTask::Info::NAME;
            add_task(
                task_callback, task_callback_memory,
                attachment_memory, asb_size, asb_align, task_type, name, QUEUE_MAIN);
        }
        template <typename TaskHeadType>
        void add_task(TInlineTask<TaskHeadType> const & inline_task)
        {
            DAXA_DBG_ASSERT_TRUE_M(static_cast<bool>(inline_task.value._internal._callback), "Detected empty callback on inline task!");

            auto task_callback = inline_task.value._internal._callback.callback;

            // Allocate and assign task callback memory
            auto task_callback_memory = reinterpret_cast<u64*>(allocate_task_memory(inline_task.value._internal._callback.size, alignof(u64)));
            std::memcpy(task_callback_memory, inline_task.value._internal._callback.data.data(), inline_task.value._internal._callback.size);

            // Allocate and copy attachments into smaller memory section
            usize const attachment_count = inline_task.value._internal._attachments.size();
            auto attachments = std::span{reinterpret_cast<TaskAttachmentInfo *>(allocate_task_memory(sizeof(TaskAttachmentInfo) * attachment_count, alignof(TaskAttachmentInfo))), attachment_count};
            std::memcpy(attachments.data(), inline_task.value._internal._attachments.data(), sizeof(TaskAttachmentInfo) * attachments.size());

            u32 asb_size = detail::get_asb_size_and_alignment(attachments).size;
            u32 asb_align = detail::get_asb_size_and_alignment(attachments).alignment;
            TaskType task_type = inline_task.value._internal._task_type;
            std::string_view name = inline_task.value._internal._name;
            add_task(
                task_callback, task_callback_memory,
                attachments, asb_size, asb_align, task_type, name, inline_task.value._internal._queue);
        }

#if !DAXA_REMOVE_DEPRECATED
        [[deprecated("Use add_task(T && inline_task) instead, API:3.1")]] void add_task(InlineTaskInfo const & inline_task_info)
        {
            add_task(InlineTask{inline_task_info});
        }
#endif

        DAXA_EXPORT_CXX void conditional(TaskGraphConditionalInfo const & conditional_info);
        DAXA_EXPORT_CXX void submit(TaskSubmitInfo const & info);
        DAXA_EXPORT_CXX void present(TaskPresentInfo const & info);

        // TODO: make move only. Return ExecutableTaskGraph.
        DAXA_EXPORT_CXX void complete(TaskCompleteInfo const & info);

        DAXA_EXPORT_CXX void execute(ExecutionInfo const & info);

        DAXA_EXPORT_CXX auto get_debug_string() -> std::string;
        DAXA_EXPORT_CXX auto get_transient_memory_size() -> usize;

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        DAXA_EXPORT_CXX static auto inc_refcnt(ImplHandle const * object) -> u64;
        DAXA_EXPORT_CXX static auto dec_refcnt(ImplHandle const * object) -> u64;

      private:
        DAXA_EXPORT_CXX void add_task(
            void (*task_callback)(daxa::TaskInterface, void*),
            u64* task_callback_memory,
            std::span<TaskAttachmentInfo> attachments,
            u32 attachment_shader_blob_size,
            u32 attachment_shader_blob_alignment,
            TaskType task_type,
            std::string_view name,
            Queue queue);

        DAXA_EXPORT_CXX auto allocate_task_memory(usize size, usize align) -> void *;
    };
} // namespace daxa
