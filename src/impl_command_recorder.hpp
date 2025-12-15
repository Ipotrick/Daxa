#pragma once

#include "impl_core.hpp"

#include <daxa/c/command_recorder.h>
#include <daxa/command_recorder.hpp>
#include <mutex>

using namespace daxa;

struct ImplDevice;

static inline constexpr usize DAXA_MAX_COMMAND_POOLS = 128u;

static inline constexpr u8 DEFERRED_DESTRUCTION_BUFFER_INDEX = 0u;
static inline constexpr u8 DEFERRED_DESTRUCTION_IMAGE_INDEX = 1u;
static inline constexpr u8 DEFERRED_DESTRUCTION_IMAGE_VIEW_INDEX = 2u;
static inline constexpr u8 DEFERRED_DESTRUCTION_SAMPLER_INDEX = 3u;
static inline constexpr u8 DEFERRED_DESTRUCTION_TIMELINE_QUERY_POOL_INDEX = 4u;

static inline constexpr usize COMMAND_RECORDER_BARRIER_MAX_BATCH_SIZE = 16u;
static inline constexpr usize COMMAND_LIST_COLOR_ATTACHMENT_MAX = 8u;

struct ImplTransientCommandArena
{
    VkCommandPool vk_command_pool = {};
    VkCommandBuffer vk_command_buffer = {};
    daxa_QueueFamily queue_family = {};
    u32 vk_queue_family_index = {};
    
    /// TODO: Replace these with arena dynamic arrays
    /// TODO: Add Automatically growing memory arena (when writing this i like the idea of having a per device pool of slabs (maybe 1-128kib) that the arenas can source their memory from)
    std::vector<std::pair<GPUResourceId, u8>> deferred_destructions = {};
    std::vector<BufferId> used_buffers = {};
    std::vector<ImageId> used_images = {};
    std::vector<ImageViewId> used_image_views = {};
    std::vector<SamplerId> used_samplers = {};
    std::vector<TlasId> used_tlass = {};
    std::vector<BlasId> used_blass = {};
};

struct ImplTransientCommandArenas
{
    std::pair<std::array<ImplTransientCommandArena, DAXA_MAX_COMMAND_POOLS>, u32> transient_command_arenas = {};
    std::array<std::deque<u32>, DAXA_QUEUE_FAMILY_MAX_ENUM> available_arenas = {};
    std::mutex mtx = {};

    void initialize() 
    {

    }

    void cleanup(VkDevice vk_device)
    {
        for (u32 i = 0; i < transient_command_arenas.second; ++i)
        {
            vkDestroyCommandPool(vk_device, transient_command_arenas.first[i].vk_command_pool, nullptr);
        }
    }

