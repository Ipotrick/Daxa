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

    enum struct TaskAccessConcurrency
    {
        CONCURRENT,
        EXCLUSIVE
    };

    auto queue_to_queue_index(daxa::Queue queue) -> u32
    {
        u32 offsets[3] = {
            0,
            1,
            1 + DAXA_MAX_COMPUTE_QUEUE_COUNT,
        };
        return offsets[static_cast<u32>(queue.type)] + queue.index;
    }

    auto queue_index_to_queue(u32 flat_index) -> daxa::Queue
    {
        daxa::Queue queues[] = {
            daxa::QUEUE_MAIN,
            daxa::QUEUE_COMPUTE_0,
            daxa::QUEUE_COMPUTE_1,
            daxa::QUEUE_COMPUTE_2,
            daxa::QUEUE_COMPUTE_3,
            daxa::QUEUE_TRANSFER_0,
            daxa::QUEUE_TRANSFER_1,
        };
        return queues[flat_index];
    }

    auto queue_bits_to_first_queue_index(u32 queue_bits) -> u32
    {
        return 31u - static_cast<u32>(std::countl_zero(queue_bits));
    }

    auto queue_index_to_queue_bit(u32 queue_index) -> u32
    {
        return 1u << queue_index;
    }

    inline auto queue_bits_to_string(u32 queue_bits) -> std::string
    {
        std::string ret = {};
        u32 iter = queue_bits;
        while(iter)
        {
            u32 queue_index = queue_bits_to_first_queue_index(iter);
            iter &= ~(1u << queue_index);

            Queue queue = queue_index_to_queue(queue_index);

            ret += to_string(queue);
            if (iter)
            {
                ret += ", ";
            }
        }
        return ret;
    }

    struct ImplTask;

    struct TaskAttachmentAccess
    {
        ImplTask* task = {};
        u32 task_index = {};
        u32 attachment_index = {};
    };

    struct TaskBarrier;

    struct AccessGroup
    {
        TaskStage stages = {};
        TaskAccessType type = {};
        u32 queue_bits = {};
        std::span<TaskAttachmentAccess> tasks = {};
        u32 final_schedule_first_batch = ~0u;
        u32 final_schedule_last_batch = {};
        TaskBarrier const * final_schedule_pre_barrier = {};
    };

    enum struct TaskResourceKind
    {
        BUFFER,
        TLAS,
        BLAS,
        IMAGE
    };

    inline auto to_string(TaskResourceKind kind) -> std::string_view
    {
        switch(kind)
        {
        case TaskResourceKind::BUFFER: return "BUFFER";
        case TaskResourceKind::TLAS: return "TLAS";
        case TaskResourceKind::BLAS: return "BLAS";
        case TaskResourceKind::IMAGE: return "IMAGE";
        default: return "UNKNOWN";
        }
    }

    struct ImplTaskResource
    {
        std::string_view name = {};
        TaskResourceKind kind = {};
        ImplExternalResource * external = {};
        std::span<AccessGroup> access_timeline = {};
        u32 final_schedule_first_batch = {};
        u32 final_schedule_last_batch = {};
        u32 final_schedule_first_submit = {};
        u32 final_schedule_last_submit = {};
        u32 queue_bits = {};
        u64 allocation_size = {};
        u64 allocation_alignment = {};
        u32 allocation_allowed_memory_type_bits = {};
        u64 allocation_offset = {};

        union {
            BufferId buffer;
            TlasId tlas;
            BlasId blas;
            ImageId image;
        } id;

        union {
            struct
            {
                usize size = {};
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

    union AttachmentShaderBlobSection
    {
        BufferId* buffer_id;
        DeviceAddress* buffer_address;
        TlasId* tlas_id; 
        DeviceAddress* tlas_address;
        std::span<ImageViewId> image_view_ids;
        std::span<ImageViewIndex> image_view_indices;
    };

    struct ImplTask
    {
        std::string_view name = {};                                 
        void (*task_callback)(daxa::TaskInterface, void*) = {};     
        u64* task_callback_memory = {};                             // holds callback captured variables
        std::span<TaskAttachmentInfo> attachments = {};             
        std::span<std::pair<ImplTaskResource*, u32>> attachment_resources = {};
        std::span<std::pair<AccessGroup*, u32>> attachment_access_groups = {};      // set when compiling
        u32 attachment_shader_blob_size = {};                       
        u32 attachment_shader_blob_alignment = {};            
        std::span<std::byte> attachment_shader_blob = {};      
        std::span<AttachmentShaderBlobSection> attachment_shader_blob_sections = {};
        std::span<std::span<ImageViewId>> attachment_image_views = {};
        TaskType task_type = {};                                    
        Queue queue = {};        
        u32 submit_index = {};                                   
        u32 final_schedule_batch = {};
    };

    struct ImplPresentInfo
    {
        std::vector<BinarySemaphore> binary_semaphores = {};
        std::span<BinarySemaphore> * additional_binary_semaphores = {};
    };


    auto task_image_access_to_layout_access(TaskAccess const & access) -> std::tuple<ImageLayout, Access, TaskAccessConcurrency>;
    auto task_access_to_access(TaskAccess const & access) -> std::pair<Access, TaskAccessConcurrency>;

    struct ImplTaskGraph;

    // Used to allocate id - because all persistent resources have unique id we need a single point
    // from which they are generated
    static inline std::atomic_uint32_t exec_unique_resource_next_index = 1;

    struct ImplExternalResource final : ImplHandle
    {
        ImplExternalResource(TaskBufferInfo a_info);
        ImplExternalResource(Device & device, BufferInfo const & a_info);
        ImplExternalResource(TaskBlasInfo a_info);
        ImplExternalResource(TaskTlasInfo a_info);
        ImplExternalResource(TaskImageInfo a_info);
        ~ImplExternalResource();

        std::string name = {};
        TaskResourceKind kind = {};

        union {
            BufferId buffer;
            BlasId blas;
            TlasId tlas;
            ImageId image;
        } id = {.buffer = {}};
        
        u32 pre_graph_queue_bits = 0u;
        bool pre_graph_is_general_layout = false;
        bool was_presented = false;
        bool is_swapchain_image = false;
        bool owns_resource = false;
        std::optional<Device> device = {};

        u32 unique_index = std::numeric_limits<u32>::max();

        static void zero_ref_callback(ImplHandle const * handle);
    };

    struct ImplTaskRuntimeInterface
    {
        ImplTaskGraph & task_graph;
        CommandRecorder & recorder;
        ImplTask * current_task = {};
    };

    struct TaskBarrier
    {
        AccessGroup * src_access_group = {};
        AccessGroup * dst_access_group = {};
        Access src_access = AccessConsts::NONE;
        Access dst_access = AccessConsts::NONE;
        ImplTaskResource* resource = {};
        ImageLayoutOperation layout_operation = {};
    };

    struct TasksBatch
    {
        std::span<std::pair<ImplTask*, u32>> tasks = {};
        std::span<TaskBarrier> pre_batch_barriers = {};
        std::span<TaskBarrier> pre_batch_image_barriers = {};
    };

    struct TasksSubmit
    {
        u32 final_schedule_first_batch = {};
        u32 final_schedule_last_batch = {};
        u32 queue_bits = {};
        std::array<u32, DAXA_QUEUE_COUNT> queue_batch_counts = {};
        std::array<std::span<TasksBatch>, DAXA_QUEUE_COUNT> queue_batches = {};
        std::array<std::string_view, DAXA_QUEUE_COUNT> queue_batch_cmd_recorder_labels = {};
        std::span<u32> queue_indices = {};
    };

    struct TaskGraphPresent
    {
        u32 submit_index = ~0u;
        Queue queue = QUEUE_MAIN;
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
        std::unordered_map<std::string_view, std::tuple<ImplTask*, u32, u32>> name_to_task_table = {};          // unique task name -> local id into task. Duplicate Task names are modified to be unique
        std::span<std::pair<ImplTaskResource*, u32>> external_resources = {};
        std::unordered_map<std::string_view, std::pair<ImplTaskResource*, u32>> name_to_resource_table = {};    // unique buffer name -> local id into buffers.
        std::unordered_map<u32, std::pair<ImplTaskResource*, u32>> external_idx_to_resource_table = {};         // global unique external id -> local id into buffers.
        ArenaDynamicArray8k<TasksSubmit> submits = {};
        u32 flat_batch_count = {};                                                                              // total batch count ignoring async compute;
        u32 queue_bits = {};
        daxa::MemoryBlock transient_memory_block = {};
        std::optional<daxa::TransferMemoryPool> staging_memory = {};
        std::optional<TaskGraphPresent> present = {};
        ImplTaskResource* swapchain_image = nullptr;
        
        static void zero_ref_callback(ImplHandle const * handle);
    };

} // namespace daxa
