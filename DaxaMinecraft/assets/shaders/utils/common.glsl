#extension GL_EXT_nonuniform_qualifier : enable

const uvec3 CHUNK_N = uvec3(8, 1, 8);

layout(set = 0, binding = 4) buffer Globals {
    mat4 viewproj_mat;
    vec4 pos;
    ivec2 frame_dim;
    float time;

    uint chunk_images[CHUNK_N.z][CHUNK_N.y][CHUNK_N.x];
}
globals_sb_view[];

#define globals globals_sb_view[p.globals_sb]
#define chunk_images(_ci) input_images[globals.chunk_images[_ci.z][_ci.y][_ci.x]]
#define output_image output_images[p.output_image_i]
