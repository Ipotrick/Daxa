#pragma once

#include <daxa/daxa.hpp>
#include <functional>
#include <memory>

#include "task_graph_types.hpp"

#if !DAXA_BUILT_WITH_UTILS_TASK_GRAPH
#error "[build error] You must build Daxa with the DAXA_ENABLE_UTILS_TASK_GRAPH CMake option enabled"
#endif

#if DAXA_BUILT_WITH_UTILS_IMGUI
#include <daxa/utils/imgui.hpp>
#include <filesystem>
#include <optional>
#endif

namespace daxa
{
    struct TaskGraph;
    struct Device;
    struct CommandSubmitInfo;
    struct PresentInfo;

    DAXA_EXPORT_CXX auto error_message_unassigned_buffer_view(std::string_view task_name, std::string_view attachment_name) -> std::string;
    DAXA_EXPORT_CXX auto error_message_unassigned_image_view(std::string_view task_name, std::string_view attachment_name) -> std::string;
    DAXA_EXPORT_CXX auto error_message_unassigned_tlas_view(std::string_view task_name, std::string_view attachment_name) -> std::string;
    DAXA_EXPORT_CXX auto error_message_unassigned_blas_view(std::string_view task_name, std::string_view attachment_name) -> std::string;
    DAXA_EXPORT_CXX auto error_message_no_access_sage(std::string_view task_name, std::string_view attachment_name, TaskAccess task_access) -> std::string;

    enum struct TaskResourceLifetimeType
    {
        TRANSIENT,
        PERSISTENT,
        PERSISTENT_DOUBLE_BUFFER,
        EXTERNAL,
    };

    [[nodiscard]] DAXA_EXPORT_CXX auto to_string(TaskResourceLifetimeType lifetime_type) -> std::string_view;

    struct TaskBufferInfo
    {
        u64 size = {};
        TaskResourceLifetimeType lifetime_type = TaskResourceLifetimeType::TRANSIENT;
        std::string_view name = {};
    };

    using TaskTlasInfo = TaskBufferInfo;

    struct TaskImageInfo
    {
        u32 dimensions = 2;
        Format format = Format::R8G8B8A8_UNORM;
        Extent3D size = {0, 0, 0};
        u32 mip_level_count = 1;
        u32 array_layer_count = 1;
        u32 sample_count = 1;
        TaskResourceLifetimeType lifetime_type = TaskResourceLifetimeType::TRANSIENT;
        std::string_view name = {};
    };

    struct TaskGraphInfo
    {
        Device device = {};
        /// @brief  Optionally the user can provide a swapchain. This enables the use of present.
        std::optional<Swapchain> swapchain = {};
        /// @brief  Task reordering can drastically improve performance,
        ///         yet is it also nice to have sequential callback execution.
        bool reorder_tasks = true;
        /// @brief  Reorder tasks to reduce total transient resource usage and improve memory aliasing.
        ///         Generally moves tasks touching similar resources closer together.
        ///         Typically moves all clears into batches close to the first use.
        bool optimize_transient_lifetimes = true;
        /// @brief  Allows task graph to alias transient resources memory (ofc only when that wont break the program)
        bool alias_transients = {};
        /// @brief  Task graph will put performance markers that are used by profilers like nsight around each tasks execution by default.
        bool enable_command_labels = true;
        std::array<f32, 4> task_graph_label_color = {0.463f, 0.333f, 0.671f, 1.0f};
        std::array<f32, 4> task_batch_label_color = {0.563f, 0.433f, 0.771f, 1.0f};
        std::array<f32, 4> task_label_color = {0.663f, 0.533f, 0.871f, 1.0f};
        /// @brief  AMD gpus of the generations RDNA3 and RDNA4 have hardware bugs that make image barriers still useful for cache flushes.
        ///         This boolean makes task graph insert image barriers for image sync instead of global barriers to help the drivers out.
        bool amd_rdna3_4_image_barrier_fix = true;
        /// @brief  Sets the size of the linear allocator of device local, host visible memory used by the linear staging allocator.
        ///         This memory is used internally as well as by tasks via the TaskInterface::get_allocator().
        ///         Setting the size to 0, disables a few task list features but also eliminates the memory allocation.
        u32 staging_memory_pool_size = 1u << 17u; // 128kib
        /// @brief  CPU Memory allocated for task data
        u32 task_memory_pool_size = 1u << 19u; // 512kib
        // Useful for debugging tools that are invisible to the graph.
        ImageUsageFlags additional_image_usage_flags = {};
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
    
