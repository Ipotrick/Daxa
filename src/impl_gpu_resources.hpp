#pragma once

#include "impl_core.hpp"

#include <daxa/gpu_resources.hpp>

namespace daxa
{
    template<typename RessourceT, typename ResourceIdT, usize MAX_N>
    struct GpuRessourcePool
    {
        static constexpr inline usize MAX_RESSOURCE_COUNT = 1u << 20u;
        static constexpr inline usize PAGE_BITS = 12u;
        static constexpr inline usize PAGE_SIZE = 1u << PAGE_BITS;
        static constexpr inline usize PAGE_MASK = PAGE_SIZE - 1u;
        static constexpr inline usize PAGE_COUNT = MAX_RESSOURCE_COUNT / PAGE_SIZE;

        using PageT = std::array<std::pair<RessourceT, u8>, PAGE_SIZE>;

        std::vector<u32> free_index_stack = {};
        u32 next_index = {};
        u32 max_ressources = {};
        std::mutex page_alloc_mtx = {};
        std::array<std::unique_ptr<PageT>, PAGE_COUNT> pages = {};

        auto get_new_slot() -> std::pair<ResourceIdT, RessourceT&>
        {
            std::unique_lock lock{ mtx };
            u32 index = {};
            if (free_index_stack.empty())
            {
                index = next_index++;
                DAXA_DBG_ASSERT_TRUE_M(index < MAX_RESSOURCE_COUNT, "exceded max ressource count");
                DAXA_DBG_ASSERT_TRUE_M(index < max_ressources, "exceded max ressource count");
            }
            else
            {
                index = freeIndexStack.back();
                freeIndexStack.pop_back();
            }

            size_t page = index >> PAGE_BITS;
            size_t offset = index & PAGE_MASK;

            if (!pages[page]) {
                pages[page] = std::make_unique<PageT>();
                for (u32 i = 0; i < PAGE_SIZE: ++i)
                {
                    pages[page]->at(i).second = 1;
                }
            }

            u8 version = pages[page]->at(index).second;

            return HandleT{ .index = index, .version = version };
        }
    };
}