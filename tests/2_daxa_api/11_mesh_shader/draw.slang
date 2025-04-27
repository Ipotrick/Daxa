const static float2 positions[3] = {
    float2(0.0, -0.5),
    float2(0.5, 0.5),
    float2(-0.5, 0.5)
};

struct Vertex
{
    float4 pos : SV_Position;
};

struct Primitive
{
    [[vk::location(0)]] nointerpolation float3 tint;
    bool cull : SV_CullPrimitive;
}

const static uint MAX_VERTS = 3;
const static uint MAX_PRIMS = 1;

[outputtopology("triangle")]
[numthreads(3, 1, 1)]
[shader("mesh")]
void entry_mesh(
    in uint tig : SV_GroupIndex,
    OutputIndices<uint3, MAX_PRIMS> triangles,
    OutputVertices<Vertex, MAX_VERTS> verts,
    OutputPrimitives<Primitive, MAX_PRIMS> primitives)
{
    SetMeshOutputCounts(MAX_VERTS, MAX_PRIMS);

    if (tig < MAX_VERTS)
    {
        verts[tig].pos = float4(positions[tig], 0, 1);
    }

    if(tig < MAX_PRIMS) {
        triangles[tig] = uint3(0,1,2);
        primitives[tig].tint = float3(1,0.5,0);
        primitives[tig].cull = false;
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
    frag_out.color = prim.tint;
    return frag_out;
}
