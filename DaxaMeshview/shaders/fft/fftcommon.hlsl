#pragma once

#include "daxa.hlsl"

struct Globals{
    uint hdrImageID;
    uint fftImageID;
    uint horFreqImageRGID;
    uint horFreqImageBAID;
    uint fullFreqRGID;
    uint fullFreqBAID;
    uint kernelID;
    uint kernelImageID;
    uint width;
    uint height;
    uint padWidth;
    uint padHeight;
};
DAXA_DEFINE_BA_BUFFER(Globals)