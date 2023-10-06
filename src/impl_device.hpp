#pragma once

#include "impl_core.hpp"

#include "impl_instance.hpp"
#include "impl_command_list.hpp"
#include "impl_pipeline.hpp"
#include "impl_swapchain.hpp"
#include "impl_gpu_resources.hpp"
#include "impl_timeline_query.hpp"
#include "impl_memory_block.hpp"

#include <daxa/c/device.h>

using namespace daxa;

struct daxa_ImplDevice final : daxa_ImplHandle
{
    daxa_Instance instance = {};
    VkPhysicalDevice vk_physical_device = {};
    VkPhysicalDeviceProperties2 vk_physical_device_properties2;
    VkDevice vk_device = {};

    PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = {};
    PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT = {};
    PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT = {};
    PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR = {};

    // Mesh shader:
    PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasksEXT = {};
    PFN_vkCmdDrawMeshTasksIndirectEXT vkCmdDrawMeshTasksIndirectEXT = {};
    PFN_vkCmdDrawMeshTasksIndirectCountEXT vkCmdDrawMeshTasksIndirectCountEXT = {};
    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = {};

    VmaAllocator vma_allocator = {};
    daxa_DeviceInfo info = {};
    std::string info_name = {};
    VkBuffer buffer_device_address_buffer = {};
    u64 * buffer_device_address_buffer_host_ptr = {};
    VmaAllocation buffer_device_address_buffer_allocation = {};

    // 'Null' resources, used to fill empty slots in the resource table after a resource is destroyed.
    // This is not necessary, as it is valid to have "garbage" in the descriptor slots given our enabled features.
    // BUT, accessing garbage descriptors normally causes a device lost immediately, making debugging much harder.
    // So instead of leaving dead descriptors dangle, daxa overwrites them with 'null' descriptors that just contain some debug value (pink 0xFF00FFFF).
    // This in particular prevents device hang in the case of a use after free if the device does not encounter a race condition on the descriptor update before.
    VkBuffer vk_null_buffer = {};
    VkImage vk_null_image = {};
    VkImageView vk_null_image_view = {};
    VkSampler vk_null_sampler = {};
    VmaAllocation vk_null_buffer_vma_allocation = {};
    VmaAllocation vk_null_image_vma_allocation = {};

    // Gpu resource table:
    GPUShaderResourceTable gpu_shader_resource_table = {};

    // Resource recycling:
    DAXA_ONLY_IF_THREADSAFETY(std::mutex main_queue_command_pool_buffer_recycle_mtx = {});
    CommandBufferPoolPool buffer_pool_pool = {};
    // Main queue:
    VkQueue main_queue_vk_queue = {};
    u32 main_queue_family_index = {};

    DAXA_ATOMIC_U64 main_queue_cpu_timeline = {};
    VkSemaphore vk_main_queue_gpu_timeline_semaphore = {};

    DAXA_ONLY_IF_THREADSAFETY(std::mutex main_queue_zombies_mtx = {});
    std::deque<std::pair<u64, std::vector<ManagedPtr>>> main_queue_submits_zombies = {};
    std::deque<std::pair<u64, CommandListZombie>> main_queue_command_list_zombies = {};
    std::deque<std::pair<u64, BufferId>> main_queue_buffer_zombies = {};
    std::deque<std::pair<u64, ImageId>> main_queue_image_zombies = {};
    std::deque<std::pair<u64, ImageViewId>> main_queue_image_view_zombies = {};
    std::deque<std::pair<u64, SamplerId>> main_queue_sampler_zombies = {};
    std::deque<std::pair<u64, SemaphoreZombie>> main_queue_semaphore_zombies = {};
    std::deque<std::pair<u64, EventZombie>> main_queue_split_barrier_zombies = {};
    std::deque<std::pair<u64, PipelineZombie>> main_queue_pipeline_zombies = {};
    std::deque<std::pair<u64, TimelineQueryPoolZombie>> main_queue_timeline_query_pool_zombies = {};
    void main_queue_collect_garbage();
    void wait_idle() const;

    static auto create(daxa_Instance instance, daxa_DeviceInfo const & info, VkPhysicalDevice physical_device) -> std::pair<daxa_Result, daxa_Device>;

    auto validate_image_slice(ImageMipArraySlice const & slice, daxa_ImageId id) -> ImageMipArraySlice;
    auto validate_image_slice(ImageMipArraySlice const & slice, daxa_ImageViewId id) -> ImageMipArraySlice;
    auto new_swapchain_image(VkImage swapchain_image, VkFormat format, u32 index, ImageUsageFlags usage, ImageInfo const & image_info) -> ImageId;

    auto slot(daxa_BufferId id) -> ImplBufferSlot &;
    auto slot(daxa_ImageId id) -> ImplImageSlot &;
    auto slot(daxa_ImageViewId id) -> ImplImageViewSlot &;
    auto slot(daxa_SamplerId id) -> ImplSamplerSlot &;

    auto slot(daxa_BufferId id) const -> ImplBufferSlot const &;
    auto slot(daxa_ImageId id) const -> ImplImageSlot const &;
    auto slot(daxa_ImageViewId id) const -> ImplImageViewSlot const &;
    auto slot(daxa_SamplerId id) const -> ImplSamplerSlot const &;

    void cleanup_buffer(daxa_BufferId id);
    void cleanup_image(daxa_ImageId id);
    void cleanup_image_view(daxa_ImageViewId id);
    void cleanup_sampler(daxa_SamplerId id);

    void zombify_buffer(daxa_BufferId id);
    void zombify_image(daxa_ImageId id);
    void zombify_image_view(daxa_ImageViewId id);
    void zombify_sampler(daxa_SamplerId id);
};
