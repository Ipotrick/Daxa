#pragma once

#include "impl_core.hpp"

#include "impl_instance.hpp"
#include "impl_command_recorder.hpp"
#include "impl_pipeline.hpp"
#include "impl_swapchain.hpp"
#include "impl_gpu_resources.hpp"
#include "impl_timeline_query.hpp"
#include "impl_features.hpp"

#include <daxa/c/device.h>

#include <atomic>

using namespace daxa;

struct SubmitZombie
{
    std::vector<daxa_BinarySemaphore> binary_semaphores = {};
    std::vector<daxa_TimelineSemaphore> timeline_semaphores = {};
};

static inline constexpr u64 MAX_PENDING_SUBMISSIONS_PER_QUEUE = 64;
static inline constexpr u64 MAIN_QUEUE_INDEX = 0;
static inline constexpr u64 FIRST_COMPUTE_QUEUE_IDX = 1;
static inline constexpr u64 FIRST_TRANSFER_QUEUE_IDX = FIRST_COMPUTE_QUEUE_IDX + DAXA_MAX_COMPUTE_QUEUE_COUNT;

struct daxa_ImplDevice final : public ImplHandle
{
    // General data:
    daxa_Instance instance = {};
    DeviceInfo2 info = {};
    VkPhysicalDevice vk_physical_device = {};
    daxa_DeviceProperties properties = {};
    PhysicalDeviceFeaturesStruct physical_device_features = {};
    VkDevice vk_device = {};
    VmaAllocator vma_allocator = {};

#if DAXA_USE_DYNAMIC_VULKAN
    // Platform specific surface functions
#if defined(_WIN32)
    PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
#endif
#if defined(__linux__)
    PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR;
    PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR;
#endif

    // Swapchain functions:
    PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
    PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
    PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
    PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
    PFN_vkQueuePresentKHR vkQueuePresentKHR;
    
    // Device functions:
    PFN_vkGetDeviceQueue vkGetDeviceQueue;
    PFN_vkCreateCommandPool vkCreateCommandPool;
    PFN_vkDestroyCommandPool vkDestroyCommandPool;
    PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
    PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
    PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
    PFN_vkEndCommandBuffer vkEndCommandBuffer;
    PFN_vkResetCommandBuffer vkResetCommandBuffer;
    PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
    PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
    PFN_vkCmdBindPipeline vkCmdBindPipeline;
    PFN_vkCmdSetViewport vkCmdSetViewport;
    PFN_vkCmdSetScissor vkCmdSetScissor;
    PFN_vkCmdDraw vkCmdDraw;
    PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
    PFN_vkCmdDrawIndirect vkCmdDrawIndirect;
    PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect;
    PFN_vkCmdDrawIndirectCount vkCmdDrawIndirectCount;
    PFN_vkCmdDrawIndexedIndirectCount vkCmdDrawIndexedIndirectCount;
    PFN_vkCmdDispatch vkCmdDispatch;
    PFN_vkCmdDispatchIndirect vkCmdDispatchIndirect;
    PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
    PFN_vkCmdCopyImage vkCmdCopyImage;
    PFN_vkCmdBlitImage vkCmdBlitImage;
    PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
    PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer;
    PFN_vkCmdUpdateBuffer vkCmdUpdateBuffer;
    PFN_vkCmdFillBuffer vkCmdFillBuffer;
    PFN_vkCmdClearColorImage vkCmdClearColorImage;
    PFN_vkCmdClearDepthStencilImage vkCmdClearDepthStencilImage;
    PFN_vkCmdResolveImage vkCmdResolveImage;
    PFN_vkCmdSetEvent vkCmdSetEvent;
    PFN_vkCmdResetEvent vkCmdResetEvent;
    PFN_vkCmdWaitEvents2 vkCmdWaitEvents2;
    PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
    PFN_vkCmdPushConstants vkCmdPushConstants;
    PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
    PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
    PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
    PFN_vkResetCommandPool vkResetCommandPool;
    PFN_vkCmdBeginRendering vkCmdBeginRendering;
    PFN_vkCmdEndRendering vkCmdEndRendering;
    PFN_vkCmdSetDepthBias vkCmdSetDepthBias;
    PFN_vkCmdSetEvent2 vkCmdSetEvent2;
    PFN_vkCmdWriteTimestamp2 vkCmdWriteTimestamp2;
    PFN_vkCmdResetQueryPool vkCmdResetQueryPool;
    
