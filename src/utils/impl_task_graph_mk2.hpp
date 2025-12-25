#pragma once

#include "../impl_core.hpp"

#include <variant>
#include <sstream>
#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/task_graph_types.hpp>
#include <memory_resource>

// This file should house the rewrite of taskgraph.
// Please DO NOT INCLUDE ANY existing impl TG headers.
// Please ONLY INCLUDE public api task graph headers.

///
/// === Design Notes === Important Implementation Details ===
///
/// Memory Allocation
/// * allocations are made using the MemoryArena/StackAllocator classes
/// * persistent allocations go into the task_memory MemoryArena.
/// * transient allocations go into the transient_memory StackAllocator.
/// * all input data to spans/string views is reallocated and copied internally
/// * the PagedVector and StaticVector classes are used for all arrays that can grow/ when the size of an array can not be known upfront.
///

#include "../impl_core.hpp"

#include <variant>
#include <sstream>
#include <daxa/utils/task_graph.hpp>

namespace daxa
{
    struct ImplDevice;

    using TaskBatchId = usize;

    using TaskId = usize;

    struct LastConcurrentAccessSplitBarrierIndex
    {
        usize index;
    };

    struct LastConcurrentAccessBarrierIndex
    {
        usize index;
    };

    struct ResourceLifetime
    {
        // TODO(msakmary, pahrens) This will not be needed once we make batches linear and not
        // contained in submit scopes
        struct CombinedBatchIndex
        {
            usize submit_scope_index = std::numeric_limits<u32>::max();
            usize task_batch_index = std::numeric_limits<u32>::max();
        };
        CombinedBatchIndex first_use;
        CombinedBatchIndex last_use;
    };

    enum struct TaskAccessConcurrency
    {
        CONCURRENT,
        EXCLUSIVE
    };

    struct PerPermTaskBuffer
    {
        /// Every permutation always has all buffers but they are not necessarily valid in that permutation.
        /// This boolean is used to check this.
        bool valid = {};
        TaskAccessConcurrency latest_access_concurrent = TaskAccessConcurrency::EXCLUSIVE;
        Access latest_access = AccessConsts::NONE;
        usize latest_access_batch_index = {};
        usize latest_access_submit_scope_index = {};

        // Multi-queue.
        Queue latest_access_submit_scope_queue = {};
        bool latest_access_submit_scope_exclusive_locked = {}; 
        bool latest_access_submit_scope_concurrent_locked = {};

        usize first_access_batch_index = {};
        usize first_access_submit_scope_index = {};
        Access first_access = AccessConsts::NONE;
        // When the last index was a read and an additional read is followed after,
        // we will combine all barriers into one, which is the first barrier that the first read generates.
        Variant<Monostate, LastConcurrentAccessSplitBarrierIndex, LastConcurrentAccessBarrierIndex> latest_concurrent_access_barrer_index = Monostate{};
        // Used to reorder tasks to the earliest possible batch within the current concurrent scope.
        // This is the first batch where the resource is completely usable in the access state given.
        // This means all barriers are done even split barriers are done at this point.
        /// WARNING: THIS DOES NOT MEAN THAT THIS IS THE FIRST BATCH AFTER THE PREVIOUS ACCESS. THE FIRST TASK TO USE THIS ACCESS DETERMINES THE START BATCH. THIS SHOULD BE IMPROVED. WE COULD FOR EXAMPLE STORE THE PRIOR DEPENDENCY TASKS/BATCHES FOR THE CURRENT CONCURRENT SEQUENCE!
        usize latest_concurrent_sequence_start_batch = ~0u;
        bool used_queue_concurrently = {};
        std::variant<BufferId, BlasId, TlasId> actual_id = BufferId{};
        BufferId as_backing_buffer_id = {};

        ResourceLifetime lifetime = {};
        usize allocation_offset = {};
        daxa::MemoryRequirements memory_requirements = {};
    };

    struct ImplTask;

    struct TaskAttachmentAccess
    {
        ImplTask* task = {};
        u32 task_index = {};
        u32 attachment_index = {};
    };

