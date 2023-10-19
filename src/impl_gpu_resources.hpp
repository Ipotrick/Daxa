#pragma once

#include "impl_core.hpp"

#include <daxa/gpu_resources.hpp>

namespace daxa
{
    static const inline VkBufferUsageFlags BUFFER_USE_FLAGS =
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT |
        VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT |
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
        VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT |
        VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT |
        VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT |
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
        VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;

    static inline constexpr u32 BUFFER_BINDING = 0;
    static inline constexpr u32 STORAGE_IMAGE_BINDING = 1;
    static inline constexpr u32 SAMPLED_IMAGE_BINDING = 2;
    static inline constexpr u32 SAMPLER_BINDING = 3;
    static inline constexpr u32 BUFFER_DEVICE_ADDRESS_BUFFER_BINDING = 4;

    struct ImplBufferSlot
    {
        // Must be c version as these have ref counted dependencies that must be manually managed inside of daxa.
        daxa_BufferInfo info = {};
        std::string info_name = {};
        VkBuffer vk_buffer = {};
        VmaAllocation vma_allocation = {};
        VkDeviceAddress device_address = {};
        void * host_address = {};
    };

    static inline constexpr i32 NOT_OWNED_BY_SWAPCHAIN = -1;

    struct ImplImageViewSlot
    {
        // Must be c version as these have ref counted dependencies that must be manually managed inside of daxa.
        daxa_ImageViewInfo info = {};
        VkImageView vk_image_view = {};
    };

    struct ImplImageSlot
    {
        ImplImageViewSlot view_slot = {};
        // Must be c version as these have ref counted dependencies that must be manually managed inside of daxa.
        daxa_ImageInfo info = {};
        std::string info_name = {};
        VkImage vk_image = {};
        VmaAllocation vma_allocation = {};
        i32 swapchain_image_index = NOT_OWNED_BY_SWAPCHAIN;
        VkImageAspectFlags aspect_flags = {}; // Inferred from format.
    };

    struct ImplSamplerSlot
    {
        // Must be c version as these have ref counted dependencies that must be manually managed inside of daxa.
        daxa_SamplerInfo info = {};
        std::string info_name = {};
        VkSampler vk_sampler = {};
    };

    enum class TryZombiefyResult
    {
        INVALID_ID,
        HAS_REMAINING_READ_LOCKS,
        MAY_BE_ZOMBIEFIED,
    };

    /**
     * @brief GpuResourcePool is intended to be used akin to a specialized memory allocator, specific to gpu resource types (like image views).
     *
     * This struct is threadsafe if the following assumptions are met:
     * * never dereference a deleted resource
     * * never delete a resource twice
     * That means the function dereference_id can be used without synchronization, even calling get_new_slot or return_old_slot in parallel is safe.
     *
     * To check if these assumptions are met at runtime, the debug define DAXA_GPU_ID_VALIDATION can be enabled.
     * The define enables runtime checking to detect use after free and double free at the cost of performance.
     */
    template <typename ResourceT>
    struct GpuResourcePool
    {
        static constexpr inline usize MAX_RESOURCE_COUNT = 1u << 20u;
        static constexpr inline usize PAGE_BITS = 10u;
        static constexpr inline usize PAGE_SIZE = 1u << PAGE_BITS;
        static constexpr inline usize PAGE_MASK = PAGE_SIZE - 1u;
        static constexpr inline usize PAGE_COUNT = MAX_RESOURCE_COUNT / PAGE_SIZE;
        using VersionAndRefcntT = u64;
        using PageT = std::array<std::pair<ResourceT, VersionAndRefcntT>, PAGE_SIZE>;

        std::vector<u32> free_index_stack = {};
        u32 next_index = {};
        u32 max_resources = {};

        std::mutex page_alloc_mtx = {};
        mutable std::mutex mut = {};
        std::array<std::unique_ptr<PageT>, PAGE_COUNT> pages = {};
        u32 valid_page_count = {};

        struct ExclusiveAccess
        {
          private:
            GpuResourcePool<ResourceT> & self;
            std::lock_guard<std::mutex> l;

          public:
            ExclusiveAccess(GpuResourcePool<ResourceT> & pool) : self{pool}, l{pool.mut} {}

