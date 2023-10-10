#include "impl_features.hpp"

namespace daxa
{
    void PhysicalDeviceFeatureTable::initialize(daxa_DeviceInfo info)
    {
        this->features = {
            .robustBufferAccess = VK_FALSE,
            .fullDrawIndexUint32 = VK_FALSE,
            .imageCubeArray = VK_TRUE,
            .independentBlend = VK_TRUE,
            .geometryShader = VK_FALSE,
            .tessellationShader = VK_TRUE,
            .sampleRateShading = VK_FALSE,
            .dualSrcBlend = VK_FALSE,
            .logicOp = VK_FALSE,
            .multiDrawIndirect = VK_TRUE, // Very useful for gpu driven rendering
            .drawIndirectFirstInstance = VK_FALSE,
            .depthClamp = VK_TRUE, // NOTE(msakmary) need self for bikeshed if breaks ping me
            .depthBiasClamp = VK_FALSE,
            .fillModeNonSolid = VK_TRUE,
            .depthBounds = VK_FALSE,
            .wideLines = VK_TRUE,
            .largePoints = VK_FALSE,
            .alphaToOne = VK_FALSE,
            .multiViewport = VK_FALSE,
            .samplerAnisotropy = VK_TRUE, // Allows for anisotropic filtering.
            .textureCompressionETC2 = VK_FALSE,
            .textureCompressionASTC_LDR = VK_FALSE,
            .textureCompressionBC = VK_FALSE,
            .occlusionQueryPrecise = VK_FALSE,
            .pipelineStatisticsQuery = VK_FALSE,
            .vertexPipelineStoresAndAtomics = VK_FALSE,
            .fragmentStoresAndAtomics = VK_TRUE,
            .shaderTessellationAndGeometryPointSize = VK_FALSE,
            .shaderImageGatherExtended = VK_FALSE,
            .shaderStorageImageExtendedFormats = VK_FALSE,
            .shaderStorageImageMultisample = VK_TRUE,            // Useful for software vrs.
            .shaderStorageImageReadWithoutFormat = VK_TRUE,      // This allows daxa shaders to not specify image layout for image binding tables and read ops.
            .shaderStorageImageWriteWithoutFormat = VK_TRUE,     // This allows daxa shaders to not specify image layout for image binding tables and write ops.
            .shaderUniformBufferArrayDynamicIndexing = VK_FALSE, // This is superseded by descriptor indexing.
            .shaderSampledImageArrayDynamicIndexing = VK_FALSE,  // This is superseded by descriptor indexing.
            .shaderStorageBufferArrayDynamicIndexing = VK_FALSE, // This is superseded by descriptor indexing.
            .shaderStorageImageArrayDynamicIndexing = VK_FALSE,  // This is superseded by descriptor indexing.
            .shaderClipDistance = VK_FALSE,
            .shaderCullDistance = VK_FALSE,
            .shaderFloat64 = VK_FALSE,
            .shaderInt64 = VK_TRUE, // Used for buffer device address math.
            .shaderInt16 = VK_FALSE,
            .shaderResourceResidency = VK_FALSE,
            .shaderResourceMinLod = VK_FALSE,
            .sparseBinding = VK_FALSE,
            .sparseResidencyBuffer = VK_FALSE,
            .sparseResidencyImage2D = VK_FALSE,
            .sparseResidencyImage3D = VK_FALSE,
            .sparseResidency2Samples = VK_FALSE,
            .sparseResidency4Samples = VK_FALSE,
            .sparseResidency8Samples = VK_FALSE,
            .sparseResidency16Samples = VK_FALSE,
            .sparseResidencyAliased = VK_FALSE,
            .variableMultisampleRate = VK_FALSE,
            .inheritedQueries = VK_FALSE,
        };
        this->chain = nullptr;
        this->buffer_device_address = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
            .pNext = chain,
            .bufferDeviceAddress = VK_TRUE,
            .bufferDeviceAddressCaptureReplay = static_cast<VkBool32>(info.flags & DAXA_DEVICE_FLAG_BUFFER_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT),
            .bufferDeviceAddressMultiDevice = VK_FALSE,
        };
        this->chain = reinterpret_cast<void *>(&this->buffer_device_address);
        this->descriptor_indexing = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
            .pNext = chain,
            .shaderInputAttachmentArrayDynamicIndexing = VK_FALSE,          // Daxa does not support sub-passes.
            .shaderUniformTexelBufferArrayDynamicIndexing = VK_FALSE,       // Daxa does not support texel buffers.
            .shaderStorageTexelBufferArrayDynamicIndexing = VK_FALSE,       // Daxa does not support texel buffers.
            .shaderUniformBufferArrayNonUniformIndexing = VK_FALSE,         // Tread uniform buffers as hard bindings.
            .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,           // Needed for bindless sampled images.
            .shaderStorageBufferArrayNonUniformIndexing = VK_TRUE,          // Needed for bindless buffers.
            .shaderStorageImageArrayNonUniformIndexing = VK_TRUE,           // Needed for bindless storage images.
            .shaderInputAttachmentArrayNonUniformIndexing = VK_FALSE,       // Daxa does not support sub-passes.
            .shaderUniformTexelBufferArrayNonUniformIndexing = VK_FALSE,    // Daxa does not support texel buffers.
            .shaderStorageTexelBufferArrayNonUniformIndexing = VK_FALSE,    // Daxa does not support texel buffers.
            .descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE,      // Tread uniform buffers as hard bindings.
            .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,        // Needed for bindless sampled images.
            .descriptorBindingStorageImageUpdateAfterBind = VK_TRUE,        // Needed for bindless storage images.
            .descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE,       // Needed for bindless buffers.
            .descriptorBindingUniformTexelBufferUpdateAfterBind = VK_FALSE, // Daxa does not support texel buffers.
            .descriptorBindingStorageTexelBufferUpdateAfterBind = VK_FALSE, // Daxa does not support texel buffers.
            .descriptorBindingUpdateUnusedWhilePending = VK_TRUE,           // Needed for bindless table updates.
            .descriptorBindingPartiallyBound = VK_TRUE,                     // Needed for sparse binding in bindless table.
            .descriptorBindingVariableDescriptorCount = VK_FALSE,           // No need for this, we have a static set.
            .runtimeDescriptorArray = VK_TRUE,                              // Allows shaders to not have a hardcoded descriptor maximum per table.
        };
        this->chain = reinterpret_cast<void *>(&this->descriptor_indexing);
        this->host_query_reset = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES,
            .pNext = this->chain,
            .hostQueryReset = VK_TRUE,
        };
        this->chain = reinterpret_cast<void *>(&this->host_query_reset);
        this->dynamic_rendering = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
            .pNext = this->chain,
            .dynamicRendering = VK_TRUE,
        };
        this->chain = reinterpret_cast<void *>(&this->dynamic_rendering);
        this->sync2 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR,
            .pNext = this->chain,
            .synchronization2 = VK_TRUE,
        };
        this->chain = reinterpret_cast<void *>(&this->sync2);
        this->timeline_semaphore = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES,
            .pNext = this->chain,
            .timelineSemaphore = VK_TRUE,
        };
        this->chain = reinterpret_cast<void *>(&this->timeline_semaphore);
        this->scalar_layout = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES,
            .pNext = this->chain,
            .scalarBlockLayout = VK_TRUE,
        };
        this->chain = reinterpret_cast<void *>(&this->scalar_layout);
        auto vk_memory_model = VkPhysicalDeviceVulkanMemoryModelFeatures{};
        if (info.flags & DAXA_DEVICE_FLAG_VK_MEMORY_MODEL)
        {
            vk_memory_model = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES,
                .pNext = this->chain,
                .vulkanMemoryModel = VK_TRUE,
                .vulkanMemoryModelDeviceScope = VK_TRUE,
                .vulkanMemoryModelAvailabilityVisibilityChains = VK_FALSE, // Low support.
            };
            this->chain = reinterpret_cast<void *>(&vk_memory_model);
        }
        if (info.flags & DAXA_DEVICE_FLAG_SHADER_ATOMIC64)
        {
            this->shader_atomic64 = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES,
                .pNext = this->chain,
                .shaderBufferInt64Atomics = VK_TRUE,
                .shaderSharedInt64Atomics = VK_TRUE,
            };
            this->chain = reinterpret_cast<void *>(&this->shader_atomic64);
        }
        if (info.flags & DAXA_DEVICE_FLAG_IMAGE_ATOMIC64)
        {
            this->image_atomic64 = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT,
                .pNext = this->chain,
                .shaderImageInt64Atomics = VK_TRUE,
                .sparseImageInt64Atomics = VK_FALSE, // I do not care about sparse images.
            };
            this->chain = reinterpret_cast<void *>(&this->image_atomic64);
        }
        if (info.flags & DAXA_DEVICE_FLAG_MESH_SHADER_BIT)
        {
            this->mesh_shader = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
                .pNext = this->chain,
                .taskShader = VK_TRUE,
                .meshShader = VK_TRUE,
                .multiviewMeshShader = VK_FALSE,
                .primitiveFragmentShadingRateMeshShader = VK_FALSE,
                .meshShaderQueries = VK_FALSE,
            };
            this->chain = reinterpret_cast<void *>(&this->mesh_shader);
        }
    }

    void PhysicalDeviceExtensionList::initialize(daxa_DeviceInfo info)
    {
        // NOTE(pahrens): Make sure to never exceed EXTENSION_LIST_MAX!
        this->size = 0;
        this->data[size++] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        this->data[size++] = {VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME};
        if (info.flags & DAXA_DEVICE_FLAG_CONSERVATIVE_RASTERIZATION)
        {
            this->data[size++] = {VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME};
        }
        if (info.flags & DAXA_DEVICE_FLAG_MESH_SHADER_BIT)
        {
            this->data[size++] = {VK_EXT_MESH_SHADER_EXTENSION_NAME};
        }
    }
} // namespace daxa