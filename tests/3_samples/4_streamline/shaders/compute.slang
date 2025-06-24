#include "daxa/daxa.inl"
#include "shared.inl"

[[vk::push_constant]] ComputePush p;

struct Sphere
{
    float3 center;
    float radius;
    int is_light;
};

struct HitInfo
{
    float3 position;
    float3 normal;
    float3 diffuse_albedo;
    float3 specular_albedo;
    float roughness;
    float distance;
    int material_id;
    bool is_emissive;
};

#define MAX_SPHERES 3
#define MAX_BOUNCES 3
#define PI 3.14159265358979323846f
#define FLT_MIN 1.17549435e-38

static const Sphere spheres[MAX_SPHERES] = {
    {{5, 5, 0}, 1.0, 1},         // light
    {{0, 1, 0}, 1.0, 0},         // object
    {{0, -1000.5, 0}, 1000.0, 0} // "inifite" ground
};

// internal RNG state
static uint seed;
static int2 pixel;

void InitRNG(float2 p, int frame, uint2 resolution)
{
    pixel = int2(p);
    seed = uint((frame * resolution.y + pixel.y) * resolution.x + pixel.x);
}

float rand()
{
    // Condensed version of pcg_output_rxs_m_xs_32_32, with simple conversion to floating-point [0,1].
    seed = seed * 747796405 + 1;
    uint word = ((seed >> ((seed >> 28) + 4)) ^ seed) * 277803737;
    word = (word >> 22) ^ word;
    return float(word) / 4294967295.0f;
}

bool intersect_sphere(float3 ro, float3 rd, Sphere sph, out float t_hit)
{
    float3 oc = ro - sph.center;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - sph.radius * sph.radius;
    float d = b * b - c;
    if (d < 0)
    {
        t_hit = -1;
        return false;
    }
    float sq = sqrt(d);
    float t0 = -b - sq;
    float t1 = -b + sq;
    t_hit = (t0 > 0) ? t0 : ((t1 > 0) ? t1 : -1);
    return t_hit > 0;
}

// cosine-weighted sampling
float3 sample_hemisphere(float3 N)
{
    float u = rand(), v = rand();
    float r = sqrt(u);
    float phi = 2 * 3.14159265 * v;
    float x = r * cos(phi);
    float y = r * sin(phi);
    float z = sqrt(max(0, 1 - u));
    // base ortonormal
    float3 up = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 tangent = normalize(cross(up, N));
    float3 bitan = cross(N, tangent);
    return x * tangent + y * bitan + z * N;
}

float3 sample_sky(float3 dir)
{
    // Convert dir to UV [0,1]
    float theta = atan2(dir.x, dir.z);           // longitude
    float phi = asin(saturate(dir.y));           // latitude
    float2 uv = float2(theta * (0.5 / PI) + 0.5, // mapθ∈[-π,π]→[0,1]
                       phi * (1.0 / PI) + 0.5);  // mapφ∈[-π/2,π/2]→[0,1]
    return p.env_map.get().SampleLevel(p.env_sampler.get(), uv, 0).rgb;
}

float2 reproj_uv(float3 world_pos,
    float3 cam_pos,
    float3 cam_right,
    float3 cam_up,
    float3 cam_fwd,
    float  fov,
    float  aspect)
{
    // world→camera
    float3 co = world_pos - cam_pos;
    float3 cam_space;
    cam_space.x = dot(co, cam_right);
    cam_space.y = dot(co, cam_up);
    cam_space.z = dot(co, cam_fwd);

    // clip-space projection (GLM/Vulkan LH-ZO mat[0][0]=1/(aspect*tan), mat[1][1]=1/tan)
    float inv_tan = 1.0 / tan(fov * 0.5);
    float clip_x  = cam_space.x * (inv_tan / aspect);
    float clip_y  = cam_space.y * inv_tan;
    float clip_z  = cam_space.z;

    // NDC
    float2 ndc = float2(clip_x, clip_y) / clip_z;   // [-1,+1]

    // UV
    return ndc * 0.5 + 0.5;                      // [0,1]
}

