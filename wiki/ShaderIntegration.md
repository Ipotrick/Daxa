Part of Daxa is making shader development easier.
This includes support for code sharing between GLSL and C++, but also integration with the rest of Daxa.

NOTE: Currently only the GLSL shader integration is maintained!

# Bindless integration (GLSL)

## Motivation and api

The way Daxa does bindless is quite cumbersome with "normal" non-sugared GLSL syntax. To access a GLSL `texture2D` for example the user would need to do all this:

```glsl
#include <daxa/daxa.inl>
layout(set = 0, binding = DAXA_SAMPLED_IMAGE_BINDING) uniform texture2D image2DTable[];
layout(set = 0, binding = DAXA_SAMPLER_BINDING) uniform sampler samplerTable[];

layout(push_constant, scalar) uniform Push
{
    uint texture_index;
    uint sampler_index;
};

void main()
{
    vec2 uv = ...;
    vec4 value = texture(
        sampler2D(
            image2DTable[texture_index], 
            samplerTable[sampler_index]
        ), 
        uv
    );
}
```
This is quite a mouthful compared to non-bindless GLSL:
```glsl
layout(set = 0, binding = 0) uniform texture2D t;
layout(set = 0, binding = 1) uniform sampler s;

void main()
{
    vec2 uv = ...;
    vec4 value = texture(sampler2D(t,s), uv);
}
```

Declaring the tables in this example might seem ok to do, but it can get unreadable and error prone. It is also unnecessary to declare the tables for every used type as all tables could get predefined. Another problem here is that we do not have any type safety, we just index with any uint into the table. One could accidentally give any old index into the tables and potentially read some random garbage from a table entry that has no value!

### Bindless resource access (Images and samplers)

Daxa has mirrored types for the host side ids for buffers, images and samplers to access resources in a bindless fashion. They are called `daxa_BufferId`, `daxa_ImageViewId` and `daxa_SamplerId` in shared files. The user can upload these ids via a push constant or buffer and then access them in the shader to get the GLSL side equivalent. 

Daxa provides the following functions for accessing:
```glsl
GLSL_IMAGE_TYPE       daxa_get_image(  GLSL_IMAGE_TYPE,       daxa_ImageViewId)
GLSL_TEXTURE_TYPE     daxa_get_texture(GLSL_TEXTURE_TYPE,     daxa_ImageViewId)
sampler               daxa_get_sampler(                       daxa_SamplerId)
```

If the user decides they want to declare their own table alias for any reason, Daxa also provides functions to get the table index from the id (the texture/sampled-image and image/storage-image indices into the tables are the same):

```glsl
daxa_u32 daxa_id_to_index(daxa_ImageId id);
daxa_u32 daxa_id_to_index(daxa_SamplerId id);
```

Now an example with the functions and ids described above:
```glsl
#include <daxa/daxa.inl>

layout(push_constant, scalar) uniform Push
{
    daxa_ImageViewId image_id;
    daxa_SamplerId sampler_id;
};

void main()
{
    vec2 uv = ...;
    vec4 value = texture(
        sampler2D(
            daxa_get_texture(texture2D, image_id), 
            daxa_get_sampler(sampler_id), 
        ), 
        uv
    );
}
```

This is already a bit nicer, but still has some problems. First one is that the user needs to construct a samplerXY object every time they want to access a texture. They also need to name the exact GLSL type every time the image is sampled. 

### Typed bindless image handles

To improve on the syntax, Daxa provides typed image handles. They follow this naming scheme:
```
(daxa_)?(RW)?Image(1D|2D|3D|1DArray|2DArray|2DMS|2DMSArray)(f32|i32|u32|i64|u64)
```

Fist comes the "daxa_" prefix, that can be disabled with `#define DAXA_ENABLE_NAMESPACE 0`.

Then there is an optional "RW", which denotes if the image handle is referring to a storage image or a sampled image. RW stands for read-write here.

Then the name Image is followed by the type of image (1D/2D/3D/1DArray/2DArray/2DMS/2DMSArray). These match the GLSL equivalent directly. So for example a daxa_Image2Df32 would correspond to a GLSL texture2D.

The last thing is the scalar type.

