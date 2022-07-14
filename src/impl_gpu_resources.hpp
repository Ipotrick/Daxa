#pragma once

#include "impl_core.hpp"

#include <daxa/gpu_resources.hpp>

namespace daxa
{
    struct ImplBufferSlot
    {
        BufferInfo info = {};
    };

    static inline constexpr i32 NOT_OWNED_BY_SWAPCHAIN = -1;

    struct ImplImageSlot
    {
        ImageInfo info = {};
        VkImageView vk_image_view_handle = {};
        VkImage vk_image_handle = {};
        i32 swapchain_image_index = NOT_OWNED_BY_SWAPCHAIN;
    };

    struct ImplImageViewSlot
    {
        ImageViewInfo info = {};
    };

    struct ImplSamplerSlot
    {
        SamplerInfo info = {};
    };

#define DAXA_DEBUG_GPU_VALIDATE_GPU_ID 1

    /**
     * @brief GpuResourcePool is intended to be used akin to a specialized memory allocator, specific to gpu resource types (like image views).
     *
     * This struct is threadsafe if the following assumptions are met:
     * * never dereference a deleted resource
     * * never delete a resource twice
     * That means the function dereference_id can be used without synchonization, even calling get_new_slot or return_old_slot in parallel is safe.
     *
     * To check if these assumptions are met at runtime, the debug define DAXA_DEBUG_GPU_VALIDATE_GPU_ID can be used.
     * The define enables runtime checking to detect use after free and double free at the cost of performance.
     */
    template <typename ResourceT, usize MAX_RESOURCE_COUNT = 1u << 20u>
    struct GpuResourcePool
    {
        static constexpr inline usize PAGE_BITS = 12u;
        static constexpr inline usize PAGE_SIZE = 1u << PAGE_BITS;
        static constexpr inline usize PAGE_MASK = PAGE_SIZE - 1u;
        static constexpr inline usize PAGE_COUNT = MAX_RESOURCE_COUNT / PAGE_SIZE;

        using PageT = std::array<std::pair<ResourceT, u8>, PAGE_SIZE>;

        std::vector<u32> free_index_stack = {};
        u32 next_index = {};
        usize max_resources = {};
        std::mutex page_alloc_mtx = {};
#ifdef DAXA_DEBUG_GPU_VALIDATE_GPU_ID
        std::mutex use_after_free_check_mtx = {};
#endif
        std::array<std::unique_ptr<PageT>, PAGE_COUNT> pages = {};

#ifdef DAXA_DEBUG_GPU_VALIDATE_GPU_ID
        void verify_ressource_id(GPUResourceId id)
        {
            size_t page = id.index >> PAGE_BITS;
            size_t offset = id.index & PAGE_MASK;
            DAXA_DBG_ASSERT_TRUE_M(pages[page] != nullptr, "detected invalid ressource id");
            DAXA_DBG_ASSERT_TRUE_M(id.version != 0, "detected invalid resource id");
        }
#endif

        auto new_slot() -> std::pair<GPUResourceId, ResourceT &>
        {
#ifdef DAXA_DEBUG_GPU_VALIDATE_GPU_ID
            std::unique_lock use_after_free_check_lock{use_after_free_check_mtx};
#endif
            std::unique_lock page_alloc_lock{page_alloc_mtx};
            u32 index = {};
            if (free_index_stack.empty())
            {
                index = next_index++;
                DAXA_DBG_ASSERT_TRUE_M(index < MAX_RESOURCE_COUNT, "exceded max resource count");
                DAXA_DBG_ASSERT_TRUE_M(index < max_resources, "exceded max resource count");
            }
            else
            {
                index = free_index_stack.back();
                free_index_stack.pop_back();
            }

            size_t page = index >> PAGE_BITS;
            size_t offset = index & PAGE_MASK;

            if (!pages[page])
            {
                pages[page] = std::make_unique<PageT>();
                for (u32 i = 0; i < PAGE_SIZE; ++i)
                {
                    pages[page]->at(i).second = 0; // set all version numbers to 0 (invalid)
                }
            }

            pages[page]->at(offset).second = std::max<u8>(pages[page]->at(index).second, 1); // make sure the version is at least one

            u8 version = pages[page]->at(offset).second;

            return {GPUResourceId{.index = index, .version = version}, pages[page]->at(offset).first};
        }

        auto return_slot(GPUResourceId id)
        {
            size_t page = id.index >> PAGE_BITS;
            size_t offset = id.index & PAGE_MASK;

#ifdef DAXA_DEBUG_GPU_VALIDATE_GPU_ID
            std::unique_lock use_after_free_check_lock{use_after_free_check_mtx};
            verify_ressource_id(id);
            DAXA_DBG_ASSERT_TRUE_M(pages[page]->at(offset).second == id.version, "detected double delete for a resource id");
#endif
            std::unique_lock page_alloc_lock{page_alloc_mtx};

            pages[page]->at(offset).second = std::max<u8>(pages[page]->at(offset).second + 1, 1); // the max is needed, as version = 0 is invalid

            free_index_stack.push_back(id.index);
        }

        auto dereference_id(GPUResourceId id) -> ResourceT &
        {
            size_t page = id.index >> PAGE_BITS;
            size_t offset = id.index & PAGE_MASK;

#ifdef DAXA_DEBUG_GPU_VALIDATE_GPU_ID
            std::unique_lock use_after_free_check_lock{use_after_free_check_mtx};
            verify_ressource_id(id);
            DAXA_DBG_ASSERT_TRUE_M(pages[page]->at(offset).second == id.version, "detected use after free for a resource id");
#endif
            return pages[page]->at(offset).first;
        }
    };

    struct GPUResourceTable
    {
        GpuResourcePool<ImplBufferSlot> buffer_slots = {};
        GpuResourcePool<std::variant<ImplImageSlot, ImplImageViewSlot, std::monostate>> image_slots = {};
        GpuResourcePool<ImplSamplerSlot> sampler_slots = {};

        VkDescriptorSetLayout vk_descriptor_set_layout_handle = {};
        VkDescriptorSet vk_descriptor_set_handle = {};
        VkDescriptorPool vk_descriptor_pool_handle = {};

        void init(usize max_buffers, usize max_images, usize max_samplers, VkDevice device);
        void cleanup(VkDevice device);
    };
} // namespace daxa