    // Memory management functions:
    PFN_vkAllocateMemory vkAllocateMemory;
    PFN_vkFreeMemory vkFreeMemory;
    PFN_vkMapMemory vkMapMemory;
    PFN_vkUnmapMemory vkUnmapMemory;
    PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
    PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges;
    PFN_vkBindBufferMemory vkBindBufferMemory;
    PFN_vkBindImageMemory vkBindImageMemory;
    PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
    PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
    PFN_vkGetDeviceBufferMemoryRequirements vkGetDeviceBufferMemoryRequirements;
    PFN_vkGetDeviceImageMemoryRequirements vkGetDeviceImageMemoryRequirements;
    
    // Resource creation/destruction functions:
    PFN_vkCreateBuffer vkCreateBuffer;
    PFN_vkDestroyBuffer vkDestroyBuffer;
    PFN_vkCreateImage vkCreateImage;
    PFN_vkDestroyImage vkDestroyImage;
    PFN_vkCreateImageView vkCreateImageView;
    PFN_vkDestroyImageView vkDestroyImageView;
    PFN_vkCreateSampler vkCreateSampler;
    PFN_vkDestroySampler vkDestroySampler;
    PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
    PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
    PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
    PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
    PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
    PFN_vkCreateComputePipelines vkCreateComputePipelines;
    PFN_vkDestroyPipeline vkDestroyPipeline;
    PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
    PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
    PFN_vkResetDescriptorPool vkResetDescriptorPool;
    PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
    PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
    PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
    
    // Synchronization functions:
    PFN_vkCreateSemaphore vkCreateSemaphore;
    PFN_vkDestroySemaphore vkDestroySemaphore;
    PFN_vkCreateFence vkCreateFence;
    PFN_vkDestroyFence vkDestroyFence;
    PFN_vkWaitForFences vkWaitForFences;
    PFN_vkResetFences vkResetFences;
    PFN_vkQueueSubmit vkQueueSubmit;
    PFN_vkQueueWaitIdle vkQueueWaitIdle;
    PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
    PFN_vkSignalSemaphore vkSignalSemaphore;
    PFN_vkWaitSemaphores vkWaitSemaphores;
    PFN_vkCreateEvent vkCreateEvent;
    PFN_vkDestroyEvent vkDestroyEvent;
    PFN_vkSetEvent vkSetEvent;
    PFN_vkResetEvent vkResetEvent;
    PFN_vkCmdPipelineBarrier2 vkCmdPipelineBarrier2;
    
    // Query functions:
    PFN_vkCreateQueryPool vkCreateQueryPool;
    PFN_vkDestroyQueryPool vkDestroyQueryPool;
    PFN_vkResetQueryPool vkResetQueryPool;
    PFN_vkGetQueryPoolResults vkGetQueryPoolResults;

    // Additional functions:
    PFN_vkGetSemaphoreCounterValue vkGetSemaphoreCounterValue;
    PFN_vkGetBufferDeviceAddress vkGetBufferDeviceAddress;

    // NOTE: more function references here
#endif

    // Dynamic State:
    PFN_vkCmdSetRasterizationSamplesEXT vkCmdSetRasterizationSamplesEXT = {};

    // Debug utils:
    PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = {};
    PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT = {};
    PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT = {};

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
    PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR = {};
    PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR = {};
    PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR = {};
    PFN_vkCmdTraceRaysIndirectKHR vkCmdTraceRaysIndirectKHR = {};

    // Push Descriptors:
    PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR = {};
    VkPhysicalDevicePushDescriptorPropertiesKHR push_descriptor_properties = {};

    // NVX binary import
    PFN_vkCmdCuLaunchKernelNVX vkCmdCuLaunchKernelNVX = {};
    PFN_vkCreateCuFunctionNVX vkCreateCuFunctionNVX = {};
    PFN_vkCreateCuModuleNVX vkCreateCuModuleNVX = {};
    PFN_vkDestroyCuFunctionNVX vkDestroyCuFunctionNVX = {};
    PFN_vkDestroyCuModuleNVX vkDestroyCuModuleNVX = {};

    // NVX image view handle
    PFN_vkGetImageViewHandleNVX vkGetImageViewHandleNVX = {};
    PFN_vkGetImageViewHandle64NVX vkGetImageViewHandle64NVX = {};
    PFN_vkGetImageViewAddressNVX vkGetImageViewAddressNVX = {};

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
    // Index with daxa_QueueFamily.
    std::array<CommandPoolPool, 3> command_pool_pools = {};

    // Gpu Shader Resource Object table:
    GPUShaderResourceTable gpu_sro_table = {};

