#ifndef __DAXA_INSTANCE_H__
#define __DAXA_INSTANCE_H__

#include <daxa/c/core.h>
#include <daxa/c/device.h>

struct daxa_ImplDevice;
typedef struct daxa_ImplDevice* daxa_Device;

typedef struct
{

} daxa_InstanceInfo;

daxa_Instance
daxa_create_instance(daxa_InstanceInfo const * info);

daxa_Result
daxa_create_device(daxa_Instance instance, daxa_DeviceInfo const * info, daxa_Device * out_device);

daxa_Result
daxa_destroy_device(daxa_Instance instance, daxa_Device device);

#endif // #ifndef __DAXA_INSTANCE_H__