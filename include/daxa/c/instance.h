#ifndef __DAXA_INSTANCE_H__
#define __DAXA_INSTANCE_H__

#include <daxa/c/device.h>

struct daxa_ImplInstance;
typedef struct daxa_ImplInstance* daxa_Instance;

typedef enum
{
    DAXA_INSTANCE_FLAG_DEBUG_UTIL = 0x1,
    DAXA_INSTANCE_FLAG_MAX_ENUM = 0xFFFFFFFF,
} daxa_InstanceFlagBits;
typedef uint64_t daxa_InstanceFlags;

typedef struct
{
    daxa_InstanceFlags flags;
} daxa_InstanceInfo;

daxa_Instance
daxa_create_instance(daxa_InstanceInfo const * info);

daxa_Result
daxa_instance_create_device(daxa_Instance instance, daxa_DeviceInfo const * info, daxa_Device * out_device);

daxa_Result
daxa_instance_destroy_device(daxa_Instance instance, daxa_Device device);

daxa_InstanceInfo const *
daxa_instance_info(daxa_Instance instance);

VkInstance
daxa_instance_get_vk_instance(daxa_Instance instance);

#endif // #ifndef __DAXA_INSTANCE_H__