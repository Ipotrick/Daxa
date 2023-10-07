#ifndef __DAXA_CORE_H__
#define __DAXA_CORE_H__

#include <stdint.h>

#if defined(__cplusplus)
#define DAXA_EXPORT extern "C"
#else
#define DAXA_EXPORT
#endif

typedef struct daxa_ImplHandle * daxa_Handle;
typedef struct daxa_ImplDevice * daxa_Device;
typedef struct daxa_ImplCommandList * daxa_CommandList;
typedef struct daxa_ImplBakedCommands * daxa_BakedCommands;
typedef struct daxa_ImplInstance * daxa_Instance;
typedef struct daxa_ImplComputePipeline * daxa_ComputePipeline;
typedef struct daxa_ImplRasterPipeline * daxa_RasterPipeline;
typedef struct daxa_ImplSwapchain * daxa_Swapchain;
typedef struct daxa_ImplBinarySemaphore * daxa_BinarySemaphore;
typedef struct daxa_ImplTimelineSemaphore * daxa_TimelineSemaphore;
typedef struct daxa_ImplEvent * daxa_Event;
typedef struct daxa_ImplTimelineQueryPool * daxa_TimelineQueryPool;
typedef struct daxa_ImplMemoryBlock * daxa_MemoryBlock;

DAXA_EXPORT uint64_t
daxa_refcnt_inc(daxa_Handle handle);
DAXA_EXPORT uint64_t
daxa_refcnt_dec(daxa_Handle handle);

typedef uint64_t daxa_Flags;

typedef char daxa_Bool8;

typedef struct
{
    char const * data;
    size_t size;
} daxa_StringView;

#endif
