#pragma once

#include "impl_core.hpp"

#include "impl_instance.hpp"
#include "impl_command_recorder.hpp"
#include "impl_pipeline.hpp"
#include "impl_swapchain.hpp"
#include "impl_gpu_resources.hpp"
#include "impl_timeline_query.hpp"

#include <daxa/c/device.h>

using namespace daxa;

struct SubmitZombie
{
    std::vector<daxa_BinarySemaphore> binary_semaphores = {};
    std::vector<daxa_TimelineSemaphore> timeline_semaphores = {};
};

struct daxa_ImplDevice final : public ImplHandle
{
    // General data:
    daxa_Instance instance = {};
    DeviceInfo info = {};
    VkPhysicalDevice vk_physical_device = {};
    daxa_DeviceProperties physical_device_properties = {};
    VkDevice vk_device = {};
    VmaAllocator vma_allocator = {};

    // Debug utils:
    PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = {};
    PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT = {};
    PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT = {};
    PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR = {};
#if defined(__APPLE__)
    PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR = {};
    PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR = {};
#endif

    // Mesh shader:
    PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasksEXT = {};
    PFN_vkCmdDrawMeshTasksIndirectEXT vkCmdDrawMeshTasksIndirectEXT = {};
    PFN_vkCmdDrawMeshTasksIndirectCountEXT vkCmdDrawMeshTasksIndirectCountEXT = {};
    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = {};

    // Ray tracing:
    PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR = {};
    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR = {};
    PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR = {};
    PFN_vkCmdWriteAccelerationStructuresPropertiesKHR vkCmdWriteAccelerationStructuresPropertiesKHR = {};
    PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR = {};
    PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR = {};

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

    // Command Buffer/Pool recycling:
    std::mutex main_queue_command_pool_buffer_recycle_mtx = {};
    CommandPoolPool buffer_pool_pool = {};

    // Gpu Shader Resource Object table:
    GPUShaderResourceTable gpu_sro_table = {};

    // Main queue:
    VkQueue main_queue_vk_queue = {};
    u32 main_queue_family_index = {};
    // Timelines are used to track how far ahead the gpu is before the cpu.
    // This difference between timelines is used to delay resource destruction.
    // Resources are destroyed when the gpu timeline reaches the value of the cpu timeline at the destruction of the resource.
    // This check is performed in collect_garbage, which is either manually called or automatically in submit, present or the device destruction.
    std::atomic_uint64_t main_queue_cpu_timeline = {};
    VkSemaphore vk_main_queue_gpu_timeline_semaphore = {};
    // When a resources refcount reaches 0 it becomes a zombie. A zombie is `metadata required for destruction` + `cpu timeline value at the point in time of destruction`.
    // collect_garbage checks if the gpu timeline reached the zombies timeline value and destroys the zombies.
    // TODO: replace with lockless queues.
    std::recursive_mutex main_queue_zombies_mtx = {};
    std::deque<std::pair<u64, CommandRecorderZombie>> main_queue_command_list_zombies = {};
    std::deque<std::pair<u64, BufferId>> main_queue_buffer_zombies = {};
    std::deque<std::pair<u64, ImageId>> main_queue_image_zombies = {};
    std::deque<std::pair<u64, ImageViewId>> main_queue_image_view_zombies = {};
    std::deque<std::pair<u64, SamplerId>> main_queue_sampler_zombies = {};
    std::deque<std::pair<u64, TlasId>> main_queue_tlas_zombies = {};
    std::deque<std::pair<u64, BlasId>> main_queue_blas_zombies = {};
    std::deque<std::pair<u64, SemaphoreZombie>> main_queue_semaphore_zombies = {};
    std::deque<std::pair<u64, EventZombie>> main_queue_split_barrier_zombies = {};
    std::deque<std::pair<u64, PipelineZombie>> main_queue_pipeline_zombies = {};
    std::deque<std::pair<u64, TimelineQueryPoolZombie>> main_queue_timeline_query_pool_zombies = {};
    std::deque<std::pair<u64, MemoryBlockZombie>> main_queue_memory_block_zombies = {};

    auto validate_image_slice(daxa_ImageMipArraySlice const & slice, daxa_ImageId id) -> daxa_ImageMipArraySlice;
    auto validate_image_slice(daxa_ImageMipArraySlice const & slice, daxa_ImageViewId id) -> daxa_ImageMipArraySlice;
    auto new_swapchain_image(VkImage swapchain_image, VkFormat format, u32 index, ImageUsageFlags usage, ImageInfo const & image_info) -> std::pair<daxa_Result, ImageId>;

    auto slot(daxa_BufferId id) const -> ImplBufferSlot const &;
    auto slot(daxa_ImageId id) const -> ImplImageSlot const &;
    auto slot(daxa_ImageViewId id) const -> ImplImageViewSlot const &;
    auto slot(daxa_SamplerId id) const -> ImplSamplerSlot const &;
    auto slot(daxa_TlasId id) const -> ImplTlasSlot const &;
    auto slot(daxa_BlasId id) const -> ImplBlasSlot const &;

    void cleanup_buffer(BufferId id);
    void cleanup_image(ImageId id);
    void cleanup_image_view(ImageViewId id);
    void cleanup_sampler(SamplerId id);
    void cleanup_tlas(TlasId id);
    void cleanup_blas(BlasId id);

    void zombify_buffer(BufferId id);
    void zombify_image(ImageId id);
    void zombify_image_view(ImageViewId id);
    void zombify_sampler(SamplerId id);
    void zombify_tlas(TlasId id);
    void zombify_blas(BlasId id);

    // TODO: Give physical device in info so that this function can be removed.
    // TODO: Better device selection.
    static auto create(daxa_Instance instance, daxa_DeviceInfo const & info, VkPhysicalDevice physical_device, daxa_Device device) -> daxa_Result;
    static void zero_ref_callback(ImplHandle const * handle);
};
