#pragma once

#if !defined(DAXA_GPU_TABLE_SET_BINDING)
#define DAXA_GPU_TABLE_SET_BINDING 0
#define DAXA_STORAGE_BUFFER_BINDING 0
#define DAXA_STORAGE_IMAGE_BINDING 1
#define DAXA_SAMPLED_IMAGE_BINDING 2
#define DAXA_SAMPLER_BINDING 3
#define DAXA_BUFFER_DEVICE_ADDRESS_BUFFER_BINDING 4
#define DAXA_ACCELERATION_STRUCTURE_BINDING 5
#endif

#define DAXA_ID_INDEX_BITS 20
#define DAXA_ID_INDEX_MASK ((uint64_t(1) << DAXA_ID_INDEX_BITS) - uint64_t(1))
#define DAXA_ID_INDEX_OFFSTET 0
#define DAXA_ID_VERSION_BITS 44
#define DAXA_ID_VERSION_MASK ((uint64_t(1) << DAXA_ID_VERSION_BITS) - uint64_t(1))
#define DAXA_ID_VERSION_OFFSTET DAXA_ID_INDEX_BITS

#define _DAXA_DECL_VEC_TYPES(SLANG_TYPE, DAXA_TYPE) \
    typedef SLANG_TYPE DAXA_TYPE;                   \
    typedef SLANG_TYPE ## 1 DAXA_TYPE ## vec1;      \
    typedef SLANG_TYPE ## 2 DAXA_TYPE ## vec2;      \
    typedef SLANG_TYPE ## 3 DAXA_TYPE ## vec3;      \
    typedef SLANG_TYPE ## 4 DAXA_TYPE ## vec4;

#define _DAXA_DECL_MAT_TYPES(SLANG_TYPE, DAXA_TYPE)      \
    typedef matrix<SLANG_TYPE, 1, 1> DAXA_TYPE ## mat1x1 \
    typedef matrix<SLANG_TYPE, 2, 1> DAXA_TYPE ## mat1x2 \
    typedef matrix<SLANG_TYPE, 3, 1> DAXA_TYPE ## mat1x3 \
    typedef matrix<SLANG_TYPE, 4, 1> DAXA_TYPE ## mat1x4 \
    typedef matrix<SLANG_TYPE, 1, 2> DAXA_TYPE ## mat2x1 \
    typedef matrix<SLANG_TYPE, 2, 2> DAXA_TYPE ## mat2x2 \
    typedef matrix<SLANG_TYPE, 3, 2> DAXA_TYPE ## mat2x3 \
    typedef matrix<SLANG_TYPE, 4, 2> DAXA_TYPE ## mat2x4 \
    typedef matrix<SLANG_TYPE, 1, 3> DAXA_TYPE ## mat3x1 \
    typedef matrix<SLANG_TYPE, 2, 3> DAXA_TYPE ## mat3x2 \
    typedef matrix<SLANG_TYPE, 3, 3> DAXA_TYPE ## mat3x3 \
    typedef matrix<SLANG_TYPE, 4, 3> DAXA_TYPE ## mat3x4 \
    typedef matrix<SLANG_TYPE, 1, 4> DAXA_TYPE ## mat4x1 \
    typedef matrix<SLANG_TYPE, 2, 4> DAXA_TYPE ## mat4x2 \
    typedef matrix<SLANG_TYPE, 3, 4> DAXA_TYPE ## mat4x3 \
    typedef matrix<SLANG_TYPE, 4, 4> DAXA_TYPE ## mat4x4

