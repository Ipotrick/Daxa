#pragma once

bool draw_ui(
    in StructuredBuffer<Globals> globals,
    in float2 uv, in float aspect, in out float3 color) {
    uv.y -= 0.8;
    uv.y *= 1.0 / aspect;
    uv *= 2;

    if (uv.y > -0.2 && uv.y < 0 && uv.x > -0.8 && uv.x < 0.8) {
        uv *= 5;
        uint inventory_i = uv.x + 4;
        uv = frac(uv);

        uv = uv * 2 - 1;
        uv.y *= -1;
        Ray ray;
        ray.o = float3(2, -0.1, 2) + uv.x * normalize(float3(1, 0, -1)) - uv.y * normalize(float3(-1, 10, -1));
        ray.nrm = normalize(float3(-1, 0.4, -1.2));
        ray.inv_nrm = 1 / ray.nrm;

        RayIntersection inter = ray_box_intersect(ray, float3(0, 0, 0), float3(1, 1, 1));
        float3 inter_pos = get_intersection_pos_corrected(ray, inter);

        BlockFace face_id;
        float2 tex_uv;
        uint tex_id;

        BlockID block_id = inventory_palette(inventory_i);

        get_texture_info(globals, inter, inter_pos, block_id, tex_uv, face_id, tex_id);

        float3 tex_col = daxa::getTexture2DArray<float4>(globals[0].texture_index).Load(int4(tex_uv.x * 16, tex_uv.y * 16, tex_id, 0)).rgb;

        if (inter.hit) {
            color = tex_col;
            return true;
        }

        float outline_scl = 0.05;
        RayIntersection outline_inter = ray_box_intersect(ray, float3(-outline_scl, -outline_scl, -outline_scl), float3(1 + outline_scl, 1 + outline_scl, 1 + outline_scl));
        if (outline_inter.hit) {
            if (globals[0].inventory_index == inventory_i) {
                color = float3(1.0, 1.0, 1.0);
            } else {
                color = float3(0.0, 0.0, 0.0);
            }
            return true;
        }
    }
    return false;
}
