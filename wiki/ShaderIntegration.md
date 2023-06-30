# Shader Integration

One of Daxa's goals is making shader development easier.

Daxa achieves this in multiple ways, for example the [bindless integration](https://github.com/Ipotrick/Daxa/tree/master/wiki/Bindless.md) on the CPU and GPU side. But as a basis for this Daxa also provides tools for easy code sharing between shaders and C++.

The details on code sharing and general shader integration will be described in this file.

## C++/Shader Code Sharing

The main motivation behind code sharing is defining things like structs stored in buffers a single time and then using them in C++ and shaders seamlessly. This concept extends to other parts of the API as well, for example [TaskGraph *`uses`* lists can be declared in shared files](https://github.com/Ipotrick/Daxa/tree/master/wiki/TaskGraph.md) to reduce code duplication as well.

Code is shared by creating header files that get included in C++ and GLSL. THe trick is that these files MUST include the `<daxa/daxa.inl>` file. This file contains variations of macros that are switched based on the compiled language. So in essence the code sharing in Daxa relies in the shared GLSL and C++ preprocessor.

The macros in `<daxa/daxa.inl>` are mainly primitive data types as well as Daxa structs and helper utilities.

An example of a Daxa shared file:

```glsl
#include <daxa/daxa.inl>

struct MyStruct
{
    daxa_u32vec2 unsigned_integer_vector2;
};
```

`daxa_u32` in this struct is a macro defined by daxa. This macro is translated to either `uvec2` in GLSL, `uint2` in hlsl or `daxa::types::f32vec2` in C++.

The full list of defined data types in Daxa can be found in the `daxa.inl` file. 

Daxa always declares the alignment of buffer references to be in [scalar block layout](https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GL_EXT_scalar_block_layout.txt).
This layout causes shared structs containing daxa_ types to have the alignment rules within C++ and GLSL buffers. This means that the C++ and GLSL version of the struct will be abi compatible. You do not need to worry about padding or the behavior of alignment with the usual GLSL format layouts like  std 140 or std430.

## Utility Macros

For the shared structs to properly work in other situations, Daxa also provides some utility macros to make it easier to write shaders.

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
