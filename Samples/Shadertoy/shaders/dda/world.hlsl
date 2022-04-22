#pragma once

#include "utils/noise.hlsl"

#define CHUNK_SIZE BUF0_SIZE

template<uint N>
int2 get_tile_i(float2 p) {
    int2 tile_i = int2(p.x, p.y);
    if (p.x < 0)
        tile_i.x -= 1;
    if (p.y < 0)
        tile_i.y -= 1;
    tile_i /= N;
    tile_i *= N;
    return tile_i;
}

float2 pixelspace_to_worldspace(StructuredBuffer<Globals> globals, float2 p) {
    float2 uv = p / float2(globals[0].frame_dim);
    uv.x *= float(globals[0].frame_dim.x) / globals[0].frame_dim.y;
    return uv * CHUNK_SIZE / 2;
}

template <uint DIV>
float3 get_color() {
    return float3(1.0, 0.2, 1.0);
}
template <>
float3 get_color<0>() {
    return float3(1.0, 1.0, 1.0);
}
template <>
float3 get_color<1>() {
    return float3(1.0, 0.0, 0.0);
}
template <>
float3 get_color<2>() {
    return float3(1.0, 1.0, 0.0);
}
template <>
float3 get_color<3>() {
    return float3(0.0, 1.0, 0.0);
}
template <>
float3 get_color<4>() {
    return float3(0.0, 1.0, 1.0);
}
template <>
float3 get_color<5>() {
    return float3(0.0, 0.0, 1.0);
}

template <uint N>
bool check_presence(in StructuredBuffer<Buf0> buf0, in int2 tile_i) {
    bool present = false;
    // for (int yi = 0; yi < N; ++yi)
    //     for (int xi = 0; xi < N; ++xi) present |= check_presence<1>(buf0, tile_i * N + int2(xi, yi));
    return present;
}

template <>
bool check_presence<1>(in StructuredBuffer<Buf0> buf0, in int2 tile_i) {
    if (tile_i.x >= 0 && tile_i.x < CHUNK_SIZE && tile_i.y >= 0 && tile_i.y < CHUNK_SIZE)
        return buf0[0].data_0[tile_i.x + tile_i.y * CHUNK_SIZE] > 0;
    return false;
}
template <>
bool check_presence<2>(in StructuredBuffer<Buf0> buf0, in int2 tile_i) {
    const uint DIV = 2;
    tile_i = int2(tile_i.x / DIV, tile_i.y / DIV);
    if (tile_i.x >= 0 && tile_i.x < (CHUNK_SIZE / DIV) && tile_i.y >= 0 && tile_i.y < (CHUNK_SIZE / DIV))
        return buf0[0].data_1[tile_i.x + tile_i.y * CHUNK_SIZE / DIV] > 0;
    return false;
}
template <>
bool check_presence<4>(in StructuredBuffer<Buf0> buf0, in int2 tile_i) {
    const uint DIV = 4;
    tile_i = int2(tile_i.x / DIV, tile_i.y / DIV);
    if (tile_i.x >= 0 && tile_i.x < (CHUNK_SIZE / DIV) && tile_i.y >= 0 && tile_i.y < (CHUNK_SIZE / DIV))
        return buf0[0].data_2[tile_i.x + tile_i.y * CHUNK_SIZE / DIV] > 0;
    return false;
}
template <>
bool check_presence<8>(in StructuredBuffer<Buf0> buf0, in int2 tile_i) {
    const uint DIV = 8;
    tile_i = int2(tile_i.x / DIV, tile_i.y / DIV);
    if (tile_i.x >= 0 && tile_i.x < (CHUNK_SIZE / DIV) && tile_i.y >= 0 && tile_i.y < (CHUNK_SIZE / DIV))
        return buf0[0].data_3[tile_i.x + tile_i.y * CHUNK_SIZE / DIV] > 0;
    return false;
}
template <>
bool check_presence<16>(in StructuredBuffer<Buf0> buf0, in int2 tile_i) {
    const uint DIV = 16;
    tile_i = int2(tile_i.x / DIV, tile_i.y / DIV);
    if (tile_i.x >= 0 && tile_i.x < (CHUNK_SIZE / DIV) && tile_i.y >= 0 && tile_i.y < (CHUNK_SIZE / DIV))
        return buf0[0].data_4[tile_i.x + tile_i.y * CHUNK_SIZE / DIV] > 0;
    return false;
}
template <>
bool check_presence<32>(in StructuredBuffer<Buf0> buf0, in int2 tile_i) {
    const uint DIV = 32;
    tile_i = int2(tile_i.x / DIV, tile_i.y / DIV);
    if (tile_i.x >= 0 && tile_i.x < (CHUNK_SIZE / DIV) && tile_i.y >= 0 && tile_i.y < (CHUNK_SIZE / DIV))
        return buf0[0].data_5[tile_i.x + tile_i.y * CHUNK_SIZE / DIV] > 0;
    return false;
}

uint get_lod(in StructuredBuffer<Buf0> buf0, in int2 tile_i) {
    if (check_presence<1>(buf0, tile_i))
        return 0;
    if (check_presence<2>(buf0, tile_i))
        return 1;
    if (check_presence<4>(buf0, tile_i))
        return 2;
    if (check_presence<8>(buf0, tile_i))
        return 3;
    if (check_presence<16>(buf0, tile_i))
        return 4;
    if (check_presence<32>(buf0, tile_i))
        return 5;
    return 6;
}
uint get_lod(in StructuredBuffer<Buf0> buf0, in float2 p) {
    return get_lod(buf0, get_tile_i<1>(p));
}

template <uint N>
bool check_subtile_presence(in StructuredBuffer<Buf0> buf0, in int2 tile_i) {
    tile_i = tile_i + CHUNK_SIZE / 2;
    if (tile_i.x < 0 || tile_i.x >= CHUNK_SIZE || tile_i.y < 0 || tile_i.y >= CHUNK_SIZE)
        return false;
    tile_i = tile_i / N;
    return check_presence<N>(buf0, tile_i - CHUNK_SIZE / N / 2);
}

bool outside_world(in int2 tile_i) {
    tile_i = tile_i + CHUNK_SIZE / 2;
    if (tile_i.x < 0 || tile_i.x >= CHUNK_SIZE || tile_i.y < 0 || tile_i.y >= CHUNK_SIZE)
        return true;
    return false;
}
