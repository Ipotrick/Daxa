#define DAXA_ENABLE_SHADER_NO_NAMESPACE 1
#include "daxa/daxa.inl"

#include "test/test0.glsl"

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
    func();
}
