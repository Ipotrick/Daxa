#include <daxa/daxa.inl>
#include "shared.inl"
#extension GL_EXT_debug_printf : enable

layout(local_size_x = 1) in;
void main()
{
    deref(align_test_dst).i0 = deref(align_test_src).i0;
    deref(align_test_dst).i1 = deref(align_test_src).i1;
    deref(align_test_dst).i2 = deref(align_test_src).i2;
    deref(align_test_dst).i3 = deref(align_test_src).i3;
    deref(align_test_dst).i4 = deref(align_test_src).i4;
    deref(align_test_dst).i5[0] = deref(align_test_src).i5[0];
    deref(align_test_dst).i5[1] = deref(align_test_src).i5[1];
    deref(align_test_dst).i5[2] = deref(align_test_src).i5[2];
    deref(align_test_dst).i6[0] = deref(align_test_src).i6[0];
    deref(align_test_dst).i6[1] = deref(align_test_src).i6[1];
    deref(align_test_dst).i6[2] = deref(align_test_src).i6[2];
    deref(align_test_dst).i7 = deref(align_test_src).i7;
}