Daxa also provides optional GLSL function overloads for all these types. These can be enabled by defining the following names:
```
DAXA_ENABLE_IMAGE_OVERLOADS_BASIC 1
DAXA_ENABLE_IMAGE_OVERLOADS_ATOMIC 1
DAXA_ENABLE_IMAGE_OVERLOADS_64BIT 1
DAXA_ENABLE_IMAGE_OVERLOADS_MULTISAMPLE 1
```

Now an example with the typed image handles:

```glsl
#include <daxa/daxa.inl>
layout(push_constant, scalar) uniform Push
{
    daxa_Image2Df32 image;
    daxa_SamplerId sampler_id;
};

void main()
{
    vec2 uv = ...;
    vec4 value = texture(image, sampler_id, uv);
}
```

Much better! We now have syntax that reads as nice or even better then non-bindless glsl. With this, we can have all the benefits, bindless handles provide, without sacrificing readability!

This syntax is heavily influenced by the GLSL extension by nvidia called GL_NV_bindless_texture, which allows very similar bindless syntax in shaders.

An important caveat here is that daxa does NOT currently allow an rwimage (image in general layout) to be sampled via the glsl textureXXX functions when the image is in general layout. This is a temporary limitation that will be improved in the future.

The host code equivalent of these strongly typed image handles is a `daxa::types::ImageId` or `daxa::types::ImageViewId`.

At the end of this page there is a list with all overloaded glsl functions and some additional ones for daxa images.

### Bindless Buffers

Buffers face an even greater problem with bindless at first glance. Daxa can not define things in advance for buffers, as they are user defined and not a finite set like images. So the user must do all the accessing themselves. An example:

```glsl
#include <daxa/daxa.inl>

layout(set = 0, binding = DAXA_STORAGE_BUFFER_BINDING, scalar) readonly buffer BufferName
{
    int data;
} 
bufferTable[];

layout(push_constant, scalar) uniform Push
{
    daxa_BufferId buffer_id;
};

void main()
{
    int value = bufferTable[daxa_id_to_index(buffer_id)].data;
}
```

This is error prone due to no type safety and ugly due to having to index the table every time the buffer is used. Here is an example of the syntax with normal binding glsl to show the difference:

```glsl
#include <daxa/daxa.inl>

layout(set = ..., binding = ..., scalar) readonly buffer BufferName
{
    int data;
} 
b;

void main()
{
    int value = b.data;
}
```

As one can see, the non sugared bindless way looks way more cluttered.

### Typed Bindless Buffer Handles

Now untyped buffer access faces the same problems as untyped image access. It is less readable and more error prone.

Luckily GLSL provides an extension (GL_EXT_buffer_reference), which allows the user to use buffer pointers on the gpu but also greatly enhance the syntax! More on this extension here: 
* https://www.google.com/search?client=firefox-b-d&q=buffer+device+address+glsl
* https://community.arm.com/arm-community-blogs/b/graphics-gaming-and-vr-blog/posts/vulkan-buffer-device-address

With this extension, I was able to implement typed, pointer-like Buffer handles for Daxa.
Daxa has so called "BufferPtr"s. A BufferPtr points to a struct defined by the user. BufferPtr can also point to primitive type like u32. Buffer pointers allow for pointer arithmetic. 
To dereference a buffer ptr the user can either access the value via `ptr.value.some_field` or `deref(ptr).some_field`.
To enable the buffer ptr syntax for a struct the user must define the macro `DAXA_ENABLE_BUFFER_PTR(StructType)`.
An example:

These handles can be casted to and from 64 bit addresses, one can even do pointer arithmetic on the integers:
```glsl
#include <daxa/daxa.inl>
struct SomeData
{
    int data;
};
DAXA_ENABLE_BUFFER_PTR(SomeData)

layout(push_constant, scalar) uniform Push
{
    daxa_BufferPtr(SomeData) readonly_ptr;
    daxa_RWBufferPtr(SomeData) read_write_ptr;
};

void main()
{
    int value = deref(readonly_ptr).data;
    deref(read_write_ptr).data = value;

    SomeData some_data = deref(readonly_ptr);
    deref(read_write_ptr) = some_data;

    // If the pointer points to an array of SomeData, the user can do this:
    uint array_index = ...;

    int value = deref(readonly_ptr + array_index).data;
    deref(read_write_ptr + array_index).data = value;

    int value = deref(readonly_ptr[array_index]).data;
    deref(read_write_ptr[array_index]).data = value;
}
```

