#pragma once

#include "daxa.hlsl"

struct Globals{
    uint hdrImageID;
    uint fftImageID;
    uint horFreqImageRID;
    uint horFreqImageGID;
    uint horFreqImageBID;
    uint width;
    uint height;
    uint padWidth;
    uint padHeight;
};
DAXA_DEFINE_BA_BUFFER(Globals)