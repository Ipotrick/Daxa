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
    handle_daxa_result(daxa_create_instance(&(daxa_InstanceInfo){0}, &instance));

    daxa_destroy_instance(instance);
    return 0;
}
