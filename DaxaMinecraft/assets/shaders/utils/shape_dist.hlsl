#pragma once

float sd_sphere(float3 p, float3 o, float s) {
    return length(p - o) - s;
}

float sd_capsule(float3 p, float3 a, float3 b, float r) {
    float3 pa = p - a, ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h) - r;
}

float sd_box(float3 p, float3 o, float3 s) {
    float3 d = abs(p - o) - s;
    return min(max(d.x, max(d.y, d.z)), 0.0) + length(max(d, 0.0));
}

float sd_box_frame(float3 p, float3 o, float3 s, float e) {
    p = abs(p - o) - s;
    float3 q = abs(p + float3(e, e, e)) - float3(e, e, e);
    return min(
        min(
            length(max(float3(p.x, q.y, q.z), 0.0)) + min(max(p.x, max(q.y, q.z)), 0.0),
            length(max(float3(q.x, p.y, q.z), 0.0)) + min(max(q.x, max(p.y, q.z)), 0.0)),
        length(max(float3(q.x, q.y, p.z), 0.0)) + min(max(q.x, max(q.y, p.z)), 0.0));
}

float sd_ellipsoid(float3 p, float3 o, float3 r) {
    float k0 = length((p - o) / r);
    float k1 = length((p - o) / (r * r));
    return k0 * (k0 - 1.0) / k1;
}

float sd_torus(float3 p, float3 o, float2 t) {
    return length(float2(length(p.xz - o.xz) - t.x, p.y - o.y)) - t.y;
}

float sd_tri_prism(float3 p, float3 o, float2 h) {
    p -= o;
    const float k = sqrt(3.0);
    h.x *= 0.5 * k;
    p.xy /= h.x;
    p.x = abs(p.x) - 1.0;
    p.y = p.y + 1.0 / k;
    if (p.x + k * p.y > 0.0)
        p.xy = float2(p.x - k * p.y, -k * p.x - p.y) / 2.0;
    p.x -= clamp(p.x, -2.0, 0.0);
    float d1 = length(p.xy) * sign(-p.y) * h.x;
    float d2 = abs(p.z) - h.y;
    return length(max(float2(d1, d2), 0.0)) + min(max(d1, d2), 0.);
}

float sd_cone(float3 p, float3 a, float3 b, float ra, float rb) {
    float rba = rb - ra;
    float baba = dot(b - a, b - a), papa = dot(p - a, p - a), paba = dot(p - a, b - a) / baba;
    float x = sqrt(papa - paba * paba * baba);
    float cax = max(0.0, x - ((paba < 0.5) ? ra : rb)), cay = abs(paba - 0.5) - 0.5;
    float k = rba * rba + baba, f = clamp((rba * (x - ra) + paba * baba) / k, 0.0, 1.0);
    float cbx = x - ra - f * rba, cby = paba - f;
    float s = (cbx < 0.0 && cay < 0.0) ? -1.0 : 1.0;
    return s * sqrt(min(cax * cax + cay * cay * baba, cbx * cbx + cby * cby * baba));
}

float sd_cylinder(float3 p, float3 a, float3 b, float r) {
    float3 pa = p - a;
    float3 ba = b - a;
    float baba = dot(ba, ba);
    float paba = dot(pa, ba);
    float x = length(pa * baba - ba * paba) - r * baba;
    float y = abs(paba - baba * 0.5) - baba * 0.5;
    float x2 = x * x;
    float y2 = y * y * baba;
    float d = (max(x, y) < 0.0) ? -min(x2, y2) : (((x > 0.0) ? x2 : 0.0) + ((y > 0.0) ? y2 : 0.0));
    return sign(d) * sqrt(abs(d)) / baba;
}

struct TreeSDF {
    float trunk_dist;
    float leaves_dist;
};