    struct AccessGroup
    {
        PipelineStageFlags used_stages = {};    // TODO: put this into a field with type Access
        TaskAccessType type = {};               // TODO: put this into a field with type Access
        u32 used_queues_bitfield = {};
        std::span<TaskAttachmentAccess> tasks = {};
        u32 final_schedule_first_batch = ~0u;
        u32 final_schedule_last_batch = {};
    };

    enum struct TaskResourceKind
    {
        BUFFER,
        TLAS,
        BLAS,
        IMAGE
    };

    struct ImplTaskResource
    {
        std::string_view name = {};
        TaskResourceKind kind = {};
        void * external = {};
        std::span<AccessGroup> access_timeline = {};
        u32 final_schedule_first_batch = {};
        u32 final_schedule_last_batch = {};
        u32 final_schedule_first_submit = {};
        u32 final_schedule_last_submit = {};
        u32 used_queues_bitfield = {};

        union {
            BufferId buffer;
            TlasId tlas;
            BlasId blas;
            ImageId image;
        } id;

        union {
            struct
            {
                usize size;
            } buffer;
            struct
            {
                ImageCreateFlags flags = {};
                u32 dimensions = {};
                Format format = {};
                Extent3D size = {0, 0, 0};
                u32 mip_level_count = {};
                u32 array_layer_count = {};
                u32 sample_count = {};
                SharingMode sharing_mode = {};
                ImageUsageFlags usage = {};
            } image;
        } info;
    };

    struct ExtendedImageSliceState
    {
        ImageSliceState state = {};
        TaskAccessConcurrency latest_access_concurrent = TaskAccessConcurrency::EXCLUSIVE;
        usize latest_access_batch_index = {};
        usize latest_access_submit_scope_index = {};

        // Multi-queue.
        Queue latest_access_submit_scope_queue = {};
        bool latest_access_submit_scope_exclusive_locked = {}; 
        bool latest_access_submit_scope_concurrent_locked = {};

        // When the last index was a read and an additional read is followed after,
        // we will combine all barriers into one, which is the first barrier that the first read generates.
        Variant<Monostate, LastConcurrentAccessSplitBarrierIndex, LastConcurrentAccessBarrierIndex> latest_concurrent_access_barrer_index = Monostate{};
        // Used to reorder tasks to the earliest possible batch within the current concurrent scope.
        // This is the first batch where the resource is completely usable in the access state given.
        // This means all barriers are done even split barriers are done at this point.
        /// WARNING: THIS DOES NOT MEAN THAT THIS IS THE FIRST BATCH AFTER THE PREVIOUS ACCESS. THE FIRST TASK TO USE THIS ACCESS DETERMINES THE START BATCH. THIS SHOULD BE IMPROVED. WE COULD FOR EXAMPLE STORE THE PRIOR DEPENDENCY TASKS/BATCHES FOR THE CURRENT CONCURRENT SEQUENCE!
        usize latest_concurrent_sequence_start_batch = ~0u;
    };

    struct PerPermTaskImage
    {
        /// Every permutation always has all buffers but they are not necessarily valid in that permutation.
        /// This boolean is used to check this.
        bool valid = {};
        bool swapchain_semaphore_waited_upon = {};
        ArenaDynamicArray8k<ExtendedImageSliceState> last_slice_states = {};
        ArenaDynamicArray8k<ExtendedImageSliceState> first_slice_states = {};
        // only for transient images
        ResourceLifetime lifetime = {};
        ImageCreateFlags create_flags = ImageCreateFlagBits::NONE;
        ImageUsageFlags usage = ImageUsageFlagBits::NONE;
        bool used_queue_concurrently = {};
        ImageId actual_image = {};
        usize allocation_offset = {};
        daxa::MemoryRequirements memory_requirements = {};
    };

    struct TaskBarrier
    {
        // when this ID is invalid, this barrier is NOT an image memory barrier but just a memory barrier.
        // So when ID invalid => memory barrier, ID valid => image memory barrier.
        TaskImageView image_id = {};
        ImageMipArraySlice slice = {};
        ImageLayout layout_before = {};
        ImageLayout layout_after = {};
        Access src_access = {};
        Access dst_access = {};
    };

    struct TaskSplitBarrier : TaskBarrier
    {
        Event split_barrier_state;
    };

