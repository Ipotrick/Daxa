#define DAXA_RAY_TRACING 1
#extension GL_EXT_ray_tracing : enable
#if ATOMIC_FLOAT == 1
#extension GL_EXT_shader_atomic_float : enable
#endif
#include <daxa/daxa.inl>

#include "shared.inl"
#include "random.glsl"

#if SER_ON == 1
#extension GL_NV_shader_invocation_reorder : enable
layout(location = 0) hitObjectAttributeNV vec3 hitValue;
#endif

DAXA_DECL_PUSH_CONSTANT(PushConstant, p)

#if DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_RAYGEN

layout(location = 0) rayPayloadEXT hitPayload prd;


#if PRIMARY_RAYS == 1
    void main()
{
    const ivec2 index = ivec2(gl_LaunchIDEXT.xy);

    uint cull_mask = 0xff;

    daxa_f32vec3 color = vec3(0);

    // Camera setup
    daxa_f32mat4x4 inv_view = deref(p.camera_buffer).inv_view;
    daxa_f32mat4x4 inv_proj = deref(p.camera_buffer).inv_proj;

    const vec2 pixelCenter = vec2(gl_LaunchIDEXT.xy) + vec2(0.5);
    const vec2 inUV = pixelCenter / vec2(gl_LaunchSizeEXT.xy);
    vec2 d = inUV * 2.0 - 1.0;

    vec4 origin = inv_view * vec4(0, 0, 0, 1);
    vec4 target = inv_proj * vec4(d.x, d.y, 1, 1);
    vec4 direction = inv_view * vec4(normalize(target.xyz), 0);

    uint rayFlags = gl_RayFlagsNoneEXT;
    float tMin = 0.0001;
    float tMax = 10000.0;
    uint cullMask = 0xFF;

#if SER_ON == 1
    hitObjectNV hitObject;
    //Initialize to an empty hit object
    hitObjectRecordEmptyNV(hitObject);

    // Trace the ray
    hitObjectTraceRayNV(hitObject,
                        daxa_accelerationStructureEXT(p.tlas), // topLevelAccelerationStructure
                        rayFlags,      // rayFlags
                        cullMask,      // cullMask
                        0,             // sbtRecordOffset
                        0,             // sbtRecordStride
                        0,             // missIndex
                        origin.xyz,    // ray origin
                        tMin,          // ray min range
                        direction.xyz, // ray direction
                        tMax,          // ray max range
                        0              // payload (location = 0)
    );
    
    if(hitObjectIsHitNV(hitObject))
    { 
        daxa_f32 tHit = hitObjectGetRayTMaxNV(hitObject);
        color = 1.0 / tHit * vec3(1.0, 1.0, 1.0);
#if ATOMIC_FLOAT == 1
        atomicAdd(deref(p.camera_buffer).hit_count, 1);
#endif
    }

#else
    
#extension GL_EXT_ray_query : enable

    rayQueryEXT rayQuery;
    rayQueryInitializeEXT(
        rayQuery, daxa_accelerationStructureEXT(p.tlas),
        rayFlags,
        cullMask, // cullMask
        origin.xyz, tMin, direction.xyz, tMax);

    while (rayQueryProceedEXT(rayQuery))
    {
        uint type = rayQueryGetIntersectionTypeEXT(rayQuery, false);
        if (type == gl_RayQueryCandidateIntersectionTriangleEXT)
        {
            rayQueryConfirmIntersectionEXT(rayQuery);
        }
        else if (type == gl_RayQueryCandidateIntersectionAABBEXT)
        {
            rayQueryGenerateIntersectionEXT(rayQuery, tMax);
        }
    }

    uint type = rayQueryGetIntersectionTypeEXT(rayQuery, true);

    if (type == gl_RayQueryCommittedIntersectionTriangleEXT)
    {
        // daxa_f32 tHit = rayQueryGetIntersectionTEXT(rayQuery, true);
        // vec2 barycentrics = rayQueryGetIntersectionBarycentricsEXT(rayQuery, true);
        // vec3 bary_color = vec3(barycentrics.x, barycentrics.y, 1.0 - barycentrics.x - barycentrics.y);
        // color = (bary_color * tHit) / (tMax * vec3(1.0, 1.0, 1.0));
        // color = bary_color;
        color = vec3(1.0, 1.0, 1.0);
#if ATOMIC_FLOAT == 1
        atomicAdd(deref(p.camera_buffer).hit_count, 1);
#endif
    }
    else if (type == gl_RayQueryCommittedIntersectionGeneratedEXT)
    {
        // daxa_f32 tHit = rayQueryGetIntersectionTEXT(rayQuery, true);
        // color = 1.0 / tHit * vec3(1.0, 1.0, 1.0);
        color = vec3(1.0, 1.0, 1.0);
#if ATOMIC_FLOAT == 1
        atomicAdd(deref(p.camera_buffer).hit_count, 1);
#endif
    }
#endif

    // imageStore(daxa_image2D(p.swapchain), index, fromLinear(vec4(prd.hitValue, 1.0)));
    imageStore(daxa_image2D(p.swapchain), index, vec4(color, 1.0));
    // imageStore(daxa_image2D(p.swapchain), index, vec4(1.0, 1.0, 1.0, 1.0));
}