            /**
             * @brief   Creates a slot for a resource in the pool.
             *          Returned slots may be recycled but are guaranteed to have a unique index + version.
             *
             * Only Threadsafe when:
             * * mutable ptr to resource is only used in parallel
             *
             * @return The new resource slot and its id. Can fail if max resources is exceeded.
             */
            auto try_create_slot() -> std::optional<std::pair<GPUResourceId, ResourceT &>>
            {
                u32 index;
                if (self.free_index_stack.empty())
                {
                    index = self.next_index++;
                    if (index == self.max_resources || index == MAX_RESOURCE_COUNT)
                    {
                        return std::nullopt;
                    }
                }
                else
                {
                    index = self.free_index_stack.back();
                    self.free_index_stack.pop_back();
                }

                auto const page = static_cast<usize>(index) >> PAGE_BITS;
                auto const offset = static_cast<usize>(index) & PAGE_MASK;

                if (!self.pages[page])
                {
                    self.pages[page] = std::make_unique<PageT>();
                    for (u32 i = 0; i < PAGE_SIZE; ++i)
                    {
                        self.pages[page]->at(i).second = 0; // set all version numbers to 0 (invalid)
                    }
                    // Needs to be sequential, so that the 0 writes to the versions are visible before the atomic op.
                    std::atomic_ref{self.valid_page_count}.fetch_add(1, std::memory_order_seq_cst);
                }

                u64 refcnt_version = std::atomic_ref{self.pages[page]->at(offset).second}.load(std::memory_order_relaxed);
                u64 const refcnt = refcnt_version >> DAXA_ID_VERSION_BITS;
                u64 const version = std::max(refcnt_version & DAXA_ID_VERSION_BITS, 1ull); // Make sure version is at least one.
                
                // set version to 1 if its not already
                u64 expected = 0;
                std::atomic_ref{self.pages[page]->at(offset).second}.compare_exchange_weak(
                    expected, 1ull,
                    std::memory_order_relaxed
                );
                
                auto const id = GPUResourceId{.index = static_cast<u64>(index), .version = static_cast<u64>(version)};
                return std::optional{std::pair<GPUResourceId, ResourceT &>(id, self.pages[page]->at(offset).first)};
            }

            /**
             * @brief   Destroyes a slot.
             *          After calling this function, the id of the slot will be forever invalid.
             *          Index may be recycled but index + version pairs are always unique.
             *
             * Always threadsafe.
             * Calling this function with a non zombie id will result in undefined behavior.
             * WARNING: Not calling unsafe_destroy_zombie_slot at some point on a zombiefied resource causes slot leaking!
             */
            void unsafe_destroy_zombie_slot(GPUResourceId id)
            {
                auto const page = static_cast<usize>(id.index) >> PAGE_BITS;
                auto const offset = static_cast<usize>(id.index) & PAGE_MASK;
                auto const refcnt_version = std::atomic_ref{self.pages[page]->at(offset).second}.load(std::memory_order_relaxed);
                u64 const version = refcnt_version & DAXA_ID_VERSION_MASK;
                // Slots that reached max version CAN NOT be recycled.
                // That is because we can not guarantee uniqueness of ids when the version wraps back to 0.
                if (version != DAXA_ID_VERSION_MASK /* this is the maximum value a version is allowed to reach */)
                {
                    self.free_index_stack.push_back(id.index);
                }
                // Clear slot:
                self.pages[page]->at(offset).first = {};
            }
        };

        auto try_zombiefy(GPUResourceId id) -> TryZombiefyResult
        {
            auto const page = static_cast<usize>(id.index) >> PAGE_BITS;
            if (page >= std::atomic_ref{this->valid_page_count}.load(std::memory_order_relaxed))
            {
                return TryZombiefyResult::INVALID_ID;
            }
            auto const offset = static_cast<usize>(id.index) & PAGE_MASK;
            bool success = false;
            u64 refcnt = {};
            while (!success)
            {
                u64 refcnt_version = std::atomic_ref{(*this->pages[page])[offset].second}.load(std::memory_order_relaxed);
                refcnt = refcnt_version >> DAXA_ID_VERSION_BITS;
                u64 const version = refcnt_version & DAXA_ID_VERSION_MASK;
                u64 const new_version = new_version + 1;
                if (version == id.version)
                {
                    u64 const new_refcnt_version = (refcnt << DAXA_ID_VERSION_BITS) | new_version;
                    success = std::atomic_ref{(*this->pages[page])[offset].second}.compare_exchange_weak(
                        refcnt_version, new_refcnt_version,
                        std::memory_order_relaxed);
                }
                else
                {
                    return TryZombiefyResult::INVALID_ID;
                }
            }
            if (refcnt == 0)
            {
                return TryZombiefyResult::MAY_BE_ZOMBIEFIED;
            }
            else
            {
                return TryZombiefyResult::HAS_REMAINING_READ_LOCKS;
            }
        }

