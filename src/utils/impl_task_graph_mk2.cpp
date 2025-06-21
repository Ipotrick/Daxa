#if DAXA_BUILT_WITH_UTILS_TASK_GRAPH

#include "../impl_core.hpp"

#include "impl_task_graph_mk2.hpp"

namespace daxa
{
    ///
    /// === Public API Implementation ===
    ///


    ///
    /// === Internals ===
    ///

    void test_DynamicArenaArray8k()
    {
        std::array<u8, 256> mem = {};
        MemoryArena arena = { "Test Arena", mem };

        DynamicArenaArray8k<u32> a{&arena};

        for (u32 i = 0; i < 5; ++i)
            a.push_back(i);
        
        printf("a init 0:\n");
        for (auto e : a)
        {
            printf("  %i\n", e);
        }

        a.insert(a.begin() + 2, 128);
        
        
        printf("a insert 128 at 2:\n");
        for (auto e : a)
        {
            printf("  %i\n", e);
        }
        
        for (u32 i = 8; i < 10; ++i)
            a.push_back(i);
        
        printf("a init 2:\n");
        for (auto e : a)
        {
            printf("  %i\n", e);
        }

        DynamicArenaArray8k<u32> b{&arena};

        for (u32 i = 5; i < 8; ++i)
            b.push_back(i);
        
        printf("b init:\n");
        for (auto e : a)
        {
            printf("  %i\n", e);
        }

        a.insert(a.begin() + 6, b.begin(), b.end());
        printf("a after b insert into a at 6:\n");
        for (auto e : a)
        {
            printf("  %i\n", e);
        }

        a.erase(a.begin() + 2);
        printf("a after erase 2:\n");
        for (auto e : a)
        {
            printf("  %i\n", e);
        }
        printf("hurray!\n");
    }

    ImplTaskGraphMk2::ImplTaskGraphMk2(u32 task_memory_size)
        : task_memory{ "TaskGraph task memory pool", task_memory_size }
    {
    } 
}
#endif