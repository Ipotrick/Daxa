
layout(push_constant) uniform Push {
    vec4 pos;
    uint globals_sb;
    uint output_image_i;
    uint chunk_buffer_i;
}
p;

layout(set = 0, binding = 3, r32ui) uniform readonly uimage3D input_images[];
layout(set = 0, binding = 3, r32ui) uniform writeonly uimage3D output_images[];

#include <common.glsl>
