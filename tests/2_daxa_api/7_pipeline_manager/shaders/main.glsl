#extension GL_EXT_scalar_block_layout : require

layout(scalar, binding = 4, set = 1) restrict readonly buffer Block { uint values[]; };

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
}
