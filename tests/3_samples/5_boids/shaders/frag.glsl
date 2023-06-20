#define DAXA_ENABLE_SHADER_NO_NAMESPACE 1
#include <shared.inl>

DAXA_DECL_PUSH_CONSTANT(DrawPushConstant, push)

layout(location = 0) out vec4 color;

void main()
{
    color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
