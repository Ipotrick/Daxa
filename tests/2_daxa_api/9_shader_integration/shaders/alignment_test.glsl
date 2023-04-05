#include <daxa/daxa.inl>
#include "shared.inl"

layout(local_size_x = 1) in;
void main()
{
    deref(align_test_dst) = deref(align_test_src);
}