struct ChunkBlockPresence {
    uint x2[1024];
    uint x4[128];
    uint x8[16];
    uint x16[2];
    bool x32[8]; 
};

uint x2_uint_bit_mask(uint3 x2_i) {
    return 1u << x2_i.z;
}

uint x2_uint_array_index(uint3 x2_i) {
    return x2_i.x + x2_i.y * 32;
}

uint x4_uint_bit_mask(uint3 x4_i) {
    return 1u << ((x4_i.z & 0xF) + 16 * (x4_i.x & 0x1));
}

uint x4_uint_array_index(uint3 x4_i) {
    return (x4_i.x >> 1 /* / 2 */) + x4_i.y * 8 /* 16 / 2 */;
}