    auto get_arena(VkDevice vk_device, daxa_QueueFamily queue_family, u32 queue_family_index, ImplTransientCommandArena*& out, std::unique_lock<std::mutex>* lock = {}) -> daxa_Result
    {
        std::unique_lock<std::mutex> local_lock = lock == nullptr ? std::unique_lock(mtx) : std::unique_lock<std::mutex>{};

        out = {};
        daxa_Result result = DAXA_RESULT_SUCCESS;
        std::deque<u32>& available_command_pools = available_arenas[queue_family];

        if (available_command_pools.size() == 0)
        {
            // Get new Pool in array
            if (transient_command_arenas.second >= DAXA_MAX_COMMAND_POOLS)
            {
                result = DAXA_RESULT_ERROR_EXCEEDED_MAX_COMMAND_POOLS;
            }
            _DAXA_RETURN_IF_ERROR(result, result);
            u32 const command_arena_index = transient_command_arenas.second++;
            ImplTransientCommandArena& transient_cmd_arena = transient_command_arenas.first[command_arena_index];
            defer
            {
                if (result != DAXA_RESULT_SUCCESS)
                {
                    --transient_command_arenas.second;
                }
            };

            // Create Command Pool
            VkCommandPoolCreateInfo const vk_command_pool_create_info{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                .queueFamilyIndex = queue_family_index,
            };
            result = static_cast<daxa_Result>(vkCreateCommandPool(vk_device, &vk_command_pool_create_info, nullptr, &transient_cmd_arena.vk_command_pool));
            _DAXA_RETURN_IF_ERROR(result, result);
            defer
            {
                if (result != DAXA_RESULT_SUCCESS)
                {
                    vkDestroyCommandPool(vk_device, transient_cmd_arena.vk_command_pool, nullptr);
                }
            };                

            // Create Command Buffer
            VkCommandBufferAllocateInfo const vk_command_buffer_allocate_info{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .pNext = nullptr,
                .commandPool = transient_cmd_arena.vk_command_pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1u,
            };
            result = static_cast<daxa_Result>(vkAllocateCommandBuffers(vk_device, &vk_command_buffer_allocate_info, &transient_cmd_arena.vk_command_buffer));
            _DAXA_RETURN_IF_ERROR(result, result);

            transient_cmd_arena.queue_family = queue_family;
            transient_cmd_arena.vk_queue_family_index = queue_family_index;

            // Append pool to available list
            available_command_pools.push_back(command_arena_index);
        }

        // Determine command pool and buffer index and fill the out variable
        u32 const command_arena_index = available_command_pools.back();
        available_command_pools.pop_back();
        out = &transient_command_arenas.first[command_arena_index];

        return result;
    }

    auto retire_arena(VkDevice vk_device, ImplTransientCommandArena * cmd_arena, std::unique_lock<std::mutex>* lock = {}) -> daxa_Result
    {
        std::unique_lock<std::mutex> local_lock = lock == nullptr ? std::unique_lock(mtx) : std::unique_lock<std::mutex>{};
        daxa_Result result = DAXA_RESULT_SUCCESS;

        u32 const cmd_arena_index = (cmd_arena - transient_command_arenas.first.data());

        result = static_cast<daxa_Result>(vkResetCommandPool(vk_device, cmd_arena->vk_command_pool, {}));
        _DAXA_RETURN_IF_ERROR(result, result);

        cmd_arena->used_buffers.clear();
        cmd_arena->used_images.clear();
        cmd_arena->used_image_views.clear();
        cmd_arena->used_samplers.clear();
        cmd_arena->used_tlass.clear();
        cmd_arena->used_blass.clear();
        cmd_arena->deferred_destructions.clear();

        available_arenas[cmd_arena->queue_family].push_front(cmd_arena_index);

        return result;
    }
};

struct ExecutableCommandListData
{
};

struct daxa_ImplCommandRecorder final : ImplHandle
{
    daxa_Device device = {};
    bool in_renderpass = {};
    daxa_CommandRecorderInfo info = {};
    std::array<VkMemoryBarrier2, COMMAND_RECORDER_BARRIER_MAX_BATCH_SIZE> memory_barrier_batch = {};
    std::array<VkImageMemoryBarrier2, COMMAND_RECORDER_BARRIER_MAX_BATCH_SIZE> image_barrier_batch = {};
    usize image_barrier_batch_count = {};
    usize memory_barrier_batch_count = {};
    struct NoPipeline
    {
    };
    Variant<NoPipeline, daxa_ComputePipeline, daxa_RasterPipeline, daxa_RayTracingPipeline> current_pipeline = NoPipeline{};

    ImplTransientCommandArena* command_arena = {};

    static void zero_ref_callback(ImplHandle const * handle);
};

struct daxa_ImplExecutableCommandList final : ImplHandle
{
    daxa_Device device = {};
    daxa_CommandRecorderInfo info = {};
    ImplTransientCommandArena* command_arena = {};

    static void zero_ref_callback(ImplHandle const * handle);
};

void executable_cmd_list_execute_deferred_destructions(daxa_Device device, ExecutableCommandListData & cmd_list);