    // Every submit to any queue increments the global submit timeline
    // Each queue stores a mapping between local submit index and global submit index for each of their in flight submits.
    // When destroying a resource it becomes a zombie, the zombie remembers the current global timeline value.
    // When collect garbage is called, the zombies timeline values are compared against submits running in all queues.
    // If the zombies global submit index is smaller then global index of all submits currently in flight (on all queues), we can safely clean the resource up.
    std::atomic_uint64_t global_submit_timeline = {};
    std::recursive_mutex zombies_mtx = {};
    std::deque<std::pair<u64, CommandRecorderZombie>> command_list_zombies = {};
    std::deque<std::pair<u64, BufferId>> buffer_zombies = {};
    std::deque<std::pair<u64, ImageId>> image_zombies = {};
    std::deque<std::pair<u64, ImageViewId>> image_view_zombies = {};
    std::deque<std::pair<u64, SamplerId>> sampler_zombies = {};
    std::deque<std::pair<u64, TlasId>> tlas_zombies = {};
    std::deque<std::pair<u64, BlasId>> blas_zombies = {};
    std::deque<std::pair<u64, SemaphoreZombie>> semaphore_zombies = {};
    std::deque<std::pair<u64, EventZombie>> split_barrier_zombies = {};
    std::deque<std::pair<u64, PipelineZombie>> pipeline_zombies = {};
    std::deque<std::pair<u64, TimelineQueryPoolZombie>> timeline_query_pool_zombies = {};
    std::deque<std::pair<u64, MemoryBlockZombie>> memory_block_zombies = {};

    // Queues
    struct ImplQueue
    {
        // Constant after initialization:
        daxa_QueueFamily family = {};
        u32 queue_index = {};
        u32 vk_queue_family_index = ~0u;
        VkQueue vk_queue = {};
        VkSemaphore gpu_queue_local_timeline = {};
        // atomically synchronized:
        std::atomic_uint64_t latest_pending_submit_timeline_value = {};

        auto initialize(daxa_Device device) -> daxa_Result;
        void cleanup(daxa_Device device);
        auto get_oldest_pending_submit(daxa_Device device, std::optional<u64> & out) -> daxa_Result;
    };
    std::array<ImplQueue, DAXA_MAX_COMPUTE_QUEUE_COUNT + DAXA_MAX_TRANSFER_QUEUE_COUNT + 1> queues = {
        ImplQueue{.family = DAXA_QUEUE_FAMILY_MAIN, .queue_index = 0},
        ImplQueue{.family = DAXA_QUEUE_FAMILY_COMPUTE, .queue_index = 0},
        ImplQueue{.family = DAXA_QUEUE_FAMILY_COMPUTE, .queue_index = 1},
        ImplQueue{.family = DAXA_QUEUE_FAMILY_COMPUTE, .queue_index = 2},
        ImplQueue{.family = DAXA_QUEUE_FAMILY_COMPUTE, .queue_index = 3},
        ImplQueue{.family = DAXA_QUEUE_FAMILY_TRANSFER, .queue_index = 0},
        ImplQueue{.family = DAXA_QUEUE_FAMILY_TRANSFER, .queue_index = 1},
    };

    auto get_queue(daxa_Queue queue) -> ImplQueue &;
    auto valid_queue(daxa_Queue queue) -> bool;

    struct ImplQueueFamily
    {
        u32 queue_count = {};
        u32 vk_index = ~0u;
    };
    std::array<ImplQueueFamily, 3> queue_families = {};

    std::array<u32, 3> valid_vk_queue_families = {};
    u32 valid_vk_queue_family_count = {};

    auto validate_image_slice(daxa_ImageMipArraySlice const & slice, daxa_ImageId id) -> daxa_ImageMipArraySlice;
    auto validate_image_slice(daxa_ImageMipArraySlice const & slice, daxa_ImageViewId id) -> daxa_ImageMipArraySlice;
    auto new_swapchain_image(VkImage swapchain_image, VkFormat format, u32 index, ImageUsageFlags usage, ImageInfo const & image_info, ImageId * out) -> daxa_Result;

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

    static auto create_2(daxa_Instance instance, daxa_DeviceInfo2 const & info, ImplPhysicalDevice const & physical_device, daxa_DeviceProperties const & properties, daxa_Device device) -> daxa_Result;
    static auto create(daxa_Instance instance, daxa_DeviceInfo const & info, VkPhysicalDevice physical_device, daxa_Device device) -> daxa_Result;
    static void zero_ref_callback(ImplHandle const * handle);

    auto load_device_functions() -> bool;
};
