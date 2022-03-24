float M_PI = 3.14159265359;

vec2 mulComplex(vec2 a, vec2 b) {
    return vec2(
        a.x*b.x - a.y*b.y,
        a.x*b.y + a.y*b.x
    );
}

vec2 expi(float x) {
    return vec2(
        cos(x),
        sin(x)
    );
}

vec2 expNegi(float x) {
    return vec2(
        cos(x),
        -sin(x)
    );
}

vec2 calcW(float k, float n, float invN) {
    return expNegi(2 * M_PI * k * n * invN);
}