    // TODO:
    // Remove support for external images image view generation
    // Create all image views for persistent and transient images when allocating and creating them with the taskgraph.
    struct ImplTask
    {
        std::string_view name = {};                                 
        void (*task_callback)(daxa::TaskInterface, void*) = {};     
        u64* task_callback_memory = {};                             // holds callback captured variables
        std::span<TaskAttachmentInfo> attachments = {};             
        std::span<AccessGroup*> attachment_access_groups = {};      // set when compiling
        u32 attachment_shader_blob_size = {};                       
        u32 attachment_shader_blob_alignment = {};            
        std::span<u8> attachment_shader_blob = {};      
        std::span<u32> attachment_in_blob_offsets = {};
        std::span<std::span<ImageViewId>> attachment_image_views = {};
        TaskType task_type = {};                                    
        Queue queue = {};        
        u32 submit_index = {};                                   
        std::span<ImageId> runtime_images_last_execution = {};      // set when executing // Used to verify image view cache
        u32 final_schedule_batch = {};
    };

    struct ImplPresentInfo
    {
        std::vector<BinarySemaphore> binary_semaphores = {};
        std::span<BinarySemaphore> * additional_binary_semaphores = {};
    };

    struct TaskBatch
    {
        ArenaDynamicArray8k<usize> pipeline_barrier_indices = {};
        ArenaDynamicArray8k<usize> wait_split_barrier_indices = {};
        ArenaDynamicArray8k<TaskId> tasks = {};
        ArenaDynamicArray8k<usize> signal_split_barrier_indices = {};
    };

    struct QueueSubmitScope
    {
        // These barriers are inserted after all batches and their sync.
        ArenaDynamicArray8k<usize> last_minute_barrier_indices = {};
        ArenaDynamicArray8k<TaskBatch> task_batches = {};
        ArenaDynamicArray8k<u64> used_swapchain_task_images = {};
        std::optional<ImplPresentInfo> present_info = {};
    };

    struct TaskBatchSubmitScope
    {
        CommandSubmitInfo submit_info = {};
        TaskSubmitInfo user_submit_info = {};
        std::array<QueueSubmitScope, DAXA_MAX_TOTAL_QUEUE_COUNT> queue_submit_scopes = {};
    };

    auto task_image_access_to_layout_access(TaskAccess const & access) -> std::tuple<ImageLayout, Access, TaskAccessConcurrency>;
    auto task_access_to_access(TaskAccess const & access) -> std::pair<Access, TaskAccessConcurrency>;

    struct ImplTaskGraph;

    // Used to allocate id - because all persistent resources have unique id we need a single point
    // from which they are generated
    static inline std::atomic_uint32_t exec_unique_resource_next_index = 1;

    struct ImplPersistentTaskBufferBlasTlas final : ImplHandle
    {
        ImplPersistentTaskBufferBlasTlas(TaskBufferInfo a_info);
        ImplPersistentTaskBufferBlasTlas(Device & device, BufferInfo const & a_info);
        ImplPersistentTaskBufferBlasTlas(TaskBlasInfo a_info);
        ImplPersistentTaskBufferBlasTlas(TaskTlasInfo a_info);
        ~ImplPersistentTaskBufferBlasTlas();

        std::variant<
            std::vector<BufferId>,
            std::vector<BlasId>,
            std::vector<TlasId>>
            actual_ids = {};

        Access pre_graph_access = {};
        u32 pre_graph_used_queues_bitfield = {};

        std::variant<
            TaskBufferInfo,
            TaskBlasInfo,
            TaskTlasInfo>
            info = {};

        Optional<Device> owned_buffer_device = {};
        Optional<BufferInfo> owned_buffer_info = {};

        // Used to allocate id - because all persistent resources have unique id we need a single point
        // from which they are generated
        u32 unique_index = std::numeric_limits<u32>::max();

        static void zero_ref_callback(ImplHandle const * handle);
    };

    struct ImplPersistentTaskImage final : ImplHandle
    {
        ImplPersistentTaskImage(TaskImageInfo const & a_info);
        ~ImplPersistentTaskImage();

        TaskImageInfo info = {};
        // One task buffer can back multiple buffers.
        std::vector<ImageId> actual_images = {};
        