#else 

const uint NBSAMPLES = 1;

void main()
{
    const ivec2 index = ivec2(gl_LaunchIDEXT.xy);

    uint cull_mask = 0xff;

    // Camera setup
    daxa_f32mat4x4 inv_view = deref(p.camera_buffer).inv_view;
    daxa_f32mat4x4 inv_proj = deref(p.camera_buffer).inv_proj;

    const vec2 pixelCenter = vec2(gl_LaunchIDEXT.xy) + vec2(0.5);
    const vec2 inUV = pixelCenter / vec2(gl_LaunchSizeEXT.xy);
    vec2 d = inUV * 2.0 - 1.0;

    vec4 origin = inv_view * vec4(0, 0, 0, 1);
    vec4 target = inv_proj * vec4(d.x, d.y, 1, 1);
    vec4 direction = inv_view * vec4(normalize(target.xyz), 0);

    uint rayFlags = gl_RayFlagsNoneEXT;
    float tMin = 0.0001;
    float tMax = 10000.0;
    uint cullMask = 0xFF;

#if SER_ON == 1
    hitObjectNV hitObject;
    //Initialize to an empty hit object
    hitObjectRecordEmptyNV(hitObject);

    // Trace the ray
    hitObjectTraceRayNV(hitObject,
                        daxa_accelerationStructureEXT(p.tlas), // topLevelAccelerationStructure
                        rayFlags,      // rayFlags
                        cullMask,      // cullMask
                        0,             // sbtRecordOffset
                        0,             // sbtRecordStride
                        0,             // missIndex
                        origin.xyz,    // ray origin
                        tMin,          // ray min range
                        direction.xyz, // ray direction
                        tMax,          // ray max range
                        0              // payload (location = 0)
    );


    int mesh_id = 0;
    
    if(hitObjectIsHitNV(hitObject))
    { 
        mesh_id = hitObjectGetInstanceCustomIndexNV(hitObject);
    }
        

    // Reorder the ray (based on the hit object or whatever else user wants)
    reorderThreadNV(hitObject, mesh_id, 2);
    // reorderThreadNV(hitObject);
    // reorderThreadNV(mesh_id, 2);

    //Get Attributes
    hitObjectGetAttributesNV(hitObject, 0); // hitObjectAttributeNV hit_value
    // Execute either the closest hit or the miss shader
    hitObjectExecuteShaderNV(hitObject, 0); // hitObjectAttributeNV hit_value
    

#else
#if ATOMIC_FLOAT == 1
    prd.is_hit = false;
#endif
    traceRayEXT(
        daxa_accelerationStructureEXT(p.tlas), // topLevelAccelerationStructure
        rayFlags,      // rayFlags
        cullMask,      // cullMask
        0,             // sbtRecordOffset
        0,             // sbtRecordStride
        0,             // missIndex
        origin.xyz,    // ray origin
        tMin,          // ray min range
        direction.xyz, // ray direction
        tMax,          // ray max range
        0              // payload (location = 0)
    );
#endif

#if ATOMIC_FLOAT == 1
    if(prd.is_hit)
    {
        atomicAdd(deref(p.camera_buffer).hit_count, 1);
    }
#endif

    // imageStore(daxa_image2D(p.swapchain), index, fromLinear(vec4(prd.hitValue, 1.0)));
    imageStore(daxa_image2D(p.swapchain), index, vec4(prd.hitValue, 1.0));
    // imageStore(daxa_image2D(p.swapchain), index, vec4(1.0, 1.0, 1.0, 1.0));
}

#endif // PRIMARY_RAYS

#elif DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_INTERSECTION

// Ray-AABB intersection
float hitAabb(const Aabb aabb, const Ray r)
{
    vec3 invDir = 1.0 / r.direction;
    vec3 tbot = invDir * (aabb.minimum - r.origin);
    vec3 ttop = invDir * (aabb.maximum - r.origin);
    vec3 tmin = min(ttop, tbot);
    vec3 tmax = max(ttop, tbot);
    float t0 = max(tmin.x, max(tmin.y, tmin.z));
    float t1 = min(tmax.x, min(tmax.y, tmax.z));
    return t1 > max(t0, 0.0) ? t0 : -1.0;
}

