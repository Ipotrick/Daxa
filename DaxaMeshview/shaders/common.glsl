struct GlobalData {
    mat4 perspective;
    mat4 view;
    mat4 vp;
    mat4 iView;
    vec4 cameraPosition;
	float verticalFOV;
	uint renderTargetWidth;
	uint renderTargetHeight;
    uint generalSamplerId;
    uint skyboxImageId;
};
layout(std430, set = 0, binding = 4) buffer GlobalDataBufferView{ 
    GlobalData globalData;
} globalDataBufferView[];

struct PrimitiveInfo {
    mat4 transform;
    mat4 ttTransform;
    uint albedoMapId;
    uint normalMapId;
    uint vertexPositionsId;
    uint vertexUVsId;
    uint vertexNormalsId;
};
layout(std430, set = 0, binding = 4) buffer PrimitiveDatasBufferView{ 
    PrimitiveInfo primitiveInfos[]; 
} primitiveDataBufferView[]; 


struct Light {
    vec3 position;
    float strength;
    vec4 color;
};
layout(std430, set = 0, binding = 4) buffer LightsBufferView{
    uint lightCount;
    Light lights[];
} lightsBufferView[];


layout(set = 0, binding = 4) buffer OrthLightBufferView{
    vec3 direction;
    uint shadowMap;
};


layout(set = 0, binding = 1) uniform sampler2D imageSampler2DViews[];