#include <shared.inl>
// #include <custom file!!>


#if ZEROED_GRID_COMPUTE_FLAG == 1
layout(local_size_x = MPM_GRID_COMPUTE_X, local_size_y = MPM_GRID_COMPUTE_Y, local_size_z = MPM_GRID_COMPUTE_Z) in;
void main()
{
  uvec3 pixel_i = gl_GlobalInvocationID.xyz;

  daxa_BufferPtr(GpuInput) config = daxa_BufferPtr(GpuInput)(daxa_id_to_address(p.input_buffer_id));

  if (pixel_i.x >= deref(config).grid_dim.x || pixel_i.y >= deref(config).grid_dim.y || pixel_i.z >= deref(config).grid_dim.z)
  {
      return;
  }

  uint cell_index = pixel_i.x + pixel_i.y * deref(config).grid_dim.x + pixel_i.z * deref(config).grid_dim.x * deref(config).grid_dim.y;

  zeroed_out_cell_by_index(cell_index);
}
#elif P2G_COMPUTE_FLAG == 1
// Main compute shader
layout(local_size_x = MPM_P2G_COMPUTE_X, local_size_y = 1, local_size_z = 1) in;
void main()
{
  uint pixel_i_x = gl_GlobalInvocationID.x;

  daxa_BufferPtr(GpuInput) config = daxa_BufferPtr(GpuInput)(daxa_id_to_address(p.input_buffer_id));

  if (pixel_i_x >= deref(config).p_count)
  {
      return;
  }

  float dx = deref(config).dx;
  float inv_dx = deref(config).inv_dx;
  float dt = deref(config).dt;
  float p_rho = 1;
  float p_vol = (dx * 0.5f) * (dx * 0.5f) * (dx * 0.5f); // Particle volume (cube)
  float p_mass = p_vol * p_rho;
  float E = 1000;
  float nu = 0.2f; //  Poisson's ratio
  float mu_0 = E / (2 * (1 + nu));
  float lambda_0 = E * nu / ((1 + nu) * (1 - 2 * nu)); // Lame parameters

  Particle particle = get_particle_by_index(pixel_i_x);

  Aabb aabb = get_aabb_by_index(pixel_i_x);

  daxa_f32vec3 w[3];
  daxa_f32vec3 fx;
  daxa_i32vec3 base_coord = calculate_particle_status(aabb, inv_dx, fx, w);

  mat3 stress = calculate_p2g(particle, dt, p_vol, mu_0, lambda_0, inv_dx);

  mat3 affine = stress + p_mass * particle.C;

  // Transactional momentum
  vec3 mv = vec3(p_mass * particle.v);

  uvec3 array_grid = uvec3(base_coord);

  // Scatter to grid
  for (uint i = 0; i < 3; ++i)
  {
      for (uint j = 0; j < 3; ++j)
      {
          for (uint k = 0; k < 3; ++k)
          {
              uvec3 coord = array_grid + uvec3(i, j, k);
              if (coord.x >= deref(config).grid_dim.x || coord.y >= deref(config).grid_dim.y || coord.z >= deref(config).grid_dim.z)
              {
                  continue;
              }

              vec3 dpos = (vec3(i, j, k) - fx) * dx;

              float weight = w[i].x * w[j].y * w[k].z;

              vec3 velocity_mass = weight * (mv + affine * dpos);
              float m = weight * p_mass;

              uint index = (coord.x + coord.y * deref(config).grid_dim.x + coord.z * deref(config).grid_dim.x * deref(config).grid_dim.y);

              set_atomic_cell_x_by_index(index, velocity_mass.x);
              set_atomic_cell_y_by_index(index, velocity_mass.y);
              set_atomic_cell_z_by_index(index, velocity_mass.z);
              set_atomic_cell_w_by_index(index, m);
          }
      }
  }

  // TODO: optimize this write
  set_particle_by_index(pixel_i_x, particle);
}