TreeSDF sd_tree_spruce(float3 b_pos, float random, float3 tree_pos) {
    TreeSDF result;

    result.trunk_dist = 1e9;
    result.leaves_dist = 1e9;

    result.trunk_dist = sd_capsule(b_pos, tree_pos, tree_pos + float3(0, 9, 0), 1);

    result.leaves_dist = min(result.leaves_dist, sd_cone(b_pos, tree_pos + float3(0, 1, 0), tree_pos + float3(0, 2.01, 0), 2.2, 2.2));
    result.leaves_dist = min(result.leaves_dist, sd_cone(b_pos, tree_pos + float3(0, 3, 0), tree_pos + float3(0, 4.01, 0), 3.1, 3.1));
    result.leaves_dist = min(result.leaves_dist, sd_torus(b_pos, tree_pos + float3(0, 3, 0), float2(3.6, 0.5 - random * 0.2)));
    result.leaves_dist = min(result.leaves_dist, sd_cone(b_pos, tree_pos + float3(0, 5, 0), tree_pos + float3(0, 6.01, 0), 2.8, 2.8));
    result.leaves_dist = min(result.leaves_dist, sd_cone(b_pos, tree_pos + float3(0, 7, 0), tree_pos + float3(0, 8.01, 0), 2.2, 2.2));
    result.leaves_dist = min(result.leaves_dist, sd_cone(b_pos, tree_pos + float3(0, 9, 0), tree_pos + float3(0, 13.0, 0), 1.4, 0.0));

    return result;
}

TreeSDF sd_tree_pine(float3 b_pos, float random, float3 tree_pos) {
    TreeSDF result;

    result.trunk_dist = 1e9;
    result.leaves_dist = 1e9;

    result.leaves_dist = min(result.leaves_dist, sd_ellipsoid(b_pos, tree_pos + float3(+0, 53, -8.0), float3(4 - random * 2, 1.1 + random * 0.5, 3)));
    result.leaves_dist = min(result.leaves_dist, sd_ellipsoid(b_pos, tree_pos + float3(+1, 52, +8.0), float3(4 - random * 2, 1.1 + random * 0.5, 3)));
    result.leaves_dist = min(result.leaves_dist, sd_ellipsoid(b_pos, tree_pos + float3(-8, 51, -1.0), float3(4 - random * 2, 1.1 + random * 0.5, 3)));
    result.leaves_dist = min(result.leaves_dist, sd_ellipsoid(b_pos, tree_pos + float3(+8, 52, +0.5), float3(4 - random * 2, 1.1 + random * 0.5, 3)));

    result.trunk_dist = min(result.trunk_dist, sd_capsule(b_pos, tree_pos + float3(0, 2, 0), tree_pos + float3(0, 40, 0), 1.2));
    result.trunk_dist = min(result.trunk_dist, sd_capsule(b_pos, tree_pos + float3(0.5, 40, 0.5), tree_pos + float3(0.5, 50, 0.5), 1 - random * 0.35));
    result.trunk_dist = min(result.trunk_dist, sd_cone(b_pos, tree_pos + float3(0, 0, 0), tree_pos + float3(0, 8, 0), 2.6, 0));

    result.trunk_dist = min(result.trunk_dist, sd_cylinder(b_pos, tree_pos + float3(0, 15, 0), tree_pos + float3(5, 14, 1), 0.6));
    result.trunk_dist = min(result.trunk_dist, sd_cylinder(b_pos, tree_pos + float3(0, 25, 0), tree_pos + float3(-3, 24, -2), 0.6));

    result.trunk_dist = min(result.trunk_dist, sd_cylinder(b_pos, tree_pos + float3(0, 40, 0), tree_pos + float3(-2, 43, -3), 0.6));
    result.trunk_dist = min(result.trunk_dist, sd_cylinder(b_pos, tree_pos + float3(0, 40, 0), tree_pos + float3(0, 43, -5), 0.6));
    result.trunk_dist = min(result.trunk_dist, sd_cylinder(b_pos, tree_pos + float3(0, 39, 0), tree_pos + float3(1, 42, 5), 0.6));
    result.trunk_dist = min(result.trunk_dist, sd_cylinder(b_pos, tree_pos + float3(0, 40, 0), tree_pos + float3(-5, 43, 1), 0.6));
    result.trunk_dist = min(result.trunk_dist, sd_cylinder(b_pos, tree_pos + float3(0, 36, 0), tree_pos + float3(5, 42, 0.5), 0.6));

    result.trunk_dist = min(result.trunk_dist, sd_cylinder(b_pos, tree_pos + float3(0, 50, 0), tree_pos + float3(0, 53, -8), 0.7));
    result.trunk_dist = min(result.trunk_dist, sd_cylinder(b_pos, tree_pos + float3(0, 49, 0), tree_pos + float3(1, 52, 8), 0.7));
    result.trunk_dist = min(result.trunk_dist, sd_cylinder(b_pos, tree_pos + float3(0, 50, 0), tree_pos + float3(-8, 51, -1), 0.7));
    result.trunk_dist = min(result.trunk_dist, sd_cylinder(b_pos, tree_pos + float3(0, 49, 0), tree_pos + float3(8, 52, 0.5), 0.7));

    return result;
}
