#include "daxa/daxa.inl"

#include "shared.inl"

const static float2 positions[3] = {
  float2(0.0, -0.5),
  float2(0.5, 0.5),
  float2(-0.5, 0.5)
};

const static float3 colors[3] = {
  float3(1.0, 1.0, 0.0),
  float3(0.0, 1.0, 1.0),
  float3(1.0, 0.0, 1.0)
};

struct Vertex
{
  float4 pos : SV_Position;
  [[vk::location(0)]] float3 color;
};

struct Primitive
{
  [[vk::location(0)]] float3 tint;
}

const static uint MAX_VERTS = 3;
const static uint MAX_PRIMS = 1;

[outputtopology("triangle")]
[numthreads(3, 1, 1)]
[shader("mesh")]
void entry_mesh(
    in uint tig : SV_GroupIndex,
    OutputVertices<Vertex, MAX_VERTS> verts,
    OutputIndices<uint3, MAX_PRIMS> triangles,
    OutputPrimitives<Primitive, MAX_PRIMS> primitives)
{
    const uint numVertices = 3;
    const uint numPrimitives = 1;
    SetMeshOutputCounts(numVertices, numPrimitives);

    if(tig < numVertices) {
        verts[tig] = {float4(positions[tig], 0, 1), colors[tig]};
    }

    if(tig < numPrimitives) {
        triangles[tig] = uint3(0,1,2);
        primitives[tig] = { float3(1,0,0) };
    }
}

struct FragmentOut
{
    [[vk::location(0)]] float3 color;
};

[shader("fragment")]
FragmentOut entry_fragment(in Vertex vertex, in Primitive prim)
{
    FragmentOut frag_out;
    frag_out.color = vertex.color * prim.tint;
    return frag_out;
}