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

struct Payload {
    float3 color;
};

[outputtopology("triangle")]
[numthreads(3, 1, 1)]
[shader("mesh")]
void entry_mesh(
    payload Payload payload,
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
        primitives[tig].tint = float3(1,0.5,0) * payload.color;
        primitives[tig].cull = false;
    }
}

[shader("amplification")]
void entry_task()
{
    Payload payload;
    payload.color = float3(0,1,1);
    DispatchMesh(1, 1, 1, payload);
}

struct FragmentOut
{
    [[vk::location(0)]] float3 color;
};

[shader("fragment")]
FragmentOut entry_fragment(in Vertex vertex, in Primitive daxa_prim_in)
{
    FragmentOut frag_out;
    frag_out.color = daxa_prim_in.tint;
    return frag_out;
}
