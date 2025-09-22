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
        std::string name = {};
    };

    struct TaskTransientImageInfo
    {
        u32 dimensions = 2;
        Format format = Format::R8G8B8A8_UNORM;
        Extent3D size = {0, 0, 0};
        u32 mip_level_count = 1;
        u32 array_layer_count = 1;
        u32 sample_count = 1;
        std::string name = {};
    };

    struct TaskImageAliasInfo
    {
        std::string alias = {};
        Variant<TaskImageView, std::string> aliased_image = {};
        u32 base_mip_level_offset = {};
        u32 base_array_layer_offset = {};
    };

    struct TaskBufferAliasInfo
    {
        std::string alias = {};
        Variant<TaskBufferView, std::string> aliased_buffer = {};
    };

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
        bool enable_command_labels = false;
        std::array<f32, 4> task_graph_label_color = {0.463f, 0.333f, 0.671f, 1.0f};
        std::array<f32, 4> task_batch_label_color = {0.563f, 0.433f, 0.771f, 1.0f};
        std::array<f32, 4> task_label_color = {0.663f, 0.533f, 0.871f, 1.0f};
        /// @brief  Records debug information about the execution if enabled. This string is retrievable with the function get_debug_string.
        bool record_debug_information = {};
        /// @brief  Sets the size of the linear allocator of device local, host visible memory used by the linear staging allocator.
        ///         This memory is used internally as well as by tasks via the TaskInterface::get_allocator().
        ///         Setting the size to 0, disables a few task list features but also eliminates the memory allocation.
        u32 staging_memory_pool_size = 262'144; // 2^16 bytes.
        // Useful for debugging tools that are invisible to the graph.
        daxa::ImageUsageFlags additional_transient_image_usage_flags = {};
        // Useful for reflection/ debugging.
        std::function<void(daxa::TaskInterface)> pre_task_callback = {};
        std::function<void(daxa::TaskInterface)> post_task_callback = {};
        std::string name = {};
    };

    struct TaskSubmitInfo
    {
        PipelineStageFlags * additional_src_stages = {};
        std::vector<ExecutableCommandList> * additional_command_lists = {};
        std::vector<BinarySemaphore> * additional_wait_binary_semaphores = {};
        std::vector<BinarySemaphore> * additional_signal_binary_semaphores = {};
        std::vector<std::pair<TimelineSemaphore, u64>> * additional_wait_timeline_semaphores = {};
        std::vector<std::pair<TimelineSemaphore, u64>> * additional_signal_timeline_semaphores = {};
    };

    struct TaskPresentInfo
    {
        std::vector<BinarySemaphore> * additional_binary_semaphores = {};
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

    struct InlineTaskInfo
    {
        TaskType task_type = TaskType::GENERAL;
        std::vector<TaskAttachmentInfo> attachments = {};
        std::function<void(TaskInterface)> task = {};
        char const * name = "unnamed";
    };

    struct InlineTask : ITask
    {
        InlineTask() = default;
        InlineTask(InlineTaskInfo const & info)
        {
            value = {}; // Prevents ICE
            value._internal._task_type = info.task_type;
            value._internal._attachments = info.attachments;
            value._internal._callback = info.task;
            value._internal._name = info.name;
        }
        InlineTask(char const * name, TaskType task_type = TaskType::GENERAL)
        {
            value._internal._name = name;
            value._internal._task_type = task_type;
        }
        static auto Raster(char const * name) -> InlineTask
        {
            return InlineTask(name, TaskType::RASTER);
        }
        static auto Compute(char const * name) -> InlineTask
        {
            return InlineTask(name, TaskType::COMPUTE);
        }
        static auto RayTracing(char const * name) -> InlineTask
        {
            return InlineTask(name, TaskType::RAY_TRACING);
        }
        static auto Transfer(char const * name) -> InlineTask
        {
            return InlineTask(name, TaskType::TRANSFER);
        }
        virtual ~InlineTask() override
        {
        }
        InlineTask(InlineTask && other)
        {
            this->value = {}; // Prevents ICE
            this->value._internal._attachments = std::move(other.value._internal._attachments);
            this->value._internal._callback = std::move(other.value._internal._callback);
            this->value._internal._name = std::move(other.value._internal._name);
            this->value._internal._task_type = std::move(other.value._internal._task_type);
            other.value._internal._attachments = {};
            other.value._internal._callback = {};
            other.value._internal._name = {};
            other.value._internal._task_type = {};
        }
        constexpr virtual auto attachments() -> std::span<TaskAttachmentInfo> override
        {
            return value._internal._attachments;
        }
        constexpr virtual auto attachments() const -> std::span<TaskAttachmentInfo const> override
        {
            return value._internal._attachments;
        }
        constexpr virtual auto name() const -> std::string_view override { return value._internal._name; };
        virtual void callback(TaskInterface ti) override
        {
            value._internal._callback(ti);
        }
        virtual auto task_type() const -> TaskType override { return value._internal._task_type; }

      private:
        enum Allow
        {
            NONE = 0,
            READ = (1 << 0),
            WRITE = (1 << 1),
            READ_WRITE = (1 << 2),
            SAMPLED = (1 << 5),
        };

      public:
        struct Internal
        {
            std::vector<TaskAttachmentInfo> _attachments = {};
            std::function<void(TaskInterface)> _callback = {};
            char const * _name = {};
            TaskType _task_type = TaskType::GENERAL;

            void _process_params(TaskStage stage, TaskAccessType type, ImageViewType & view_override, TaskBufferBlasTlasViewOrBufferBlasTlas auto param)
            {
                _attachments.push_back(inl_attachment(TaskAccess{stage, type}, param));
            }
            void _process_params(TaskStage stage, TaskAccessType type, ImageViewType & view_override, TaskImageViewOrTaskImage auto param)
            {
                _attachments.push_back(inl_attachment(TaskAccess{stage, type}, param, view_override));
            }
            void _process_params(TaskStage stage, TaskAccessType type, ImageViewType & view_override, ImageViewType param)
            {
                view_override = param;
            }
        };
        template <Allow ALLOWED_ACCESS, TaskStage STAGE = TaskStage::NONE>
        struct InternalValue
        {
            Internal _internal = {};

            template <TaskResourceViewOrResource... TResources>
            auto reads(TResources... v) -> InlineTask
                requires((ALLOWED_ACCESS & Allow::READ) != 0)
            {
                ImageViewType view_override = ImageViewType::MAX_ENUM;
                (_internal._process_params(STAGE, TaskAccessType::READ, view_override, v), ...);
                return InlineTask{std::move(_internal)};
            }
            template <TaskResourceViewOrResourceOrImageViewType... TResources>
            auto writes(TResources... v) -> InlineTask
                requires((ALLOWED_ACCESS & Allow::WRITE) != 0)
            {
                ImageViewType view_override = ImageViewType::MAX_ENUM;
                (_internal._process_params(STAGE, TaskAccessType::WRITE, view_override, v), ...);
                return InlineTask{std::move(_internal)};
            }
            template <TaskResourceViewOrResourceOrImageViewType... TResources>
            auto writes_concurrent(TResources... v) -> InlineTask
                requires((ALLOWED_ACCESS & Allow::WRITE) != 0)
            {
                ImageViewType view_override = ImageViewType::MAX_ENUM;
                (_internal._process_params(STAGE, TaskAccessType::WRITE_CONCURRENT, view_override, v), ...);
                return InlineTask{std::move(_internal)};
            }
            template <TaskResourceViewOrResourceOrImageViewType... TResources>
            auto reads_writes(TResources... v) -> InlineTask
                requires((ALLOWED_ACCESS & Allow::READ_WRITE) != 0)
            {
                ImageViewType view_override = ImageViewType::MAX_ENUM;
                (_internal._process_params(STAGE, TaskAccessType::READ_WRITE, view_override, v), ...);
                return InlineTask{std::move(_internal)};
            }
            template <TaskResourceViewOrResourceOrImageViewType... TResources>
            auto readwrites_concurrent(TResources... v) -> InlineTask
                requires((ALLOWED_ACCESS & Allow::READ_WRITE) != 0)
            {
                ImageViewType view_override = ImageViewType::MAX_ENUM;
                (_internal._process_params(STAGE, TaskAccessType::READ_WRITE_CONCURRENT, view_override, v), ...);
                return InlineTask{std::move(_internal)};
            }
            template <TaskImageViewOrTaskImageOrImageViewType... TResources>
                requires((ALLOWED_ACCESS & Allow::SAMPLED) != 0)
            auto samples(TResources... v) -> InlineTask
            {
                ImageViewType view_override = ImageViewType::MAX_ENUM;
                (_internal._process_params(STAGE, TaskAccessType::SAMPLED, view_override, v), ...);
                return InlineTask{std::move(_internal)};
            }
        };
        union
        {
            InternalValue<Allow::NONE, TaskStage::NONE> value = {};
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::VERTEX_SHADER> vs;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::VERTEX_SHADER> vertex_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::TESSELLATION_CONTROL_SHADER> tcs;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::TESSELLATION_CONTROL_SHADER> tesselation_control_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::TESSELLATION_EVALUATION_SHADER> tes;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::TESSELLATION_EVALUATION_SHADER> tesselation_evaluation_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::GEOMETRY_SHADER> gs;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::GEOMETRY_SHADER> geometry_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::FRAGMENT_SHADER> fs;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::FRAGMENT_SHADER> fragment_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::COMPUTE_SHADER> cs;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::COMPUTE_SHADER> compute_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::RAY_TRACING_SHADER> rts;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::RAY_TRACING_SHADER> ray_tracing_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::TASK_SHADER> ts;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::TASK_SHADER> task_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::MESH_SHADER> ms;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::MESH_SHADER> mesh_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::PRE_RASTERIZATION_SHADERS> prs;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::PRE_RASTERIZATION_SHADERS> pre_rasterization_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::RASTER_SHADER> rs;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::RASTER_SHADER> raster_shader;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::SHADER> s;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::SHADER> shader;
            InternalValue<Allow(Allow::WRITE | Allow::READ_WRITE), TaskStage::COLOR_ATTACHMENT> ca;
            InternalValue<Allow(Allow::WRITE | Allow::READ_WRITE), TaskStage::COLOR_ATTACHMENT> color_attachment;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::DEPTH_STENCIL_ATTACHMENT> dsa;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE | Allow::SAMPLED), TaskStage::DEPTH_STENCIL_ATTACHMENT> depth_stencil_attachment;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE), TaskStage::RESOLVE> resolve;
            InternalValue<Allow::READ, TaskStage::PRESENT> present;
            InternalValue<Allow::READ, TaskStage::INDIRECT_COMMAND> indirect_cmd;
            InternalValue<Allow::READ, TaskStage::INDEX_INPUT> index_input;
            InternalValue<Allow(Allow::READ | Allow::WRITE), TaskStage::TRANSFER> tf;
            InternalValue<Allow(Allow::READ | Allow::WRITE), TaskStage::TRANSFER> transfer;
            InternalValue<Allow(Allow::READ | Allow::WRITE), TaskStage::HOST> host;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE), TaskStage::AS_BUILD> asb;
            InternalValue<Allow(Allow::READ | Allow::WRITE | Allow::READ_WRITE), TaskStage::AS_BUILD> acceleration_structure_build;
        };

        template <TaskResourceViewOrResource... TResources>
        auto reads(TaskStage stage, TResources... v) -> InlineTask
        {
            ImageViewType view_override = ImageViewType::MAX_ENUM;
            (value._internal._process_params(stage, TaskAccessType::READ, view_override, v), ...);
            return InlineTask{std::move(value._internal)};
        }
        template <TaskResourceViewOrResourceOrImageViewType... TResources>
        auto writes(TaskStage stage, TResources... v) -> InlineTask
        {
            ImageViewType view_override = ImageViewType::MAX_ENUM;
            (value._internal._process_params(stage, TaskAccessType::WRITE, view_override, v), ...);
            return InlineTask{std::move(value._internal)};
        }
        template <TaskResourceViewOrResourceOrImageViewType... TResources>
        auto writes_concurrent(TaskStage stage, TResources... v) -> InlineTask
        {
            ImageViewType view_override = ImageViewType::MAX_ENUM;
            (value._internal._process_params(stage, TaskAccessType::WRITE_CONCURRENT, view_override, v), ...);
            return InlineTask{std::move(value._internal)};
        }
        template <TaskResourceViewOrResourceOrImageViewType... TResources>
        auto reads_writes(TaskStage stage, TResources... v) -> InlineTask
        {
            ImageViewType view_override = ImageViewType::MAX_ENUM;
            (value._internal._process_params(stage, TaskAccessType::READ_WRITE, view_override, v), ...);
            return InlineTask{std::move(value._internal)};
        }
        template <TaskResourceViewOrResourceOrImageViewType... TResources>
        auto readwrites_concurrent(TaskStage stage, TResources... v) -> InlineTask
        {
            ImageViewType view_override = ImageViewType::MAX_ENUM;
            (value._internal._process_params(stage, TaskAccessType::READ_WRITE_CONCURRENT, view_override, v), ...);
            return InlineTask{std::move(value._internal)};
        }
        template <TaskImageViewOrTaskImageOrImageViewType... TResources>
        auto samples(TaskStage stage, TResources... v) -> InlineTask
        {
            ImageViewType view_override = ImageViewType::MAX_ENUM;
            (value._internal._process_params(stage, TaskAccessType::SAMPLED, view_override, v), ...);
            return InlineTask{std::move(value._internal)};
        }

        template <TaskResourceViewOrResource... TResources>
        auto reads(TResources... v) -> InlineTask
        {
            return reads(TaskStage::NONE, v...);
        }
        template <TaskResourceViewOrResourceOrImageViewType... TResources>
        auto writes(TResources... v) -> InlineTask
        {
            return writes(TaskStage::NONE, v...);
        }
        template <TaskResourceViewOrResourceOrImageViewType... TResources>
        auto writes_concurrent(TResources... v) -> InlineTask
        {
            return writes_concurrent(TaskStage::NONE, v...);
        }
        template <TaskResourceViewOrResourceOrImageViewType... TResources>
        auto reads_writes(TResources... v) -> InlineTask
        {
            return reads_writes(TaskStage::NONE, v...);
        }
        template <TaskResourceViewOrResourceOrImageViewType... TResources>
        auto readwrites_concurrent(TResources... v) -> InlineTask
        {
            return readwrites_concurrent(TaskStage::NONE, v...);
        }
        template <TaskImageViewOrTaskImageOrImageViewType... TResources>
        auto samples(TResources... v) -> InlineTask
        {
            return samples(TaskStage::NONE, v...);
        }

        template <TaskResourceViewOrResource... TResources>
        auto uses(TaskAccess access, TResources... v) -> InlineTask
        {
            ImageViewType view_override = ImageViewType::MAX_ENUM;
            (value._internal._process_params(access.stage, access.type, view_override, v), ...);
            return InlineTask{std::move(value._internal)};
        }

        auto executes(std::function<void(TaskInterface)> const & c) && -> InlineTask
        {
            value._internal._callback = std::move(c);
            return std::move(*this);
        }

      private:
        InlineTask(Internal && internal)
        {
            this->value._internal = std::move(internal);
        }
    };

    template <typename TaskHeadTaskT>
    struct InlineTaskWithHead : TaskHeadTaskT
    {
        TaskHeadTaskT::AttachmentViews views = {};
        std::function<void(daxa::TaskInterface)> task = {};
        void callback(daxa::TaskInterface ti)
        {
            task(ti);
        }
    };

    struct TaskBufferClearInfo
    {
        TaskBufferView buffer = {};
        u64 offset = {};
        u64 size = ~0ull; // default clears all
        u32 clear_value = {};
        std::string_view name = {};
    };

    struct TaskImageClearInfo
    {
        TaskImageView view = {};
        ClearValue clear_value = std::array{0u, 0u, 0u, 0u};
        std::string_view name = {};
    };

    struct TaskBufferCopyInfo
    {
        TaskBufferView src = {};
        TaskBufferView dst = {};
        std::string_view name = {};
    };

    struct TaskImageCopyInfo
    {
        TaskImageView src = {};
        TaskImageView dst = {};
        std::string_view name = {};
    };

    struct ImplTaskGraph;

    struct TaskGraph : ManagedPtr<TaskGraph, ImplTaskGraph *>
    {
        TaskGraph() = default;

        DAXA_EXPORT_CXX TaskGraph(TaskGraphInfo const & info);
        DAXA_EXPORT_CXX ~TaskGraph();

        DAXA_EXPORT_CXX void use_persistent_buffer(TaskBuffer const & buffer);
        DAXA_EXPORT_CXX void use_persistent_blas(TaskBlas const & blas);
        DAXA_EXPORT_CXX void use_persistent_tlas(TaskTlas const & tlas);
        DAXA_EXPORT_CXX void use_persistent_image(TaskImage const & image);

        DAXA_EXPORT_CXX auto create_transient_buffer(TaskTransientBufferInfo const & info) -> TaskBufferView;
        DAXA_EXPORT_CXX auto create_transient_image(TaskTransientImageInfo const & info) -> TaskImageView;

        DAXA_EXPORT_CXX auto transient_buffer_info(TaskBufferView const & transient) -> TaskTransientBufferInfo const &;
        DAXA_EXPORT_CXX auto transient_image_info(TaskImageView const & transient) -> TaskTransientImageInfo const &;

        DAXA_EXPORT_CXX void clear_buffer(TaskBufferClearInfo const & info);
        DAXA_EXPORT_CXX void clear_image(TaskImageClearInfo const & info);

        DAXA_EXPORT_CXX void copy_buffer_to_buffer(TaskBufferCopyInfo const & info);
        DAXA_EXPORT_CXX void copy_image_to_image(TaskImageCopyInfo const & info);

        template <typename TTask>
            requires std::is_base_of_v<IPartialTask, TTask>
        void add_task(TTask const & task)
        {
            using NoRefTTask = std::remove_reference_t<TTask>;
            // DAXA_DBG_ASSERT_TRUE_M(task.attachments().size() == task.attachments()._offset, "Detected task attachment count differing from Task head declared attachment count!");
            struct WrapperTask : ITask
            {
                NoRefTTask _task;
                std::array<TaskAttachmentInfo, NoRefTTask::ATTACH_COUNT> _attachments = {};
                WrapperTask(NoRefTTask const & task) : _task{task}
                {
                    for (u32 i = 0; i < NoRefTTask::ATTACH_COUNT; ++i)
                    {
                        switch (_task.attachments()[i].type)
                        {
                        case daxa::TaskAttachmentType::BUFFER:
                        {
                            TaskBufferAttachmentInfo info;
                            info.name = _task.attachments()[i].value.buffer.name;
                            info.task_access = _task.attachments()[i].value.buffer.task_access;
                            info.shader_array_size = _task.attachments()[i].value.buffer.shader_array_size;
                            info.shader_as_address = _task.attachments()[i].value.buffer.shader_as_address;
                            info.view = daxa::get<TaskBufferView>(task.views.views[i]);
                            _attachments[i] = info;
                        }
                        break;
                        case daxa::TaskAttachmentType::TLAS:
                        {
                            TaskTlasAttachmentInfo info;
                            info.name = _task.attachments()[i].value.tlas.name;
                            info.task_access = _task.attachments()[i].value.tlas.task_access;
                            info.shader_as_address = _task.attachments()[i].value.tlas.shader_as_address;
                            info.view = daxa::get<TaskTlasView>(task.views.views[i]);
                            _attachments[i] = info;
                        }
                        break;
                        case daxa::TaskAttachmentType::BLAS:
                        {
                            TaskBlasAttachmentInfo info;
                            info.name = _task.attachments()[i].value.blas.name;
                            info.task_access = _task.attachments()[i].value.blas.task_access;
                            info.view = daxa::get<TaskBlasView>(task.views.views[i]);
                            _attachments[i] = info;
                        }
                        break;
                        case daxa::TaskAttachmentType::IMAGE:
                        {
                            TaskImageAttachmentInfo info;
                            info.name = _task.attachments()[i].value.image.name;
                            info.task_access = _task.attachments()[i].value.image.task_access;
                            info.view_type = _task.attachments()[i].value.image.view_type;
                            info.shader_array_size = _task.attachments()[i].value.image.shader_array_size;
                            info.shader_array_type = _task.attachments()[i].value.image.shader_array_type;
                            info.shader_as_index = _task.attachments()[i].value.image.shader_as_index;
                            info.view = daxa::get<TaskImageView>(task.views.views[i]);
                            _attachments[i] = info;
                        }
                        break;
                        default:
                        {
                            DAXA_DBG_ASSERT_TRUE_M(false, "Declared attachment count does not match actually declared attachment count");
                        }
                        break;
                        }
                    }
                }
                constexpr virtual auto attachments() -> std::span<TaskAttachmentInfo> { return _attachments; }
                constexpr virtual auto attachments() const -> std::span<TaskAttachmentInfo const> { return _attachments; }
                constexpr virtual auto name() const -> std::string_view { return NoRefTTask::name(); }
                virtual auto task_type() const -> TaskType { return _task.task_type(); };
                virtual void callback(TaskInterface ti) { _task.callback(ti); };
            };
            auto wrapped_task = std::make_unique<WrapperTask>(std::move(task));
            add_task(std::move(wrapped_task));
        }
        void add_task(InlineTaskInfo const & inline_task_info)
        {
            add_task(std::make_unique<InlineTask>(inline_task_info));
        }
        template <typename T>
        void add_task(T && inline_task)
            requires std::is_base_of_v<InlineTask, T> || std::is_same_v<InlineTask, T>
        {
            add_task(std::make_unique<T>(std::move(inline_task)));
        }

        DAXA_EXPORT_CXX void conditional(TaskGraphConditionalInfo const & conditional_info);
        DAXA_EXPORT_CXX void submit(TaskSubmitInfo const & info);
        DAXA_EXPORT_CXX void present(TaskPresentInfo const & info);

        // TODO: make move only. Return ExecutableTaskGraph.
        DAXA_EXPORT_CXX void complete(TaskCompleteInfo const & info);

        DAXA_EXPORT_CXX void execute(ExecutionInfo const & info);

        DAXA_EXPORT_CXX auto get_debug_string() -> std::string;
        DAXA_EXPORT_CXX auto get_transient_memory_size() -> daxa::usize;

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        DAXA_EXPORT_CXX static auto inc_refcnt(ImplHandle const * object) -> u64;
        DAXA_EXPORT_CXX static auto dec_refcnt(ImplHandle const * object) -> u64;

      private:
        DAXA_EXPORT_CXX void add_task(std::unique_ptr<ITask> && task);
    };
} // namespace daxa
