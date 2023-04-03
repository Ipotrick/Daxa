#define DAXA_ENABLE_IMAGE_OVERLOADS_BASIC 1
#include <daxa/daxa.inl>

#include "shader_integration.inl"

layout(push_constant, scalar) uniform PushConstant
{
    ShaderIntegrationTaskListUses resources;
};

layout(local_size_x = 16, local_size_y = 16) in;
void main()
{
    const ivec3 index = ivec3(gl_GlobalInvocationID);
    const float set_value = deref(resources.settings).set_value;
    imageStore(resources.image, index, vec4(set_value,0,0,0));
}