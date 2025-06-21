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


    ImplTaskGraphMk2::ImplTaskGraphMk2(u32 task_memory_size)
        : task_memory{ "TaskGraph task memory pool", task_memory_size }
    {
    } 
}
#endif