        Access pre_graph_access = {};
        u32 pre_graph_used_queues_bitfield = ~0u;
        bool pre_graph_is_undefined_layout = true;

        u32 unique_index = std::numeric_limits<u32>::max();

        static void zero_ref_callback(ImplHandle const * handle);
    };

    struct PermIndepTaskBufferInfo
    {
        struct External
        {
            std::variant<TaskBuffer, TaskBlas, TaskTlas> buffer_blas_tlas = {};

            auto get() -> ImplPersistentTaskBufferBlasTlas &
            {
                ImplPersistentTaskBufferBlasTlas * ret = {};
                std::visit([&](auto & ptr)
                           { ret = ptr.get(); }, buffer_blas_tlas);
                return *ret;
            }
            auto get() const -> ImplPersistentTaskBufferBlasTlas const &
            {
                ImplPersistentTaskBufferBlasTlas const * ret = {};
                std::visit([&](auto & ptr)
                           { ret = ptr.get(); }, buffer_blas_tlas);
                return *ret;
            }
        };
        struct Owned
        {
            TaskAttachmentType type = {};
            TaskTransientBufferInfo info = {};
        };
        Variant<External, Owned> task_buffer_data;
        std::string_view name = {};

        inline auto get_external() -> ImplPersistentTaskBufferBlasTlas &
        {
            return daxa::get<External>(task_buffer_data).get();
        }
        inline auto get_external() const -> ImplPersistentTaskBufferBlasTlas const &
        {
            return daxa::get<External>(task_buffer_data).get();
        }
        inline auto is_external() const -> bool
        {
            return daxa::holds_alternative<External>(task_buffer_data);
        }
    };

    struct PermIndepTaskImageInfo
    {
        struct External
        {
            TaskImage image = {};
            auto get() -> ImplPersistentTaskImage &
            {
                return **r_cast<ImplPersistentTaskImage **>(&image);
            }
            auto get() const -> ImplPersistentTaskImage const &
            {
                return **r_cast<ImplPersistentTaskImage const * const *>(&image);
            }
        };
        struct Owned
        {
            TaskTransientImageInfo info = {};
            // When the resource is temporal we have to store the in-between-runs state of the resource
            
        };
        Variant<External, Owned> task_image_data;
        std::string_view name = {};

        inline auto get_external() -> ImplPersistentTaskImage &
        {
            return daxa::get<External>(task_image_data).get();
        }
        inline auto get_external() const -> ImplPersistentTaskImage const &
        {
            return daxa::get<External>(task_image_data).get();
        }
        inline auto is_external() const -> bool
        {
            return daxa::holds_alternative<External>(task_image_data);
        }
    };

    struct ImplTaskRuntimeInterface
    {
        // interface:
        ImplTaskGraph & task_graph;
        CommandRecorder & recorder;
        ImplTask * current_task = {};
        types::DeviceAddress device_address = {};
        bool reuse_last_command_list = true;
        std::optional<BinarySemaphore> last_submit_semaphore = {};
    };

    struct TasksSubmit
    {
        u32 first_task = {};
        u32 task_count = {};
        u32 used_queues_bitfield = {};  // set when compiling
    };

    struct ImplTaskGraph final : ImplHandle
    {
        ImplTaskGraph(TaskGraphInfo a_info); 
        ~ImplTaskGraph();

        static inline std::atomic_uint32_t exec_unique_next_index = 1;
        u32 unique_index = {};
        TaskGraphInfo info;
        MemoryArena task_memory = {};
        bool compiled = {};

        ArenaDynamicArray8k<ImplTask> tasks = {};
        ArenaDynamicArray8k<ImplTaskResource> resources = {};
        std::span<std::pair<ImplTaskResource*, u32>> external_resources = {};
        std::unordered_map<std::string_view, std::pair<ImplTaskResource*, u32>> name_to_resource_table = {}; // unique buffer name -> local id into buffers.
        std::unordered_map<u32, std::pair<ImplTaskResource*, u32>> external_idx_to_resource_table = {};      // global unique external id -> local id into buffers.
        daxa::MemoryBlock transient_memory_block = {};

        ArenaDynamicArray8k<TasksSubmit> submits = {};

        static void zero_ref_callback(ImplHandle const * handle);
    };

} // namespace daxa