The host side equivalent of the strongly typed buffer is `daxa::types::BufferDeviceAddress`. To get and set a buffer pointer on the host one calls `daxa::Device::get_device_address(daxa::BufferId)`.

Daxa also has a convenience function to retrieve the address of a buffer via the BufferId inside the shader. A buffer address can be casted to and from a buffer pointer:
```glsl
daxa_u64 daxa_id_to_address(BufferId)
```

Example usage:
```glsl
#include <daxa/daxa.inl>

struct SomeData
{
    int data;
};
DAXA_ENABLE_BUFFER_PTR(SomeData)

layout(push_constant, scalar) uniform Push
{
    daxa_BufferId buffer_id;
};

void main()
{
    uint64_t address = daxa_id_to_address(buffer_id);
    daxa_BufferPtr(SomeData) ptr = daxa_BufferPtr(SomeData)(address);
    address = (u64)(ptr);
}
```

Daxa also predefines buffer pointer types for its primitive types like daxa_u32 or daxa_ImageViewId.

## Code Sharing Between GLSL and C++

Daxa provides headers for code sharing and the shader integration. In files that are shared one must include 
`<daxa/daxa.inl>`. The header with that can then be included in GLSL/HLSL and C++.

The shader headers are called `daxa.glsl` and `daxa.hlsl`.

All things defined in the shared and shader headers are prefixed with "daxa_". This prefix acts as a namespace for these files.

Daxa also gives the user the option to redefine certain names without the prefix by defining `DAXA_ENABLE_NAMESPACE 0`, or `DAXA_ENABLE_NAMESPACE_PRIMITIVES 0`.

Daxa declares primitive types that can be used in shared files. These will be defined as the language specific thing via macros. An example:

| shared       | GLSL | C++/daxa.hpp         |
| :----------- | :--- | :------------------- |
| daxa_u32     | uint | daxa::types::u32     |
| daxa_f32vec2 | vec2 | daxa::types::f32vec2 |


Daxa also provides counterparts for all the strongly typed handles and resource ids:
| shared/GLSL              | C++                              |
| ------------------------ | :------------------------------- |
| daxa_BufferPtr(TYPE)     | daxa::types::BufferDeviceAddress |
| daxa_RWBufferPtr(TYPE)   | daxa::types::BufferDeviceAddress |
| daxa_RWImage(DIM,SCALAR) | daxa::types::Image(View)?Id      |
| daxa_Image(DIM,SCALAR)   | daxa::types::Image(View)?Id      |
| daxa_BufferId            | daxa::types::BufferId            |
| daxa_ImageViewId         | daxa::types::Image(View)?Id      |
| daxa_SamplerId           | daxa::types::SamplerId           |

One can declare `struct`s like normal in shared files:

```
struct MyStruct
{
    daxa_u32 field;
};
```

These shared `struct`s can be used for push constants for example.

### Layouts And Bindings

The user must never declare a resource or a resource table like it would otherwise be usual with GLSL. Daxa declares all necessary GLSL constructs in daxa.glsl and convenient functions and macros to easily access any shader resource via id (or buffer device address/ buffer reference).
The user can however still declare their own table aliases for the descriptor set.
Daxa defines the following macros that can be used to declare bindings more easily.
It is highly recommended to use these macros if one decides to do their own bindings, as that allows the implementation of Daxa (for example the set of binding index) to change in the future without breaking ones code.

* `#define DAXA_STORAGE_IMAGE_BINDING 1`
* `#define DAXA_SAMPLED_IMAGE_BINDING 2`
* `#define DAXA_SAMPLER_BINDING 3`

* `#define DAXA_BUFFER_REFERENCE_LAYOUT layout(scalar, buffer_reference, buffer_reference_align = 4)`
* `#define DAXA_STORAGE_IMAGE_LAYOUT layout(binding = DAXA_STORAGE_IMAGE_BINDING, set = 0)`
* `#define DAXA_SAMPLED_IMAGE_LAYOUT layout(binding = DAXA_SAMPLED_IMAGE_BINDING, set = 0)`
* `#define DAXA_SAMPLER_LAYOUT layout(binding = DAXA_SAMPLER_BINDING, set = 0)`

## Practical defines