        auto try_read_lock(GPUResourceId id) -> bool
        {
            // refcnt is in upper bits, version in lower bits.
            auto const page = static_cast<usize>(id.index) >> PAGE_BITS;
            // clamp page to valid index
            if (page >= std::atomic_ref{this->valid_page_count}.load(std::memory_order_relaxed))
            {
                return false;
            }
            auto const offset = static_cast<usize>(id.index) & PAGE_MASK;
            bool success = false;
            u64 new_refcnt = {};
            while (!success)
            {
                u64 refcnt_version = std::atomic_ref{(*this->pages[page])[offset].second}.load(std::memory_order_relaxed);
                u64 const refcnt = refcnt_version >> DAXA_ID_VERSION_BITS;
                u64 const version = refcnt_version & DAXA_ID_VERSION_MASK;
                new_refcnt = refcnt + 1;
                if (version == id.version && new_refcnt < ((1ull << 20) - 1ull))
                {
                    u64 const new_refcnt_version = (new_refcnt << DAXA_ID_VERSION_BITS) | version;
                    success = std::atomic_ref{(*this->pages[page])[offset].second}.compare_exchange_weak(
                        refcnt_version, new_refcnt_version,
                        std::memory_order_relaxed);
                }
                else
                {
                    return false;
                }
            }
            return true;
        }

        /// @return if resource is zombie AND has zero read locks.
        auto read_unlock(GPUResourceId id) -> bool
        {
            auto const page = static_cast<usize>(id.index) >> PAGE_BITS;
            auto const offset = static_cast<usize>(id.index) & PAGE_MASK;
            // dec refcnt
            u64 const refcnt_version = std::atomic_ref{(*this->pages[page])[offset].second}.fetch_sub(1ull << DAXA_ID_VERSION_BITS, std::memory_order_relaxed);
            u64 const refcnt = refcnt_version >> DAXA_ID_VERSION_BITS;
            u64 const version = refcnt_version & DAXA_ID_VERSION_MASK;
            // version != id.version => user zombified object
            // refcnt == 0 => no read locks remaining
            //   => we must zombiefy
            return version != id.version && refcnt == 0;
        }

        /**
         * @brief   Returns exclusive accessor.
         *
         * Always threadsafe.
         * @return exclusive accessor.
         */
        auto exclusive() -> ExclusiveAccess
        {
            return ExclusiveAccess{*this};
        }

        /**
         * @brief   Checks if an id referes to a valid resource.
         *
         * Always threadsafe.
         * @returns if the given id is valid.
         */
        auto is_id_valid(GPUResourceId id) const -> bool
        {
            auto const page = static_cast<usize>(id.index) >> PAGE_BITS;
            auto const offset = static_cast<usize>(id.index) & PAGE_MASK;
            if (id.version == 0 || page >= static_cast<usize>(std::atomic_ref{this->valid_page_count}.load(std::memory_order_relaxed)))
            {
                return false;
            }
            u64 const refcnt_version = std::atomic_ref{(*this->pages[page])[offset].second}.load(std::memory_order_relaxed);
            u64 const refcnt = refcnt_version >> DAXA_ID_VERSION_BITS;
            u64 const version = refcnt_version & DAXA_ID_VERSION_MASK;
            return refcnt != 0 && version == id.version;
        }

        /**
         * @brief   Checks if an id referes to a valid resource.
         *
         * Only Threadsafe when:
         * * resource is not destroyed before the reference is used for the last time.
         *
         * Calling this function with an invalid id will result in undefined behavior.
         * @returns resource.
         */
        auto unsafe_get(GPUResourceId id) const -> ResourceT const &
        {
            auto const page = static_cast<usize>(id.index) >> PAGE_BITS;
            auto const offset = static_cast<usize>(id.index) & PAGE_MASK;
            return pages[page]->at(offset).first;
        }
    };

    struct GPUShaderResourceTable
    {
        GpuResourcePool<ImplBufferSlot> buffer_slots = {};
        GpuResourcePool<ImplImageSlot> image_slots = {};
        GpuResourcePool<ImplSamplerSlot> sampler_slots = {};

        VkDescriptorSetLayout vk_descriptor_set_layout = {};
        VkDescriptorSetLayout uniform_buffer_descriptor_set_layout = {};
        VkDescriptorSet vk_descriptor_set = {};
        VkDescriptorPool vk_descriptor_pool = {};

        // Contains pipeline layouts with varying push constant range size.
        // The first size is 0 word, second is 1 word, all others are a power of two (maximum is MAX_PUSH_CONSTANT_BYTE_SIZE).
        std::array<VkPipelineLayout, PIPELINE_LAYOUT_COUNT> pipeline_layouts = {};

        void initialize(u32 max_buffers, u32 max_images, u32 max_samplers, VkDevice device, VkBuffer device_address_buffer, PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT);
        void cleanup(VkDevice device);
    };

    void write_descriptor_set_sampler(VkDevice vk_device, VkDescriptorSet vk_descriptor_set, VkSampler vk_sampler, u32 index);

    void write_descriptor_set_buffer(VkDevice vk_device, VkDescriptorSet vk_descriptor_set, VkBuffer vk_buffer, VkDeviceSize offset, VkDeviceSize range, u32 index);

    void write_descriptor_set_image(VkDevice vk_device, VkDescriptorSet vk_descriptor_set, VkImageView vk_image_view, ImageUsageFlags usage, u32 index);
} // namespace daxa