    struct TaskGraphDebugUi;

    struct ExecutionInfo
    {
        TaskGraphDebugUi * debug_ui = {};
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
    concept TaskCallbackLambda = std::is_copy_constructible_v<T> && std::is_trivially_destructible_v<T> && sizeof(T) <= MAX_TASK_CALLBACK_DATA_SIZE && alignof(T) <= alignof(daxa::u64);

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
                TFunc & f_local = *static_cast<TFunc*>(v);
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

    inline namespace detail
    {
        constexpr inline auto replace_joker_stage(TaskStages stage, TaskStages default_stage) -> TaskStages
        {
            if ((stage & TaskStages::JOKER) != TaskStages::NONE)
            {
                return (stage & ~TaskStages::JOKER) | default_stage;
            }
            return stage;
        }

        template <typename TaskHeadAttachmentDeclT>
        auto complete_head_attachment_info(TaskHeadAttachmentDeclT const & attachment_decl, TaskViewVariant const & view, TaskStages default_stage, std::string_view task_name) -> TaskAttachmentInfo
        {
            TaskAttachmentInfo ret = attachment_decl;
            DAXA_DBG_ASSERT_TRUE_M(((ret.value.common.task_access.stage & TaskStages::JOKER) == TaskStages::NONE) || (default_stage != TaskStages::NONE), error_message_no_access_sage(task_name, ret.value.common.name, ret.value.common.task_access));
            ret.value.common.task_access.stage = replace_joker_stage(ret.value.common.task_access.stage, default_stage);

            if (auto * buffer_ptr = get_if<TaskBufferView>(&view))
            {
                ret.value.buffer.view = *buffer_ptr;
            }
            else if (auto * blas_ptr = get_if<TaskBlasView>(&view))
            {
                ret.value.blas.view = *blas_ptr;
            }
            else if (auto * tlas_ptr = get_if<TaskTlasView>(&view))
            {
                ret.value.tlas.view = *tlas_ptr;
            }
            else if (auto * image_ptr = get_if<TaskImageView>(&view))
            {
                ret.value.image.view = *image_ptr;
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

    template <typename TaskHeadT>
    struct TInlineTask
    {
        // construction interface:

        TInlineTask()
        {
            value = {}; // Prevents ICE
        }
        
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
            value.~InternalValue<Allow::NONE, TaskStages::NONE>();
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
            TaskStages _default_stage = TaskStages::ANY_COMMAND;
            Queue _queue = QUEUE_MAIN;

            void _process_parameter(TaskStages &, TaskAccessType, ImageViewType &, std::string_view &, TaskStages) {}

            void _process_parameter(TaskStages & stage, TaskAccessType type, ImageViewType &, char const * & name, TaskBufferViewOrTaskBuffer auto param)
            {
                auto info = TaskAttachmentInfo{};
                info.type = daxa::TaskAttachmentType::BUFFER;
                info.value.buffer = TaskBufferAttachmentInfo{
                    .name = name,
                    .task_access = TaskAccess{stage, type},
                    .view = param,
                };
                _attachments.push_back(info);
            }
            void _process_parameter(TaskStages & stage, TaskAccessType type, ImageViewType &, char const * & name, TaskBlasViewOrTaskBlas auto param)
            {
                auto info = TaskAttachmentInfo{};
                info.type = daxa::TaskAttachmentType::BLAS;
                info.value.blas = TaskBlasAttachmentInfo{
                    .name = name,
                    .task_access = TaskAccess{stage, type},
                    .view = param,
                };
                _attachments.push_back(info);
            }
            void _process_parameter(TaskStages & stage, TaskAccessType type, ImageViewType &, char const * & name, TaskTlasViewOrTaskTlas auto param)
            {
                auto info = TaskAttachmentInfo{};
                info.type = daxa::TaskAttachmentType::TLAS;
                info.value.tlas = TaskTlasAttachmentInfo{
                    .name = name,
                    .task_access = TaskAccess{stage, type},
                    .view = param,
                };
                _attachments.push_back(info);
            }
            void _process_parameter(TaskStages & stage, TaskAccessType type, ImageViewType & view_override, char const * & name, TaskImageViewOrTaskImage auto param)
            {
                auto info = TaskAttachmentInfo{};
                info.type = daxa::TaskAttachmentType::IMAGE;
                info.value.image = TaskImageAttachmentInfo{
                    .name = name,
                    .task_access = TaskAccess{stage, type},
                    .view = param,
                    .view_type = view_override,
                };
                _attachments.push_back(info);
            }
            void _process_parameter(TaskStages &, TaskAccessType, ImageViewType & view_override, char const * &, ImageViewType param)
            {
                view_override = param;
            }
            void _process_parameter(TaskStages &, TaskAccessType, ImageViewType &, char const * & name, char const * & param)
            {
                name = param;
            }
            void _process_parameter(TaskStages & stages, TaskAccessType, ImageViewType &, char const * &, TaskStages param)
            {
                stages = param;
            }
        };
        template <Allow ALLOWED_ACCESS, TaskStages STAGE = TaskStages::NONE>
        struct InternalValue
        {
            Internal _internal = {};

            // inline attachment interface:

            template <AttachmentParamBasic... TParams>
            auto _process_parameters(TaskAccessType access_type, TaskStages set_stage, TParams... v) -> TInlineTask &
            {
                ImageViewType view_override = ImageViewType::MAX_ENUM;
                char const * attachment_name = "unnamed attachment";
                TaskStages stage_override = set_stage;
                (_internal._process_parameter(stage_override, access_type, view_override, attachment_name, v), ...);
                return *reinterpret_cast<TInlineTask *>(this);
            }

            template <AttachmentParamBasic... TParams>
            auto reads(TParams... v) -> TInlineTask &
                requires((ALLOWED_ACCESS & Allow::READ) != 0)
            {
                return _process_parameters(TaskAccessType::READ, STAGE, v...);
            }

            template <AttachmentParamBasic... TParams>
            auto writes(TParams... v) -> TInlineTask &
                requires((ALLOWED_ACCESS & Allow::WRITE) != 0)
            {
                return _process_parameters(TaskAccessType::WRITE, STAGE, v...);
            }

            template <AttachmentParamBasic... TParams>
            auto writes_concurrent(TParams... v) -> TInlineTask &
                requires((ALLOWED_ACCESS & Allow::WRITE) != 0)
            {
                return _process_parameters(TaskAccessType::WRITE_CONCURRENT, STAGE, v...);
            }

            template <AttachmentParamBasic... TParams>
            auto reads_writes(TParams... v) -> TInlineTask &
                requires((ALLOWED_ACCESS & Allow::READ_WRITE) != 0)
            {
                return _process_parameters(TaskAccessType::READ_WRITE, STAGE, v...);
            }

            template <AttachmentParamBasic... TParams>
            auto reads_writes_concurrent(TParams... v) -> TInlineTask &
                requires((ALLOWED_ACCESS & Allow::READ_WRITE) != 0)
            {
                return _process_parameters(TaskAccessType::READ_WRITE_CONCURRENT, STAGE, v...);
            }

            template <AttachmentParamSampled... TParams>
            auto samples(TParams... v) -> TInlineTask &
                requires((ALLOWED_ACCESS & Allow::SAMPLED) != 0)
            {
                return _process_parameters(TaskAccessType::SAMPLED, STAGE, v...);
            }

            // task head attachment interface:

            static inline constexpr bool HAS_HEAD = !std::is_same_v<TaskHeadT, NoTaskHeadStruct>;

            auto _process_th_views(TaskHeadT::Views const & views, TaskAccessType access = TaskAccessType::NONE, TaskStages override_stage = TaskStages::NONE, bool keep_access = false) -> TInlineTask &
            {
                constexpr bool IS_GENERIC_CALL = STAGE == TaskStages::NONE;
                if constexpr (!IS_GENERIC_CALL)
                {
                    override_stage = STAGE;
                }

                TaskStages default_stage = task_type_default_stage(_internal._task_type);

                auto av = views.convert_to_array(); // converts views to flat array
                for (u32 i = 0; i < TaskHeadT::ATTACHMENT_COUNT; ++i)
                {
                    TaskAttachmentInfo & attach = _internal._attachments[i];

                    if (!keep_access)
                    {
                        attach.value.common.task_access.stage = override_stage;
                        attach.value.common.task_access.type = access;
                    }
                    DAXA_DBG_ASSERT_TRUE_M(((attach.value.common.task_access.stage & TaskStages::JOKER) == TaskStages::NONE) || (default_stage != TaskStages::NONE), error_message_no_access_sage(_internal._name, attach.value.common.name, attach.value.common.task_access));
                    attach.value.common.task_access.stage = replace_joker_stage(attach.value.common.task_access.stage, default_stage);

                    if (daxa::holds_alternative<TaskViewUndefined>(av[i])) // skip unassigned views
                    {
                    }
                    else if (TaskBufferView * buffer_ptr = daxa::get_if<TaskBufferView>(&av[i]); buffer_ptr != nullptr)
                    {
                        DAXA_DBG_ASSERT_TRUE_M(!buffer_ptr->is_empty(), error_message_unassigned_buffer_view(_internal._name, attach.value.buffer.name));
                        attach.value.buffer.view = *buffer_ptr;
                    }
                    else if (TaskImageView * image_ptr = daxa::get_if<TaskImageView>(&av[i]); image_ptr != nullptr)
                    {
                        DAXA_DBG_ASSERT_TRUE_M(!image_ptr->is_empty(), error_message_unassigned_image_view(_internal._name, attach.value.image.name));
                        attach.value.image.view = *image_ptr;
                    }
                    else if (TaskBlasView * blas_ptr = daxa::get_if<TaskBlasView>(&av[i]); blas_ptr != nullptr)
                    {
                        DAXA_DBG_ASSERT_TRUE_M(!blas_ptr->is_empty(), error_message_unassigned_tlas_view(_internal._name, attach.value.blas.name));
                        attach.value.blas.view = *blas_ptr;
                    }
                    else if (TaskTlasView * tlas_ptr = daxa::get_if<TaskTlasView>(&av[i]); tlas_ptr != nullptr)
                    {
                        DAXA_DBG_ASSERT_TRUE_M(!tlas_ptr->is_empty(), error_message_unassigned_blas_view(_internal._name, attach.value.tlas.name));
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
            InternalValue<Allow::NONE, TaskStages::NONE> value = {};
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStages::VERTEX_SHADER> vertex_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStages::TESSELLATION_CONTROL_SHADER> tesselation_control_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStages::TESSELLATION_EVALUATION_SHADER> tesselation_evaluation_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStages::GEOMETRY_SHADER> geometry_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStages::FRAGMENT_SHADER> fragment_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStages::COMPUTE_SHADER> compute_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStages::RAY_TRACING_SHADER> ray_tracing_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStages::TASK_SHADER> task_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStages::MESH_SHADER> mesh_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStages::PRE_RASTERIZATION_SHADERS> pre_rasterization_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStages::RASTER_SHADER> raster_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStages::SHADER> shader;
            InternalValue<Allow(Allow::WRITE | Allow::READ_WRITE), TaskStages::COLOR_ATTACHMENT> color_attachment;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStages::DEPTH_STENCIL_ATTACHMENT> depth_stencil_attachment;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE), TaskStages::RESOLVE> resolve;
            InternalValue<Allow::READ, TaskStages::INDIRECT_COMMAND_READ> indirect_cmd;
            InternalValue<Allow::READ, TaskStages::INDEX_INPUT> index_input;
            InternalValue<Allow(Allow::READ | Allow::WRITE), TaskStages::TRANSFER> transfer;
            InternalValue<Allow(Allow::READ | Allow::WRITE), TaskStages::HOST> host;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE), TaskStages::AS_BUILD> acceleration_structure_build;
        };

      private:
#ifndef __clang__ // MSVC STL does not implement these for clang :/
        // Per c++ spec, it is only legal to access multiple union members at the same time IF AND ONLY IF:
        // * all types in the union are standard layout
        // * all types in the union share a common initial sequence for all members accessed
        //   * or all types within the union are layout compatible
        using TEST_TYPE_A = InternalValue<Allow::NONE, TaskStages::NONE>;
        using TEST_TYPE_B = InternalValue<Allow::NONE, TaskStages::NONE>;
        static constexpr inline bool UNION_MEMBERS_ARE_STANDART_LAYOUT = std::is_standard_layout_v<TEST_TYPE_A> && std::is_standard_layout_v<TEST_TYPE_B>;
        static constexpr inline bool UNION_MEMBERS_ARE_LAYOUT_COMPATIBLE = std::is_layout_compatible_v<TEST_TYPE_A, TEST_TYPE_B>;
        static_assert(UNION_MEMBERS_ARE_STANDART_LAYOUT && UNION_MEMBERS_ARE_LAYOUT_COMPATIBLE);
#endif

      public:
        // untyped inline attachments interface:

        template <AttachmentParamBasic... TParams>
        auto reads(TParams... v) -> TInlineTask & { return value._process_parameters(TaskAccessType::READ, value._internal._default_stage, v...); }

        template <AttachmentParamBasic... TParams>
        auto writes(TParams... v) -> TInlineTask & { return value._process_parameters(TaskAccessType::WRITE, value._internal._default_stage, v...); }

        template <AttachmentParamBasic... TParams>
        auto writes_concurrent(TParams... v) -> TInlineTask & { return value._process_parameters(TaskAccessType::WRITE_CONCURRENT, value._internal._default_stage, v...); }

        template <AttachmentParamBasic... TParams>
        auto reads_writes(TParams... v) -> TInlineTask & { return value._process_parameters(TaskAccessType::READ_WRITE, value._internal._default_stage, v...); }

        template <AttachmentParamBasic... TParams>
        auto reads_writes_concurrent(TParams... v) -> TInlineTask & { return value._process_parameters(TaskAccessType::READ_WRITE_CONCURRENT, value._internal._default_stage, v...); }

        template <AttachmentParamSampled... TParams>
        auto samples(TParams... v) -> TInlineTask & { return value._process_parameters(TaskAccessType::SAMPLED, value._internal._default_stage, v...); }


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

            TaskStages default_stage = value._internal._default_stage != TaskStages::NONE ? value._internal._default_stage : head_default_stage;
            for (u32 i = 0; i < NewTaskHeadInfo::ATTACHMENT_COUNT; ++i)
            {
                ret.value._internal._attachments.push_back(detail::complete_head_attachment_info(NewTaskHeadInfo::AT._internal.value[i], {}, default_stage, value._internal._name));
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
            return value._process_th_views(views, TaskAccessType::NONE, TaskStages::NONE, true);
        }

        // overloads for stage:

        auto reads(TaskStages stage, TaskHeadT::Views const & views) -> TInlineTask &
            requires(HAS_HEAD)
        {
            return value._process_th_views(views, TaskAccessType::READ, stage);
        }

        auto writes(TaskStages stage, TaskHeadT::Views const & views) -> TInlineTask &
            requires(HAS_HEAD)
        {
            return value._process_th_views(views, TaskAccessType::WRITE, stage);
        }

        auto writes_concurrent(TaskStages stage, TaskHeadT::Views const & views) -> TInlineTask &
            requires(HAS_HEAD)
        {
            return value._process_th_views(views, TaskAccessType::WRITE_CONCURRENT, stage);
        }

        auto reads_writes(TaskStages stage, TaskHeadT::Views const & views) -> TInlineTask &
            requires(HAS_HEAD)
        {
            return value._process_th_views(views, TaskAccessType::READ_WRITE, stage);
        }

        auto reads_writes_concurrent(TaskStages stage, TaskHeadT::Views const & views) -> TInlineTask &
            requires(HAS_HEAD)
        {
            return value._process_th_views(views, TaskAccessType::READ_WRITE_CONCURRENT, stage);
        }

        auto samples(TaskStages stage, TaskHeadT::Views const & views) -> TInlineTask &
            requires(HAS_HEAD)
        {
            return value._process_th_views(views, TaskAccessType::SAMPLED, stage);
        }

        auto uses(TaskStages stage, TaskHeadT::Views const & views) -> TInlineTask &
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
        TaskBufferView src_buffer = {};
        TaskBufferView dst_buffer = {};
        Queue queue = QUEUE_MAIN;
        std::string_view name = {};
    };

    struct TaskImageCopyInfo
    {
        TaskImageView src_image = {};
        TaskImageView dst_image = {};
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

        DAXA_EXPORT_CXX auto register_buffer(ExternalTaskBuffer const & buffer) -> TaskBufferView;
        DAXA_EXPORT_CXX auto register_blas(ExternalTaskBlas const & blas) -> TaskBlasView;
        DAXA_EXPORT_CXX auto register_tlas(ExternalTaskTlas const & tlas) -> TaskTlasView;
        DAXA_EXPORT_CXX auto register_image(ExternalTaskImage const & image) -> TaskImageView;

        DAXA_EXPORT_CXX auto create_task_buffer(TaskBufferInfo info) -> TaskBufferView;
        DAXA_EXPORT_CXX auto create_task_tlas(TaskTlasInfo info) -> TaskTlasView;
        DAXA_EXPORT_CXX auto create_task_image(TaskImageInfo info) -> TaskImageView;

        DAXA_EXPORT_CXX auto task_buffer_info(TaskBufferView const & task_buffer) -> TaskBufferInfo;
        DAXA_EXPORT_CXX auto task_tlas_info(TaskTlasView const & task_tlas) -> TaskTlasInfo;
        DAXA_EXPORT_CXX auto task_image_info(TaskImageView const & task_image) -> TaskImageInfo;

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
                converted_attachments[i] = detail::complete_head_attachment_info(ATTACHMENTS[i], view_array[i], default_stage, TTask::Info::NAME);
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

        DAXA_EXPORT_CXX void submit(TaskSubmitInfo const & info);
        DAXA_EXPORT_CXX void present(TaskPresentInfo const & info);

        // TODO: make move only. Return ExecutableTaskGraph.
        DAXA_EXPORT_CXX void complete(TaskCompleteInfo const & info);

        DAXA_EXPORT_CXX void execute(ExecutionInfo const & info);

        // Alternative to clear_buffer and clear_image, these functions are only for persistent resources and they can only be called AFTER recording.
        // All persistent resources are automatically cleared to zero before the first graph execution.
        DAXA_EXPORT_CXX void request_persistent_buffer_clear(TaskBufferView const & task_buffer);
        DAXA_EXPORT_CXX void request_persistent_image_clear(TaskImageView const & task_image);

        DAXA_EXPORT_CXX auto get_resource_memory_block_size() -> usize;

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
        
#if DAXA_BUILT_WITH_UTILS_IMGUI
    struct ImplTaskGraphDebugUi;

    struct TaskGraphDebugUiInfo 
    {
        Device device = {};
        ImGuiRenderer imgui_renderer = {};
        std::optional<std::filesystem::path> buffer_layout_cache_folder = {};
    };

    struct TaskGraphDebugUi : ManagedPtr<TaskGraphDebugUi, ImplTaskGraphDebugUi *>
    {
        DAXA_EXPORT_CXX TaskGraphDebugUi() = default;
        DAXA_EXPORT_CXX TaskGraphDebugUi(TaskGraphDebugUiInfo const & info);

        DAXA_EXPORT_CXX auto update(TaskGraph & task_graph) -> bool;

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        DAXA_EXPORT_CXX static auto inc_refcnt(ImplHandle const * object) -> u64;
        DAXA_EXPORT_CXX static auto dec_refcnt(ImplHandle const * object) -> u64;
    };
#endif
} // namespace daxa