Daxa defines certain macros depending on the context the source code is compiled in:
* `DAXA_SHADER` defined when the source is compiled as a shader
* `DAXA_SHADERLANG` defined as either `DAXA_SHADERLANG_GLSL` or `DAXA_SHADERLANG_HLSL` if `DAXA_SHADER` is defined

## Common Errors
see [our page on common issues](https://github.com/Ipotrick/Daxa/wiki/Platform-support-and-potential-Problems#shaders)

## Daxa Image handle glsl function overloads
Shadow sampling functions do not map directly to glsl functions. These are the shadow sampling functions, and for what types they are overloaded:
* textureShadow(image, sampler_id, uv, compare) is overloaded for
  * 2Df32
  * Cubef32
  * 2dArrayf32
  * CubeArrayf32
* textureGradShadow(image, sampler_id, uv, compare, ddx, ddy) is overloaded for
  * 2Df32
  * Cubef32
  * 2dArrayf32
* textureLodShadow(image, sampler_id, uv, compare, lod) is overloaded for
  * 2Df32

Daxa also also provides gather functions, these are also renamed a bit from glsl:

* textureGatherX(image, sampler_id, uv)
* textureGatherY(image, sampler_id, uv)
* textureGatherZ(image, sampler_id, uv)
* textureGatherW(image, sampler_id, uv)

  are overloaded for:
  * Image2D(f32|i32|u32)
  * ImageCube(f32|i32|u32)
  * ImageCubeArray(f32|i32|u32)
  * Image2DArray(f32|i32|u32)

Daxa's glsl function overloads:

* texture(image, sampler_id, uv)
* textureLod(image, sampler_id, uv, bias)
* textureGrad(image, sampler_id, uv, dx, dy) 
* textureSize(image) 
* textureQueryLevels(image)

  are overloaded for 
  * Image1D(f32|i32|u32)
  * Image2D(f32|i32|u32)
  * Image3D(f32|i32|u32)
  * ImageCube(f32|i32|u32)
  * ImageCubeArray(f32|i32|u32)
  * Image1DArray(f32|i32|u32)
  * Image2DArray(f32|i32|u32)

* texelFetch(image, index, sample_or_lod) is overloaded for
  * Image1D(f32|i32|u32)
  * Image2D(f32|i32|u32)
  * Image3D(f32|i32|u32)
  * Image1DArray(f32|i32|u32)
  * Image2DArray(f32|i32|u32)

* imageLoad(rwimage, index)
* imageStore(rwimage, index, value)
* imageSize(image) 

  are overloaded for
  * RWImage1D(f32|i32|u32)
  * RWImage2D(f32|i32|u32)
  * RWImage3D(f32|i32|u32)
  * RWImageCube(f32|i32|u32)
  * RWImageCubeArray(f32|i32|u32)
  * RWImage1DArray(f32|i32|u32)
  * RWImage2DArray(f32|i32|u32)

If DAXA_ENABLE_IMAGE_OVERLOADS_ATOMIC is enabled, daxa also provides the following overloads:
* imageAtomicCompSwap(rwimage, index (, sampleIndex), compare, data)
* imageAtomicExchange(rwimage, index (, sampleIndex), compare, data)
* imageAtomicAdd(rwimage, index (, sampleIndex), compare, data)
* imageAtomicAnd(rwimage, index (, sampleIndex), compare, data)
* imageAtomicOr(rwimage, index (, sampleIndex), compare, data)
* imageAtomicXor(rwimage, index (, sampleIndex), compare, data)
* imageAtomicMin(rwimage, index (, sampleIndex), compare, data)
* imageAtomicMax(rwimage, index (, sampleIndex), compare, data)

  for the image types:
  * RWImage1D(i32,u32)
  * RWImage2D(i32,u32)
  * RWImage3D(i32,u32)
  * RWImageCube(i32,u32)
  * RWImageCubeArray(i32,u32)
  * RWImage1DArray(i32,u32)
  * RWImage2DArray(i32,u32)

If DAXA_ENABLE_IMAGE_OVERLOADS_MULTISAMPLE is enabled, daxa provides the basic rwimage functions listed above for 2DMS 2DMsArray rwimages as well.

If DAXA_ENABLE_IMAGE_OVERLOADS_64BIT is enabled, daxa provides the above listed functions for i64 and u64 rwimages as well.