#pragma once

#include "impl_core.hpp"
#include "impl_sync.hpp"
#include "impl_pipeline.hpp"

#include <daxa/c/command_list.h>
#include <daxa/command_list.hpp>

using namespace daxa;

struct ImplDevice;

static inline constexpr u8 DEFERRED_DESTRUCTION_BUFFER_INDEX = 0;
static inline constexpr u8 DEFERRED_DESTRUCTION_IMAGE_INDEX = 1;
static inline constexpr u8 DEFERRED_DESTRUCTION_IMAGE_VIEW_INDEX = 2;
static inline constexpr u8 DEFERRED_DESTRUCTION_SAMPLER_INDEX = 3;
static inline constexpr u8 DEFERRED_DESTRUCTION_TIMELINE_QUERY_POOL_INDEX = 4;
// TODO: maybe reintroduce this in some fashion?
// static inline constexpr usize DEFERRED_DESTRUCTION_COUNT_MAX = 32;

static inline constexpr usize COMMAND_LIST_BARRIER_MAX_BATCH_SIZE = 16;
static inline constexpr usize COMMAND_LIST_COLOR_ATTACHMENT_MAX = 16;
static inline constexpr usize COMMAND_LIST_UNIFORM_BUFFER_BINDINGS_COUNT = 8;

struct CommandBufferPoolPool
{
    auto get(daxa_Device device) -> std::pair<VkCommandPool, VkCommandBuffer>;

    void put_back(std::pair<VkCommandPool, VkCommandBuffer> pool_and_buffer);

    void cleanup(daxa_Device device);

    std::vector<std::pair<VkCommandPool, VkCommandBuffer>> pools_and_buffers = {};
};

struct CommandListZombie
{
    VkCommandBuffer vk_cmd_buffer = {};
    VkCommandPool vk_cmd_pool = {};
};

struct daxa_ImplCommandList final : ImplHandle
{
    daxa_Device device = {};
    daxa_CommandListInfo info = {};
    std::string info_name = {};
    VkCommandBuffer vk_cmd_buffer = {};
    VkCommandPool vk_cmd_pool = {};
    bool recording_complete = {};
    std::array<VkMemoryBarrier2, COMMAND_LIST_BARRIER_MAX_BATCH_SIZE> memory_barrier_batch = {};
    std::array<VkImageMemoryBarrier2, COMMAND_LIST_BARRIER_MAX_BATCH_SIZE> image_barrier_batch = {};
    usize image_barrier_batch_count = {};
    usize memory_barrier_batch_count = {};
    usize split_barrier_batch_count = {};
    std::vector<std::pair<GPUResourceId, u8>> deferred_destructions = {};
    std::array<daxa_SetUniformBufferInfo, COMMAND_LIST_UNIFORM_BUFFER_BINDINGS_COUNT> current_constant_buffer_bindings = {};
    // TODO:    These vectors seem to be fast enough. overhead is around 1-4% in cmd recording.
    //          It might be cool to have some slab allocator for these.
    // If there is demand, we could make an instance or cmd list flag to disable the submit checks.
    // TODO:    Also collect ref counted handles.
    std::vector<BufferId> used_buffers = {};
    std::vector<ImageId> used_images = {};
    std::vector<ImageViewId> used_image_views = {};
    std::vector<SamplerId> used_samplers = {};

    void flush_uniform_buffer_bindings(VkPipelineBindPoint bind_point, VkPipelineLayout pipeline_layout);

    static void zero_ref_callback(ImplHandle const * handle);
};
