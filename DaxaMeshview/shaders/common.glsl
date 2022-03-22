

struct GlobalData {
    mat4 vp;
    mat4 view;
    mat4 itvp;
    mat4 itview;
};
layout(std430, set = 0, binding = 4) buffer GlobalDataBufferView{ GlobalData globalData; } globalDataBufferView[];

struct PrimitiveInfo {
    mat4 transform;
    mat4 inverseTransposeTransform;
};
layout(std430, set = 0, binding = 4) buffer PrimitiveDatasBufferView{ PrimitiveInfo primitiveInfos[]; } primitiveDataBufferView[]; 

struct Light {
    vec3 position;
    float strength;
    vec4 color;
};
layout(std430, set = 0, binding = 4) buffer LightsBufferView{
    uint lightCount;
    Light lights[];
} lightsBufferView[];
layout(std140, set = 0, binding = 4) buffer OrthLightBufferView{
    vec3 direction;
    uint16_t shadowMap;
};
layout(set = 0, binding = 1) uniform sampler2D imageSampler2DViews[];