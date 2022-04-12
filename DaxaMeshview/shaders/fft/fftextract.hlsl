#include "daxa.hlsl"
#include "fftutil.hlsl"

struct Push {
    uint globalsID;
};
[[vk::push_constant]] Push p;

[numthreads(8,8,1)]
void Main() {

}