#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 4) buffer Globals {
    mat4 viewproj_mat;
    vec4 pos;
    ivec2 frame_dim;
    float time;

    uint chunk_image_0i, chunk_image_1i;
}
globals_sb_view[];

#define globals globals_sb_view[p.globals_sb]
#define chunk_image_0 input_images[globals.chunk_image_0i]
#define chunk_image_1 input_images[globals.chunk_image_1i]
#define output_image output_images[p.output_image_i]