float2 calculate_motion_vector(float3 world_pos, GpuInput current_info, GpuInput prev_info)
{
    float2 uvCurr = reproj_uv(world_pos,
        current_info.camera_pos,
        current_info.camera_right,
        current_info.camera_up,
        current_info.camera_forward,
        current_info.fov,
        current_info.aspect);

    float2 uvPrev = reproj_uv(world_pos,
            prev_info.camera_pos,
            prev_info.camera_right,
            prev_info.camera_up,
            prev_info.camera_forward,
            prev_info.fov,
            prev_info.aspect);

    return (uvPrev - uvCurr) * current_info.frame_dim;
}

// credits: https://github.com/NVIDIA-RTX/Streamline/blob/main/docs/ProgrammingGuideDLSS_RR.md
float3 env_brdf_approx2(float3 specular_color, float alpha, float n_dot_v) 
{ 
    n_dot_v = abs(n_dot_v); 
    // [Ray Tracing Gems, Chapter 32]
    float4 X; 
    X.x = 1.f; 
    X.y = n_dot_v; 
    X.z = n_dot_v * n_dot_v; 
    X.w = n_dot_v * X.z; 
    float4 Y; 
    Y.x = 1.f; 
    Y.y = alpha; 
    Y.z = alpha * alpha; 
    Y.w = alpha * Y.z; 
    float2x2 M1 = float2x2(0.99044f, -1.28514f, 1.29678f, -0.755907f); 
    float3x3 M2 = float3x3(1.f, 2.92338f, 59.4188f, 20.3225f, -27.0302f, 222.592f, 121.563f, 626.13f, 316.627f); 
    float2x2 M3 = float2x2(0.0365463f, 3.32707, 9.0632f, -9.04756); 
    float3x3 M4 = float3x3(1.f, 3.59685f, -1.36772f, 9.04401f, -16.3174f, 9.22949f, 5.56589f, 19.7886f, -20.2123f); 
    float bias = dot(mul(M1, X.xy), Y.xy) * rcp(dot(mul(M2, X.xyw), Y.xyw)); 
    float scale = dot(mul(M3, X.xy), Y.xy) * rcp(dot(mul(M4, X.xzw), Y.xyw)); 
    // This is a hack for specular reflectance of 0
    bias *= saturate(specular_color.g * 50); 
    return mad(specular_color, max(0, scale), max(0, bias)); 
} 

void get_material_properties(int hit_id, float n_dot_v, out float3 diffuse_albedo, out float3 specular_albedo, out float roughness)
{
    if (hit_id == 0) // Light
    {
        diffuse_albedo = float3(1, 1, 1);
        roughness = 1.0;
        specular_albedo = env_brdf_approx2(float3(0, 0, 0), roughness, n_dot_v);
    }
    else if (hit_id == 1) // Red sphere object
    {
        diffuse_albedo = float3(1, 0.3, 0.1);
        roughness = 1.0;
        specular_albedo = env_brdf_approx2(float3(0.04, 0.04, 0.04), roughness, n_dot_v);
    }
    else // Green ground
    {
        diffuse_albedo = float3(0.5, 0.8, 0.2);
        roughness = 1.0;
        specular_albedo = env_brdf_approx2(float3(0.04, 0.04, 0.04), roughness, n_dot_v);
    }
}

