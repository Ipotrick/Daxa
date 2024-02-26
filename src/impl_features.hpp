#pragma once

#include <daxa/daxa.hpp>
#include <daxa/device.hpp>

#include "impl_core.hpp"

#include "cpp_wrapper.hpp"
#include <daxa/c/device.h>

namespace daxa
{
    // NOTE(pahrens): Arbitrary limit. Bump if needed.
    static constexpr inline u32 EXTENSION_LIST_MAX = 32;

        struct PhysicalDeviceFeatureTable
    {
        VkPhysicalDeviceFeatures features = {};
        VkPhysicalDeviceVulkan12Features features12 = {};
        VkPhysicalDeviceBufferDeviceAddressFeatures buffer_device_address = {};
        VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing = {};
        VkPhysicalDeviceHostQueryResetFeatures host_query_reset = {};
        VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering = {};
        VkPhysicalDeviceSynchronization2Features sync2 = {};
        VkPhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore = {};
        VkPhysicalDeviceScalarBlockLayoutFeatures scalar_layout = {};
        VkPhysicalDeviceVariablePointerFeatures variable_pointers = {};
        std::optional<VkPhysicalDeviceVulkanMemoryModelFeatures> memory_model = {};
        std::optional<VkPhysicalDeviceShaderAtomicInt64Features> shader_atomic64 = {};
        std::optional<VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT> image_atomic64 = {};
        std::optional<VkPhysicalDeviceMeshShaderFeaturesEXT> mesh_shader = {};
        std::optional<VkPhysicalDeviceAccelerationStructureFeaturesKHR> acceleration_structure = {};
        std::optional<VkPhysicalDeviceRayTracingPipelineFeaturesKHR> ray_tracing_pipeline = {};
        std::optional<VkPhysicalDeviceRayQueryFeaturesKHR> ray_query = {};
        std::optional<VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR> ray_tracing_position_fetch = {};
        std::optional<VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV > ray_tracing_invocation_reorder = {};
        void * chain = {};

        void initialize(daxa_DeviceInfo info);
    };

    struct PhysicalDeviceExtensionList
    {
        char const * data[EXTENSION_LIST_MAX] = {};
        u32 size = {};

        void initialize(daxa_DeviceInfo info);
    };
} // namespace daxa
