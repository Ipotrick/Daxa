#ifndef __DAXA_INSTANCE_H__
#define __DAXA_INSTANCE_H__

#include <daxa/c/device.h>

typedef daxa_Flags daxa_InstanceFlags;
static const daxa_InstanceFlags DAXA_INSTANCE_FLAG_DEBUG_UTIL = 0x1;

typedef struct
{
    daxa_InstanceFlags flags;
} daxa_InstanceInfo;

DAXA_EXPORT daxa_Result
daxa_create_instance(daxa_InstanceInfo const * info, daxa_Instance *out_instance);

DAXA_EXPORT daxa_Result
daxa_instance_create_device(daxa_Instance instance, daxa_DeviceInfo const * info, daxa_Device * out_device);

// Returns previous ref count.
DAXA_EXPORT uint64_t
daxa_instance_inc_refcnt(daxa_Instance instance);
// Returns previous ref count.
DAXA_EXPORT uint64_t
daxa_instance_dec_refcnt(daxa_Instance instance);

DAXA_EXPORT daxa_InstanceInfo const *
daxa_instance_info(daxa_Instance instance);

DAXA_EXPORT VkInstance
daxa_instance_get_vk_instance(daxa_Instance instance);

#endif // #ifndef __DAXA_INSTANCE_H__
