#include <daxa/daxa.inl>
#include <daxa/utils/task_graph.inl>

struct TestU64Alignment
{
    daxa_u32 i0;
    daxa_u32 i1;
    daxa_u32 i2;
    daxa_u64 i3;
    daxa_u32 i4;
    daxa_u64 i5[3];
    daxa_u32 i6[3];
    daxa_u64 i7;
};
DAXA_DECL_BUFFER_PTR_ALIGN(TestU64Alignment, 8)

DAXA_DECL_TASK_HEAD_BEGIN(TestShaderTaskHead)
DAXA_TH_BUFFER_PTR(COMPUTE_SHADER_READ,         daxa_BufferPtr(TestU64Alignment),   align_test_src)
DAXA_TH_BUFFER_PTR(COMPUTE_SHADER_READ_WRITE,   daxa_RWBufferPtr(TestU64Alignment), align_test_dst)
DAXA_DECL_TASK_HEAD_END

struct TestShaderPush
{
    DAXA_TH_BLOB(TestShaderTaskHead, attachments)
};

// Only used to check complication of All Task Head Uses:
DAXA_DECL_TASK_HEAD_BEGIN(TestTaskHead)
// Declares used task image, that is NOT accessable within the shader.
DAXA_TH_IMAGE(              TRANSFER_READ, REGULAR_2D,                      copy_read_img)
// Declares used task buffer, that is NOT accessable within the shader.
DAXA_TH_BUFFER(             TRANSFER_READ,                                  copy_read_buf)
// Declares used task image, that is used as a storage image with type 2d, accessable in shader as glsl type image2D.
DAXA_TH_IMAGE_ID(           COMPUTE_SHADER_STORAGE_READ_ONLY, REGULAR_2D,   shader_id_read_img)
// Declares used task buffer, accessable in shader as a buffer id.
DAXA_TH_BUFFER_ID(          COMPUTE_SHADER_READ,                            shader_id_read_buf)
// Declares used task buffer likfe above, with the difference that its a buffer ptr in the shader.
DAXA_TH_BUFFER_PTR(         COMPUTE_SHADER_READ, daxa_BufferPtr(uint),      shader_ptr_read_buf)
DAXA_DECL_TASK_HEAD_END

// The above task head will be translated to the following struct in glsl:
// struct TestTaskHead {
//   daxa_ImageViewId shader_id_read_img;
//   daxa_ImageViewId shader_id_read_array_img[2];
//   daxa_BufferId shader_id_read_buf;
//   daxa_BufferId shader_id_read_array_buf[2];
//   daxa_BufferPtr(uint) shader_ptr_read_buf;
//   daxa_BufferPtr(uint) shader_ptr_read_array_buf[2];
// };

struct TestTaskPush
{
    // The Shader blob can be referenced within shared files via the macro DAXA_TH_BLOB like so to declare shared structs:
    // This will become `TestTaskHead attachments` in shader code and  `TestTaskHead::AttachmentShaderBlob attachments` in c++.
    // Both have the exact same size and alignment, so code sharing works like normal.
    DAXA_TH_BLOB(TestTaskHead, attachments)
};

// Test compilation of shared functions with shader shared types
daxa_u32 test_fn_u32(daxa_u32 by_value)
{
    return (by_value * by_value) * 4;
}

struct SomeStruct
{
    daxa_f32 value;
};
DAXA_DECL_BUFFER_PTR(SomeStruct)

struct RWHandles
{
    daxa_RWBufferPtr(SomeStruct) my_buffer;
    daxa_ImageViewId my_float_image;
    daxa_ImageViewId my_uint_image;
    daxa_SamplerId my_sampler;
};
DAXA_DECL_BUFFER_PTR(RWHandles)

struct BindlessTestPush
{
    RWHandles handles;
    daxa_RWBufferPtr(RWHandles) next_shader_input;
};

struct Handles
{
    daxa_BufferPtr(SomeStruct) my_buffer;
    daxa_ImageViewId my_image;
    daxa_SamplerId my_sampler;
};
DAXA_DECL_BUFFER_PTR(Handles)

struct BindlessTestFollowPush
{
    daxa_BufferPtr(Handles) shader_input;
};