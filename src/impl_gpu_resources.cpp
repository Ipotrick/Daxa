#include "impl_gpu_resources.hpp"

namespace daxa
{
    void GPUResourceTable::init(usize max_buffers, usize max_images, usize max_samplers, VkDevice device)
    {
        buffer_slots.max_resources = max_buffers;
        image_slots.max_resources = max_images;
        sampler_slots.max_resources = max_samplers;
    }

    void GPUResourceTable::cleanup(VkDevice device)
    {
    }
} // namespace daxa