void main()
{
    Ray ray;
    ray.origin = gl_ObjectRayOriginEXT;
    ray.direction = gl_ObjectRayDirectionEXT;

    // mat4 model = mat4(
    //   gl_WorldToObjectEXT[0][0], gl_WorldToObjectEXT[0][1], gl_WorldToObjectEXT[0][2], 0,
    //   gl_WorldToObjectEXT[1][0], gl_WorldToObjectEXT[1][1], gl_WorldToObjectEXT[1][2], 0,
    //   gl_WorldToObjectEXT[2][0], gl_WorldToObjectEXT[2][1], gl_WorldToObjectEXT[2][2], 0,
    //   gl_WorldToObjectEXT[3][0], gl_WorldToObjectEXT[3][1], gl_WorldToObjectEXT[3][2], 1.0);

    mat4 inv_model = mat4(
        gl_ObjectToWorld3x4EXT[0][0], gl_ObjectToWorld3x4EXT[0][1], gl_ObjectToWorld3x4EXT[0][2], gl_ObjectToWorld3x4EXT[0][3],
        gl_ObjectToWorld3x4EXT[0][1], gl_ObjectToWorld3x4EXT[1][1], gl_ObjectToWorld3x4EXT[1][2], gl_ObjectToWorld3x4EXT[1][3],
        gl_ObjectToWorld3x4EXT[2][0], gl_ObjectToWorld3x4EXT[2][1], gl_ObjectToWorld3x4EXT[2][2], gl_ObjectToWorld3x4EXT[2][3],
        0, 0, 0, 1.0);

    ray.origin = (inv_model * vec4(ray.origin, 1)).xyz;
    ray.direction = (inv_model * vec4(ray.direction, 0)).xyz;

    float tHit = -1;

    uint i = gl_PrimitiveID + gl_GeometryIndexEXT + gl_InstanceCustomIndexEXT;

    Aabb aabb = deref(p.aabb_buffer).aabbs[i];
    // aabb.minimum = (model * vec4(aabb.minimum, 1)).xyz;
    // aabb.maximum = (model * vec4(aabb.maximum, 1)).xyz;

    tHit = hitAabb(aabb, ray);

    // Report hit point
    if (tHit > 0)
        reportIntersectionEXT(tHit, 0); // 0 is the hit kind (hit group index)
}

#elif DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_ANY_HIT

layout(location = 0) rayPayloadInEXT hitPayload prd;

// hardcoded dissolve
const float dissolve = 0.3;

void main()
{

    uint seed = prd.seed; // We don't want to modify the PRD
    if (rnd(seed) > dissolve)
        ignoreIntersectionEXT;
}

#elif DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_CALLABLE

layout(location = 3) callableDataInEXT rayLight cLight;

#if defined(SPOT_LIGHT)

// Hardcoded light position
vec3 lightPos = vec3(1.0, 2.0, 0.4);

// Hardcoded light intensity
float lightIntensity = 10.0;

void main()
{
    vec3 lDir = lightPos - cLight.inHitPosition;
    cLight.outLightDistance = length(lDir);
    cLight.outIntensity = lightIntensity / (cLight.outLightDistance * cLight.outLightDistance);
    cLight.outLightDir = normalize(lDir);
}

#else

// Harcoded light direction
vec3 lightDirection = vec3(0.0, 1.0, 0.0);

void main()
{
    cLight.outLightDistance = 1000000;
    cLight.outIntensity = 3.0;
    cLight.outLightDir = normalize(lightDirection);
}

#endif // SPOT_LIGHT

#elif DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_CLOSEST_HIT

#if defined(HIT_TRIANGLE)

#extension GL_EXT_ray_tracing_position_fetch : require

hitAttributeEXT vec2 attribs;

layout(location = 0) rayPayloadInEXT hitPayload prd;

void main()
{

    // Barycentric coordinates from GL_EXT_ray_tracing extension
    const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

    // Vertex positions in object space from GL_EXT_ray_tracing_position_fetch extension
    vec3 v0 = gl_HitTriangleVertexPositionsEXT[0];
    vec3 v1 = gl_HitTriangleVertexPositionsEXT[1];
    vec3 v2 = gl_HitTriangleVertexPositionsEXT[2];

    // Color vertices (adjust these colors as desired)
    vec3 color0 = vec3(1.0, 0.0, 0.0); // Red
    vec3 color1 = vec3(0.0, 1.0, 0.0); // Green
    vec3 color2 = vec3(0.0, 0.0, 1.0); // Blue

    // Interpolate colors using barycentric coordinates
    vec3 interpolatedColor = barycentrics.x * color0 + barycentrics.y * color1 + barycentrics.z * color2;

    // Output the interpolated color
    prd.hitValue = interpolatedColor;
#if ATOMIC_FLOAT == 1
    prd.is_hit = true;
#endif // HIT_TRIANGLE
}