#elif GRID_COMPUTE_FLAG == 1
layout(local_size_x = MPM_GRID_COMPUTE_X, local_size_y = MPM_GRID_COMPUTE_Y, local_size_z = MPM_GRID_COMPUTE_Z) in;
void main()
{
  uvec3 pixel_i = gl_GlobalInvocationID.xyz;

  daxa_BufferPtr(GpuInput) config = daxa_BufferPtr(GpuInput)(daxa_id_to_address(p.input_buffer_id));

  if (pixel_i.x >= deref(config).grid_dim.x || pixel_i.y >= deref(config).grid_dim.y || pixel_i.z >= deref(config).grid_dim.z)
  {
      return;
  }

  uint cell_index = pixel_i.x + pixel_i.y * deref(config).grid_dim.x + pixel_i.z * deref(config).grid_dim.x * deref(config).grid_dim.y;

  float dt = deref(config).dt;
  float gravity = deref(config).gravity;
  uint bound = 3;

  Cell cell = get_cell_by_index(cell_index);

  if(cell.info.w != 0) {
    cell.info.xyz /= cell.info.w; // Normalize by mass
    // if cell velocity less than 0 and pixel_i.xyz < bound, set to 0
    bool bound_x =
        (pixel_i.x < bound) && (cell.info.x < 0) || (pixel_i.x > deref(config).grid_dim.x - bound) && (cell.info.x > 0);
    bool bound_y = 
        (pixel_i.y < bound) && (cell.info.y < 0) || (pixel_i.y > deref(config).grid_dim.y - bound) && (cell.info.y > 0);
    bool bound_z = 
        (pixel_i.z < bound) && (cell.info.z < 0) || (pixel_i.z > deref(config).grid_dim.z - bound) && (cell.info.z > 0);
    cell.info += dt * vec4(0, gravity, 0, 0); // Gravity
    if(bound_x) {
      cell.info.x = 0;
    }
    if(bound_y) {
      cell.info.y = 0;
    }
    if(bound_z) {
      cell.info.z = 0;
    }

    set_cell_by_index(cell_index, cell);
  }
}
#elif G2P_COMPUTE_FLAG == 1
// Main compute shader
layout(local_size_x = MPM_P2G_COMPUTE_X, local_size_y = 1, local_size_z = 1) in;
void main()
{
  uint pixel_i_x = gl_GlobalInvocationID.x;

  daxa_BufferPtr(GpuInput) config = daxa_BufferPtr(GpuInput)(daxa_id_to_address(p.input_buffer_id));

  if (pixel_i_x >= deref(config).p_count)
  {
      return;
  }

  float dx = deref(config).dx;
  float inv_dx = deref(config).inv_dx;
  float dt = deref(config).dt;

  Particle particle = get_particle_by_index(pixel_i_x);
  Aabb aabb = get_aabb_by_index(pixel_i_x);
  
  daxa_f32vec3 w[3];
  daxa_f32vec3 fx;
  daxa_i32vec3 base_coord = calculate_particle_status(aabb, inv_dx, fx, w);

  particle.C = mat3(0);
  particle.v = vec3(0.f);
  
  uvec3 array_grid = uvec3(base_coord);

  for(uint i = 0; i < 3; ++i) {
    for(uint j = 0; j < 3; ++j) {
      for(uint k = 0; k < 3; ++k) {
        uvec3 coord = array_grid + uvec3(i, j, k);
        
        if(coord.x >= deref(config).grid_dim.x || coord.y >= deref(config).grid_dim.y || coord.z >= deref(config).grid_dim.z) {
          continue;
        }

        vec3 dpos = (vec3(i, j, k) - fx) * dx;

        float weight = w[i].x * w[j].y * w[k].z;

        uint index = coord.x + coord.y * deref(config).grid_dim.x + coord.z * deref(config).grid_dim.x * deref(config).grid_dim.y;

        vec4 grid_value = get_cell_by_index(index).info;

        vec3 w_grid = vec3(grid_value.xyz * weight);

        particle.v += w_grid; // Velocity
        particle.C += 4 * inv_dx * inv_dx * weight * outer_product(grid_value.xyz, dpos);
      }
    }
  }

  aabb.min += dt * particle.v;
  aabb.max += dt * particle.v;

  set_aabb_by_index(pixel_i_x, aabb);

  // TODO: optimize this write
  set_particle_by_index(pixel_i_x, particle);
}
#elif SPHERE_TRACING_FLAG == 1

// Main compute shader
layout(local_size_x = MPM_SHADING_COMPUTE_X, local_size_y = MPM_SHADING_COMPUTE_Y, local_size_z = 1) in;
void main()
{
    uvec2 pixel_i = gl_GlobalInvocationID.xy;

    uvec2 frame_dim = uvec2(deref(p.camera).frame_dim);

    if (pixel_i.x >= deref(p.camera).frame_dim.x || pixel_i.y >= deref(p.camera).frame_dim.y)
        return;
    // Camera setup
    daxa_f32mat4x4 inv_view = deref(p.camera).inv_view;
    daxa_f32mat4x4 inv_proj = deref(p.camera).inv_proj;

    Ray ray =
        get_ray_from_current_pixel(daxa_f32vec2(pixel_i.xy), daxa_f32vec2(frame_dim), inv_view, inv_proj);

    vec3 col = vec3(0, 0, 0); // Background color
    daxa_f32 t = 0.0f;
    daxa_f32 dist = 0.0f;
    
    bool hit = false;
    daxa_BufferPtr(GpuInput) config = daxa_BufferPtr(GpuInput)(daxa_id_to_address(p.input_buffer_id));

    while (t < MAX_DIST) {
        vec3 pos = ray.origin + t * ray.direction;
        daxa_f32 min_dist = MAX_DIST;
        for (uint i = 0; i < deref(config).p_count; ++i) {
            Particle particle = get_particle_by_index(i);
            Aabb aabb = get_aabb_by_index(i);
            daxa_f32vec3 p_center = (aabb.min + aabb.max) * 0.5f;
            daxa_f32 hit_dist = compute_sphere_distance(pos, p_center, PARTICLE_RADIUS);
            if (hit_dist < min_dist) {
                min_dist = hit_dist;
            }
        }

        if (min_dist < MIN_DIST) {
            hit = true;
            break;
        }

        t += min_dist;
        if (t >= MAX_DIST) {
            break;
        }
    }

    if (hit) {
        col = vec3(0.3, 0.8, 1); // Sphere color
    }

    imageStore(daxa_image2D(p.image_id), ivec2(pixel_i.xy), vec4(col, 1.0));
}

#else 
// Main compute shader
layout(local_size_x = 4, local_size_y = 4, local_size_z = 4) in;
void main()
{
}
#endif