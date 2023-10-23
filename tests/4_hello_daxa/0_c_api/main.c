#include <daxa/c/daxa.h>
#include <stdio.h>
#include <stdlib.h>

void handle_daxa_result(daxa_Result res)
{
    if (res != DAXA_RESULT_SUCCESS)
    {
        printf("DAXA ERROR: %d\n", (int)res);
        exit(-1);
    }
}

int main(void)
{
    daxa_Instance instance;
    handle_daxa_result(daxa_create_instance(&DAXA_DEFAULT_INSTANCE_INFO, &instance));

    daxa_Device device;
    daxa_DeviceInfo dinfo = DAXA_DEFAULT_DEVICE_INFO;
    dinfo.name = (daxa_StringView){.data = "name", .size = 4};
    handle_daxa_result(daxa_instance_create_device(instance, &dinfo, &device));

    daxa_dvc_dec_refcnt(device);
    daxa_instance_dec_refcnt(instance);
    return 0;
}
