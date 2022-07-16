struct Push
{
    uint globals_id;
};

//[[vk::push_constant]] const Push p;

[numthreads(1, 1, 1)] void main()
{
    //StructuredBuffer<Globals> globals = daxa::get_buffer<Globals>(p.globals_id);
}