float3 path_trace(float2 pix, out HitInfo first_hit, out float2 motion_vector, out float2 spec_motion_vector)
{
    GpuInput info = *p.ptr;
    // ray origin dir configuration
    float2 uv = (pix / info.frame_dim) * 2 - 1;
    uv.y = -uv.y;
    uv.x *= info.aspect;
    float2 q = uv * tan(info.fov * 0.5);
    float3 ro = info.camera_pos;
    float3 rd = normalize(q.x * info.camera_right + q.y * info.camera_up + info.camera_forward);

    // RNG seed
    InitRNG(pix, p.frame_index, info.frame_dim);

    float3 throughput = float3(1, 1, 1);
    float3 radiance = float3(0, 0, 0);
    float t_hit;
    bool hit_something = false;

    // Initialize default values
    first_hit.position = float3(0, 0, 0);
    first_hit.normal = float3(0, 0, 1);
    first_hit.diffuse_albedo = float3(0, 0, 0);
    first_hit.specular_albedo = float3(0, 0, 0);
    first_hit.roughness = 1.0;
    first_hit.distance = info.far_plane;
    first_hit.material_id = -1;
    first_hit.is_emissive = false;

    for (int bounce = 0; bounce < MAX_BOUNCES; ++bounce)
    {
        // check intersection
        float t_min = info.far_plane;
        int hit_id = -1;
        [unroll] for (int i = 0; i < MAX_SPHERES; i++) if (intersect_sphere(ro, rd, spheres[i], t_hit) && t_hit < t_min)
        {
            t_min = t_hit;
            hit_id = i;
        }

        if (hit_id < 0)
        {
            // sin hit -> background
            float3 env = sample_sky(rd);
            radiance += throughput * env;

            // If it is the first bounce, we save sky info
            if (bounce == 0)
            {
                first_hit.diffuse_albedo = env;
                first_hit.normal = -rd;
                first_hit.is_emissive = true;
            }
            break;
        }
        
        Sphere sph = spheres[hit_id];
        float3 hit_pos = ro + rd * t_min;
        float3 n = normalize(hit_pos - sph.center);

        float n_dot_v = max(0.0, dot(n, -rd));

        // Save information of the first hit
        if (bounce == 0) 
        {
            first_hit.position = hit_pos;
            first_hit.normal = n;
            first_hit.distance = t_min;
            first_hit.material_id = hit_id;
            hit_something = true;

            // Get material properties
            get_material_properties(hit_id, n_dot_v, first_hit.diffuse_albedo, 
                                   first_hit.specular_albedo, first_hit.roughness);
            
            first_hit.is_emissive = (sph.is_light == 1);
        }


        // accumulate object emission
        if (sph.is_light == 1)
        {
            radiance += throughput * float3(10, 10, 8);
            break;
        }

        float3 base_color, spec_color;
        float roughness;
        get_material_properties(hit_id, n_dot_v, base_color, spec_color, roughness);


        float3 new_dir = sample_hemisphere(n);

        // cosine att * albedo
        float cos_nl = max(dot(new_dir, n), 0);
        throughput *= base_color * cos_nl;

        // next bounce
        ro = hit_pos + n * 1e-3;
        rd = new_dir;
    }
    
    // Calculate motion vectors
    if (hit_something)
    {
        motion_vector = calculate_motion_vector(first_hit.position, info, *p.prev_ptr);
        // For purely diffuse materials, we use the same motion vector
        spec_motion_vector = motion_vector;
        first_hit.distance = distance(ro, first_hit.position) / info.far_plane;
    }
    else
    {
        motion_vector = float2(FLT_MIN, FLT_MIN);
        spec_motion_vector = float2(FLT_MIN, FLT_MIN);
        first_hit.distance = 1.0;  // Far plane
    }

    return radiance;
}

[shader("compute")]
    [numthreads(8, 8, 1)] void
    entry_streamline(uint3 pix : SV_DispatchThreadID)
{
    if (pix.x >= p.ptr.frame_dim.x || pix.y >= p.ptr.frame_dim.y)
        return;
    float2 coord = pix.xy + float2(0.5);
    HitInfo first_hit;
    float2 motion_vector;
    float2 spec_motion_vector;
    float3 col = path_trace(coord, first_hit, motion_vector, spec_motion_vector);

    if (p.render_mode == 1) {
        // Visualize motion vectors
        float2 mv_display = motion_vector;
        if (motion_vector.x == FLT_MIN && motion_vector.y == FLT_MIN) {
            p.image_id.get()[pix.xy] = float4(0, 0, 0, 1); // Background black
        } else {
            // mv_display = mv_display * 10.0; // Amplify x10
            // mv_display = saturate(mv_display);
            
            p.image_id.get()[pix.xy] = float4(
                mv_display.x,  // Red = mov X
                mv_display.y,  // Green = mov Y
                0, 
                1
            );
        }
    } else if (p.render_mode == 2) {
        // Visualize depth
        p.image_id.get()[pix.xy] = float4(first_hit.distance, first_hit.distance, first_hit.distance, 1);
    } else {
        p.image_id.get()[pix.xy] = float4(col, 1);
    }
    p.diffuse_albedo_id.get()[pix.xy] = first_hit.diffuse_albedo;
    p.specular_albedo_id.get()[pix.xy] = first_hit.specular_albedo;
    
    // Pack normal (xyz) and roughness (w) into a float4
    p.normal_roughness_id.get()[pix.xy] = float4(first_hit.normal, first_hit.roughness);
    
    p.mvec_id.get()[pix.xy] = motion_vector;
    p.spec_mvec_id.get()[pix.xy] = spec_motion_vector;
    p.depth_id.get()[pix.xy] = first_hit.distance;
}