#else // HIT_TRIANGLE

layout(location = 0) rayPayloadInEXT hitPayload prd;
layout(location = 1) rayPayloadEXT bool isShadowed;
layout(location = 3) callableDataEXT rayLight cLight;

// Change this to 0 to see the difference between callables
#define DAXA_CALLABLE_INDEX 0
// #define DEBUG_NORMALS

void main()
{
    vec3 world_pos = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;

    cLight.inHitPosition = world_pos;

    if (DAXA_CALLABLE_INDEX < 2)
        executeCallableEXT(DAXA_CALLABLE_INDEX, 3);
    else
    {
        vec3 lDir = vec3(1.0, 2.0, 0.4) - cLight.inHitPosition;
        cLight.outLightDistance = length(lDir);
        cLight.outIntensity = 100.0 / (cLight.outLightDistance * cLight.outLightDistance);
        cLight.outLightDir = normalize(lDir);
    }

    mat4 model = mat4(
        gl_ObjectToWorldEXT[0][0], gl_ObjectToWorldEXT[0][1], gl_ObjectToWorldEXT[0][2], 0,
        gl_ObjectToWorldEXT[0][1], gl_ObjectToWorldEXT[1][1], gl_ObjectToWorldEXT[1][2], 0,
        gl_ObjectToWorldEXT[2][0], gl_ObjectToWorldEXT[2][1], gl_ObjectToWorldEXT[2][2], 0,
        gl_ObjectToWorldEXT[3][0], gl_ObjectToWorldEXT[3][1], gl_ObjectToWorldEXT[3][2], 1.0);

    uint prim_index = gl_PrimitiveID + gl_GeometryIndexEXT + gl_InstanceCustomIndexEXT;

    Aabb aabb = deref(p.aabb_buffer).aabbs[prim_index];
    vec3 center = (aabb.minimum + aabb.maximum) * 0.5;
    center = (model * vec4(center, 1)).xyz;

    // Computing the normal at hit position
    vec3 world_nrm = normalize(world_pos - center);

    {
        vec3 absN = abs(world_nrm);
        float maxC = max(max(absN.x, absN.y), absN.z);
        world_nrm = (maxC == absN.x) ? vec3(sign(world_nrm.x), 0, 0) : (maxC == absN.y) ? vec3(0, sign(world_nrm.y), 0)
                                                                                        : vec3(0, 0, sign(world_nrm.z));
    }

    const vec3 diffuse = world_nrm * 0.5 + 0.5;

#ifdef DEBUG_NORMALS
    prd.hitValue = diffuse;
#else
    float attenuation = 0.3;

    // Tracing shadow ray only if the light is visible from the surface
    if (dot(world_nrm, cLight.outLightDir) > 0)
    {
        float tMin = 0.001;
        float tMax = cLight.outLightDistance;
        vec3 origin = world_pos;
        vec3 rayDir = cLight.outLightDir;
        uint flags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;
        isShadowed = true;
        traceRayEXT(
            daxa_accelerationStructureEXT(p.tlas),
            flags,  // rayFlags
            0xFF,   // cullMask
            0,      // sbtRecordOffset
            0,      // sbtRecordStride
            1,      // missIndex
            origin, // ray origin
            tMin,   // ray min range
            rayDir, // ray direction
            tMax,   // ray max range
            1       // payload (location = 1)
        );

        if (isShadowed)
        {
            attenuation = 0.3;
        }
        else
        {
            attenuation = 1.0;
        }
    }

    prd.hitValue = vec3(cLight.outIntensity * attenuation * (diffuse));
#if ATOMIC_FLOAT == 1
    prd.is_hit = true;
#endif // ATOMIC_FLOAT
#endif // DEBUG_NORMALS
}

#endif // HIT_TRIANGLE

#elif DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_MISS

#if defined(MISS_SHADOW)

layout(location = 1) rayPayloadInEXT bool isShadowed;

void main()
{
    isShadowed = false;
}

#else

layout(location = 0) rayPayloadInEXT hitPayload prd;

void main()
{
    prd.hitValue = vec3(0.5, 0.7, 1.0);
}

#endif // MISS_SHADOW

#endif // DAXA_SHADER_STAGE