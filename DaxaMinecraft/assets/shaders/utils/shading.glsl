
float calc_light_point(in vec3 v_pos, in vec3 v_nrm, vec3 light_pos) {
    vec3 light_del = light_pos - v_pos;
    float distsq = dot(light_del, light_del);
    vec3 light_del_dir = normalize(light_del);
    vec3 nrm = normalize(v_nrm);
    float dotnrm = dot(nrm, light_del_dir);
    return max(0, dotnrm / distsq);
}

float calc_light_directional(in vec3 v_nrm, vec3 light_dir) {
    vec3 nrm = normalize(v_nrm);
    float dotnrm = dot(nrm, light_dir);
    return max(0, dotnrm);
}
