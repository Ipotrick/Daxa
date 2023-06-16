# Shader Integration

One of Daxas goals is making shader development easier.

Daxa achieves this in multiple ways, for example the [bindless integration](https://github.com/Ipotrick/Daxa/tree/master/wiki/Bindless.md) on the cpu and gpu side. But as a basis for this daxa also provides tools for easy code sharing between shaders and c++.

The details on code sharing and general shader integration will be described in this file.

## C++/Shader Code Sharing

The main motivation behind code sharing is defining things like structs stored in buffers a single time and then using them in c++ and shaders seemlessly. This concept extends to other parts of the api as well, for example [TaskList use lists can be declared in shared files](https://github.com/Ipotrick/Daxa/tree/master/wiki/TaskList.md) to reduce code duplication as well.

Code is shared by creating header files that get included in c++ and glsl. THe trick is that these files MUST include the `<daxa/daxa.inl>` file. This file contains variations of makros that are switched based on the compiled language. So in essence the code sharing in daxa relies in the shared glsl and c++ preprocessor.

The makros in `<daxa/daxa.inl>` are mainly primitive data types as well as daxa structs and helper utilities.

An example of a daxa shared file:

```glsl
#include <daxa/daxa.inl>

struct MyStruct
{
    daxa_u32vec2 unsigned_integer_vector2;
};
```

`daxa_u32` in this struct is a makro defined by daxa. This makro is translated to either `uvec2` in glsl, `uint2` in hlsl or `daxa::types::f32vec2` in c++.

The full list of defined data types in daxa can be found in the `daxa.inl` file. 

Daxa always declares the alignment of buffer references to be in [scalar block layout](https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GL_EXT_scalar_block_layout.txt).
This layout causes shared structs containing daxa_ types to have the alignment rules within c++ and glsl buffers. This means that the c++ and glsl version of the struct will be abi compatible. You do not need to worry about padding or the behavior of alignment with the usua lglsl format layouts like  std 140 or std430.

## Utility Makros

For the shared structs to properly work in other situations daxa also provides some utility makros to make it easier to write shaders.

`DAXA_DECL_PUSH_CONSTANT(STRUCT, NAME)` takes a struct and declares a push constant with the given name in global scope.

Example:
```glsl
struct Push
{
    daxa_u32 field;
};

DAXA_DECL_PUSH_CONSTANT(Push, push)

layout(local_size_x = 1)
void main()
{
    uint value = push.field;
}
```

`DAXA_DECL_UNIFORM_BUFFER(SLOT)` defines the beginning of a uniform buffer declaration. Daxa provides fixed 8 uniform buffer slots. 

Example:
```glsl
DAXA_DECL_UNIFORM_BUFFER(0) BufferBlock
{
    daxa_u32 field;
};
```
