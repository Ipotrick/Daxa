#pragma once

#define FAT_ZCURVE 1

struct PresenceBuffer {
    uint x_64[1 << 0];
    uint x_32[1 << 0];
    uint x_16[1 << (3 * 0 + 3)];
    uint x_08[1 << (3 * 1 + 3)];
    uint x_04[1 << (3 * 2 + 3)];
    uint x_02[1 << (3 * 3 + 3)];
    uint x_01[1 << (3 * 4 + 3)];

    struct Index {
        uint i;
        uint shift;
    };

    template <uint N>
    bool x_data(uint i, uint shift);

    template <uint N>
    Index get_index(uint3 in_chunk_p) {
        uint i, i_;
#if FAT_ZCURVE
        uint3 pL = in_chunk_p / 16;
        uint3 pM = in_chunk_p / 4 - pL * 4;
        uint3 pS = in_chunk_p % 4;

        uint indexL = pL.x + pL.y * 4 + pL.z * 4 * 4;
        uint indexM = pM.x + pM.y * 4 + pM.z * 4 * 4;
        uint indexS = pS.x + pS.y * 4 + pS.z * 4 * 4;

        i_ = indexS + indexM * 64 + indexL * 64 * 64;
#else
        i_ = in_chunk_p.x + in_chunk_p.y * 64 + in_chunk_p.z * 64 * 64;
#endif

        i_ /= (1u << (3 * uint(N)));
        i = i_ / 32;

        uint shift = i_ - i * 32;
        Index result;
        result.i = i;
        result.shift = shift;
        return result;
    }

    template <uint N>
    uint load_data(Index index) {
        return x_data<N>(index.i, index.shift);
    }

    template <uint N>
    bool load_presence(uint3 p) {
        return (load_data<N>(get_index<N>(p)) & 0x1) != 0;
    }

    uint load_lod(uint3 p);

    uint load_medium(uint3 p) {
        return load_data<0>(get_index<0>(p)) & 0xe;
    }
};

template <>
bool PresenceBuffer::x_data<6>(uint i, uint shift) { return x_64[0]; }
template <>
bool PresenceBuffer::x_data<5>(uint i, uint shift) { return (x_32[i] & (0xfu << shift)) >> shift; }
template <>
bool PresenceBuffer::x_data<4>(uint i, uint shift) { return (x_16[i] & (0xfu << shift)) >> shift; }
template <>
bool PresenceBuffer::x_data<3>(uint i, uint shift) { return (x_08[i] & (0xfu << shift)) >> shift; }
template <>
bool PresenceBuffer::x_data<2>(uint i, uint shift) { return (x_04[i] & (0xfu << shift)) >> shift; }
template <>
bool PresenceBuffer::x_data<1>(uint i, uint shift) { return (x_02[i] & (0xfu << shift)) >> shift; }
template <>
bool PresenceBuffer::x_data<0>(uint i, uint shift) { return (x_01[i] & (0xfu << shift)) >> shift; }

template <>
bool PresenceBuffer::load_presence<6>(uint3 p) { return x_64[0] != 0; }

uint PresenceBuffer::load_lod(uint3 p) {
    if (load_presence<6>(p))
        return 6;
    if (load_presence<5>(p))
        return 5;
    if (load_presence<4>(p))
        return 4;
    if (load_presence<3>(p))
        return 3;
    if (load_presence<2>(p))
        return 2;
    if (load_presence<1>(p))
        return 1;
    return 0;
}