#define _DAXA_DECL_VEC_MAT_8_16_32_64_TYPES(SLANG_TYPE, DAXA_TYPE)\
    _DAXA_DECL_VEC_TYPES(SLANG_TYPE ## 8_t, DAXA_TYPE ## 8)\
    _DAXA_DECL_VEC_TYPES(SLANG_TYPE ## 16_t, DAXA_TYPE ## 16)\
    _DAXA_DECL_VEC_TYPES(SLANG_TYPE ## 32_t, DAXA_TYPE ## 32)\
    _DAXA_DECL_VEC_TYPES(SLANG_TYPE ## 64_t, DAXA_TYPE ## 64)\
    _DAXA_DECL_MAT_TYPES(SLANG_TYPE ## 8_t, DAXA_TYPE ## 8)\
    _DAXA_DECL_MAT_TYPES(SLANG_TYPE ## 16_t, DAXA_TYPE ## 16)\
    _DAXA_DECL_MAT_TYPES(SLANG_TYPE ## 32_t, DAXA_TYPE ## 32)\
    _DAXA_DECL_MAT_TYPES(SLANG_TYPE ## 64_t, DAXA_TYPE ## 64)

namespace daxa
{
    _DAXA_DECL_VEC_MAT_8_16_32_64_TYPES(uint, u)
    _DAXA_DECL_VEC_MAT_8_16_32_64_TYPES(int, i)
    _DAXA_DECL_VEC_TYPES(half, f16)
    _DAXA_DECL_VEC_TYPES(float, f32)
    _DAXA_DECL_VEC_TYPES(double, f64)
    _DAXA_DECL_MAT_TYPES(half, f16)
    _DAXA_DECL_MAT_TYPES(float, f32)
    _DAXA_DECL_MAT_TYPES(double, f64)
    _DAXA_DECL_VEC_TYPES(bool, b32)
    _DAXA_DECL_MAT_TYPES(bool, b32)
    
    struct BufferId
    {
        daxa_u64 value;
        daxa_u32 index()
        {
            return daxa_u32(DAXA_ID_INDEX_MASK & value);
        }
        daxa_u64 version()
        {
            return (value >> DAXA_ID_VERSION_OFFSTET);
        }
    };

    struct ImageViewId
    {
        daxa_u64 value;
        daxa_u32 index()
        {
            return daxa_u32(DAXA_ID_INDEX_MASK & value);
        }
        daxa_u64 version()
        {
            return (value >> DAXA_ID_VERSION_OFFSTET);
        }
    };

    struct ImageViewIndex
    {
        daxa_u32 value;
        daxa_u32 index()
        {
            return value;
        }
    };

    struct SamplerId
    {
        daxa_u64 value;
        daxa_u32 index()
        {
            return daxa_u32(DAXA_ID_INDEX_MASK & value);
        }
        daxa_u64 version()
        {
            return (value >> DAXA_ID_VERSION_OFFSTET);
        }
    };
    
    [[vk::binding(DAXA_SAMPLER_BINDING, 0)]] SamplerState SamplerStateTable[];
    extension SamplerState
    {
        static SamplerState get(daxa::SamplerId i) { return SamplerStateTable[i.index()]; }
    }

    [[vk::binding(DAXA_STORAGE_BUFFER_BINDING, 0)]] ByteAddressBuffer ByteAddressBufferTable[];
    extension ByteAddressBuffer
    {
        static ByteAddressBuffer get(daxa::BufferId i) { return ByteAddressBufferTable[i.index()]; }
    }

    [[vk::binding(DAXA_STORAGE_BUFFER_BINDING, 0)]] RWByteAddressBuffer RWByteAddressBufferTable[];
    [[vk::binding(DAXA_STORAGE_BUFFER_BINDING, 0)]] coherent RWByteAddressBuffer CoherentRWByteAddressBufferTable[];
    extension RWByteAddressBuffer
    {
        static RWByteAddressBuffer get(daxa::BufferId i) { return RWByteAddressBufferTable[i.index()]; }
        static RWByteAddressBuffer get_coherent(daxa::BufferId i) { return CoherentRWByteAddressBufferTable[i.index()]; }
    }
    
    [[vk::binding(DAXA_BUFFER_DEVICE_ADDRESS_BUFFER_BINDING, 0)]] StructuredBuffer<daxa_u64> buffer_addresses;
    func id_to_address(daxa::BufferId buffer_id) -> daxa_u64
    {
        return buffer_addresses[buffer_id.index()];
    }

    #define _DAXA_DECL_TEXTURE_ACCESSOR_f32u32i32(BINDING, IMAGE_VIEW_TYPE)\
        [[vk::binding(BINDING, 0)]] IMAGE_VIEW_TYPE<daxa::f32vec4> IMAGE_VIEW_TYPE##_ftable4[];\
        [[vk::binding(BINDING, 0)]] IMAGE_VIEW_TYPE<daxa::u32vec4> IMAGE_VIEW_TYPE##_utable4[];\
        [[vk::binding(BINDING, 0)]] IMAGE_VIEW_TYPE<daxa::i32vec4> IMAGE_VIEW_TYPE##_itable4[];\
        [[vk::binding(BINDING, 0)]] globallycoherent IMAGE_VIEW_TYPE<daxa::f32vec4> IMAGE_VIEW_TYPE##_coherent_ftable4[];\
        [[vk::binding(BINDING, 0)]] globallycoherent IMAGE_VIEW_TYPE<daxa::u32vec4> IMAGE_VIEW_TYPE##_coherent_utable4[];\
        [[vk::binding(BINDING, 0)]] globallycoherent IMAGE_VIEW_TYPE<daxa::i32vec4> IMAGE_VIEW_TYPE##_coherent_itable4[];\
        extension IMAGE_VIEW_TYPE<float>\
        {\
            static IMAGE_VIEW_TYPE<daxa::f32vec4> get(daxa::ImageViewId i) { return IMAGE_VIEW_TYPE##_ftable4[i.index()]; }\
            static IMAGE_VIEW_TYPE<daxa::f32vec4> get_coherent(daxa::ImageViewId i) { return IMAGE_VIEW_TYPE##_coherent_ftable4[i.index()]; }\
            static IMAGE_VIEW_TYPE<daxa::f32vec4> get(daxa::ImageViewIndex i) { return IMAGE_VIEW_TYPE##_ftable4[i.value]; }\
            static IMAGE_VIEW_TYPE<daxa::f32vec4> get_coherent(daxa::ImageViewIndex i) { return IMAGE_VIEW_TYPE##_coherent_ftable4[i.value]; }\
        }    \
        extension IMAGE_VIEW_TYPE<uint>\
        {\
            static IMAGE_VIEW_TYPE<daxa::u32vec4> get(daxa::ImageViewId i) { return IMAGE_VIEW_TYPE##_utable4[i.index()]; }\
            static IMAGE_VIEW_TYPE<daxa::u32vec4> get_coherent(daxa::ImageViewId i) { return IMAGE_VIEW_TYPE##_coherent_utable4[i.index()]; }\
            static IMAGE_VIEW_TYPE<daxa::u32vec4> get(daxa::ImageViewIndex i) { return IMAGE_VIEW_TYPE##_utable4[i.value]; }\
            static IMAGE_VIEW_TYPE<daxa::u32vec4> get_coherent(daxa::ImageViewIndex i) { return IMAGE_VIEW_TYPE##_coherent_utable4[i.value]; }\
        }    \
        extension IMAGE_VIEW_TYPE<int>\
        {\
            static IMAGE_VIEW_TYPE<daxa::i32vec4> get(daxa::ImageViewId i) { return IMAGE_VIEW_TYPE##_itable4[i.index()]; }\
            static IMAGE_VIEW_TYPE<daxa::i32vec4> get_coherent(daxa::ImageViewId i) { return IMAGE_VIEW_TYPE##_coherent_itable4[i.index()]; }\
            static IMAGE_VIEW_TYPE<daxa::i32vec4> get(daxa::ImageViewIndex i) { return IMAGE_VIEW_TYPE##_itable4[i.value]; }\
            static IMAGE_VIEW_TYPE<daxa::i32vec4> get_coherent(daxa::ImageViewIndex i) { return IMAGE_VIEW_TYPE##_coherent_itable4[i.value]; }\
        }
    
    _DAXA_DECL_TEXTURE_ACCESSOR_f32u32i32(DAXA_STORAGE_IMAGE_BINDING, RWTexture1D)
    _DAXA_DECL_TEXTURE_ACCESSOR_f32u32i32(DAXA_STORAGE_IMAGE_BINDING, RWTexture2D)
    _DAXA_DECL_TEXTURE_ACCESSOR_f32u32i32(DAXA_STORAGE_IMAGE_BINDING, RWTexture3D)
    _DAXA_DECL_TEXTURE_ACCESSOR_f32u32i32(DAXA_STORAGE_IMAGE_BINDING, RWTexture1DArray)
    _DAXA_DECL_TEXTURE_ACCESSOR_f32u32i32(DAXA_STORAGE_IMAGE_BINDING, RWTexture2DArray)
    
    _DAXA_DECL_TEXTURE_ACCESSOR_f32u32i32(DAXA_SAMPLED_IMAGE_BINDING, Texture1D)
    _DAXA_DECL_TEXTURE_ACCESSOR_f32u32i32(DAXA_SAMPLED_IMAGE_BINDING, Texture2D)
    _DAXA_DECL_TEXTURE_ACCESSOR_f32u32i32(DAXA_SAMPLED_IMAGE_BINDING, Texture3D)
    _DAXA_DECL_TEXTURE_ACCESSOR_f32u32i32(DAXA_SAMPLED_IMAGE_BINDING, Texture1DArray)
    _DAXA_DECL_TEXTURE_ACCESSOR_f32u32i32(DAXA_SAMPLED_IMAGE_BINDING, Texture2DArray)
    _DAXA_DECL_TEXTURE_ACCESSOR_f32u32i32(DAXA_SAMPLED_IMAGE_BINDING, TextureCube)
    _DAXA_DECL_TEXTURE_ACCESSOR_f32u32i32(DAXA_SAMPLED_IMAGE_BINDING, TextureCubeArray)
    _DAXA_DECL_TEXTURE_ACCESSOR_f32u32i32(DAXA_SAMPLED_IMAGE_BINDING, Texture2DMS)
} // namespace daxa

// These are hopefully only temporary global namespace declarations:
_DAXA_DECL_VEC_MAT_8_16_32_64_TYPES(uint, daxa_u)
_DAXA_DECL_VEC_MAT_8_16_32_64_TYPES(int, daxa_i)
_DAXA_DECL_VEC_TYPES(half, daxa_f16)
_DAXA_DECL_VEC_TYPES(float, daxa_f32)
_DAXA_DECL_VEC_TYPES(double, daxa_f64)
_DAXA_DECL_MAT_TYPES(half, daxa_f16)
_DAXA_DECL_MAT_TYPES(float, daxa_f32)
_DAXA_DECL_MAT_TYPES(double, daxa_f64)
_DAXA_DECL_VEC_TYPES(bool, daxa_b32)
_DAXA_DECL_MAT_TYPES(bool, daxa_b32)
typedef daxa::BufferId daxa_BufferId
typedef daxa::ImageViewId daxa_ImageViewId
typedef daxa::SamplerId daxa_SamplerId
#define daxa_RWBufferPtr(STRUCT_TYPE) Ptr<STRUCT_TYPE>
#define daxa_BufferPtr(STRUCT_TYPE) Ptr<STRUCT_TYPE>
#define DAXA_DECL_BUFFER_PTR_ALIGN(STRUCT_TYPE, ALIGN)
#define DAXA_DECL_BUFFER_PTR(STRUCT_TYPE)
#define deref(PTR) (*PTR)
#define deref_i(PTR, INDEX) PTR[INDEX]
#define as_address(PTR) uint64_t(PTR)