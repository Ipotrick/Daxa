# Bindess Shader Resource Object Access

## Why Bindless?

As GPUs become more generalized and programmable, they deviated from the old OpenGL style binding slot model which limited the user to only a handful of resources per shader.

In modern APIs like Vulkan/D3D12/Metal/CUDA, it is common to have abstractions that can bind thousands of shader resources at once. Better yet, in many of these APIs it is trivial to bind a massive table of resources - say all your model textures at once - and then dynamically index this table in the shader.

This way of accessing resources by an index, a plain pointer, or other handle is commonly called "bindless" resource access. Bindless allows for much more flexible and powerful shaders, freeing developers from old burdens.

In addition to the additional freedom and flexibility, bindless can make resource management much simpler as well. Anyone that has ever dealt with Vulkan descriptor sets, layouts, and pools knows how mentally taxing it can be to try to write an efficient and easy to use abstraction in Vulkan. Bindless makes all this much simpler.

For the latest GPUs bindless resource access is very efficient and has practically no overhead on modern GPUs. It is also where all APIs are headed to and I confidently believe it is the future. This is why Daxa forces the user to use bindless resource handles only.

In addition to the benefits in flexibility and convenience, bindless can have great CPU side performance benefits. Relatively speaking descriptor set updates and binds are quite expensive compared to draw and compute dispatch calls. With bindless it is possible to eliminate most descriptor set updates and reducing the binding call count to the number of pipeline binding calls. This reduction in set update and bind calls can greatly benefit CPU performance.

You do NOT need to interact with any binding logic in daxa. No descriptor sets, descriptor layouts, pipeline layouts binding, set numbers, pools, set allocation or shader reflection. Daxa handles all the descriptor management behind the scenes. The internals for this are actually very simple and efficient. There is no need for pessimizations like caching pipelines or layouts.

## Bindless Shader Integration

In oder to make bindless work seemlessly within shaders, daxa needs to provide GLSL and HLSL headers that define abstractions for shaders to conveniently access the SROs bindlessly.

Daxa provides a GLSL and HLSL header. These include shader types for the SRO ids: `daxa_BufferId`, `daxa_ImageViewId` and `daxa_SamplerId` in GLSL and `daxa::BufferId`, `daxa::ImageViewId` and `daxa::SamplerId` in HLSL. The headers also define functions to use these ids within shaders.

You do not need any language extensions or custom compilers for any of this shown here to work. It is all implemented in preprocessor makros within `daxa.glsl` and `daxa.hlsl`.

## Images

All images are created with internal default image views. In fact their ids can be trivically converted to an image view id:

```c++
daxa::ImageId image_id = device.create_image({...});
daxa::ImageViewId image_view_id = image_id.default_view();
```

This default view covers the full mip and layer dimensions of the image.
This can significaly reduce boilerplate, as for most images only the default view is nessecary for all uses.

> Daxa only supports separate images and samplers, so no combined image samplers. This simplifies the api and allows for more consistent HLSL support.

### GLSL

The shader access works by transforming a `daxa_ImageViewId` with or without a `daxa_SamplerId` into a GLSL `texture`, `image`, or `sampler` locally. 

Examples of transforming image and sampler ids into glsl objects locally:

```glsl
#include <daxa/daxa.glsl>
...
daxa_ImageViewId img = ...;
daxa_SamplerId smp = ...;
ivec4 v = texture(daxa_isampler3D(img,smp), vec3(...));

daxa_ImageViewId img2 = ...;
imageStore(daxa_image2D(img2), ivec2(...), vec4(...));

daxa_ImageViewId img3 = ...;
uvec2 size = textureSize(daxa_texture1DArray(img3));
...
```
> Note that they can not be treated as local variables and can only be used IN PLACE of the usage like shown below. You CAN however pass the ids as value types to functions structs and buffers!

> Daxa default enables many glsl extensions like [GL_EXT_samplerless_texture_functions](https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GL_EXT_samplerless_texture_functions.txt). It worth to check those out as they can be a bit unknown but extremely useful.

### GLSL Annotations For Images

In glsl, it is possible to annotate image variables with custom [qualifiers](https://www.khronos.org/opengl/wiki/Type_Qualifier_(GLSL)) and [image formats](https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)). Such annotations can be for example: coherent or readonly and r32ui or rgba16f.
Custom qualifiers are really useful and can provide better performance and more possibilities in some cases. Image formats on the other hand are sometimes required by some glsl functions (imageAtomicOr for example).

To provide the image accessor makros, daxa predefines image tables without any annotations. These make the access makros such as `daxa_image2D` possible to use.

Predefining all possible permutations of qualifiers for all image types would be thousands of loc, destroying compile time perf. Because of this daxa only predeclares the tables used in the makros without qualifiers.

To still give a nice way to gain access to daxa image views with the benefits of the annotations, daxa goes the middleground and allows the declaration of new accessors for images with annotations.
These custom accessors declare a new table with these annotations. Each user defined accessor has a unique name `ACCESSOR_NAME`. This name is used to identify the accessor when using it with `daxa_access(ACCESSOR_NAME, image_view_id)`.

```glsl
DAXA_DECL_IMAGE_ACCESSOR(TYPE, ANNOTATIONS, ACCESSOR_NAME) // Declares new accessor.
DAXA_DECL_IMAGE_ACCESSOR_WITH_FORMAT(TYPE, FORMAT, ANNOTATIONS, ACCESSOR_NAME) // Declares new accessor with format
daxa_access(ACCESSOR_NAME, image_view_id) // Uses the accessor by name to convert an image view id to the given glsl type.
```

Example:

```glsl
DAXA_DECL_IMAGE_ACCESSOR(image2D, coherent restrict, RWCoherRestr)
DAXA_DECL_IMAGE_ACCESSOR(iimage2DArray, writeonly restrict, WORestr)
DAXA_DECL_IMAGE_ACCESSOR_WITH_FORMAT(uimage2D, r32ui, , r32uiImage)
...
void main() {
    daxa_ImageViewId img0, img1, img2 = ...;
    vec4 v = imageLoad(daxa_access(3WCoherRestr, img0), ivec2(0,0));
    imageStore(daxa_access(WORestr, img1), ivec2(0,0), 0, ivec4(v));
    imageAtomicOr(daxa_access(r32uiImage, img2), ivec2(0,0), 1 << 31);
}
```

### HLSL

To get access to images in hlsl, you simply create a local hlsl texture object in the shader from the image id.

Constructing a texture handle in hlsl is done with makro constructors similarly to glsl. These constructors look like this: `daxa_##HLSL_TEXTURE_TYPE(TEX_RET_TYPE, IMAGE_VIEW_ID)`.

> Note: In contrast to glsl you can treat the returned hlsl texture handles as local variables.

> Note: Currently only 4 component return types for texture functions are implemented, this is done to reduce the header bloat. The generated code will be of the same quality.

Example:

```hlsl
#include <daxa/daxa.hlsl>
...
daxa::ImageViewId img = ...;
daxa::SamplerId smp = ...;
// Alternative one: using a makro to locally construct a texture handle. Used in place.
int4 v = daxa_Texture3D(int4, img).Sample(smp, float3(...));
int4 v = t.Sample(smp, float3(...));

daxa::ImageViewId img2 = ...;
daxa_RWTexture2D(float4, img2)[int2(...)] = float4(...);

daxa::ImageViewId img3 = ...;
// As you can treat them as local variables in hlsl, such things are also possible:
Texture1DArray<float4> t = daxa_Texture1DArray(float4, img3);
uint mips; uint width; uint elements; uint levels;
t.GetDimensions(mips, width, elements, levels);
...
```

## Buffers

Each buffer is created with a buffer device address and optionally a mapped host pointer, as long as the memory requirements allow for it.

The host and device pointers can be retrieved:
```c++
void* host_ptr                              = device.get_buffer_host_address(buffer_id);
daxa::types::BufferDeviceAddress device_ptr = device.get_buffer_device_address(buffer_id);
```

## Buffer References and Buffer Pointers GLSL

The general way to access buffers in daxa is via buffer device address and glsl's [buffer reference](https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GLSL_EXT_buffer_reference.txt).

> As daxa requires very specific glsl layout specifiers for its buffer references in order for other features to work properly, its very important to use Daxa's makros to declare new buffer references!

Daxa provides 4 ways to declare a new buffer reference: 
- `DAXA_DECL_BUFFER_REFERENCE_ALIGN(ALIGNMENT)`: declares head for new buffer reference block with given alignment
- `DAXA_DECL_BUFFER_REFERENCE`: declares head for new buffer reference block with default alignment (4)
- `DAXA_DECL_BUFFER_PTR_ALIGN(STRUCT, ALIGNMENT)`: decalres readonly and readwrite buffer pointers to given struct type with given alignment
- `DAXA_DECL_BUFFER_PTR(STRUCT)`: decalres readonly and readwrite buffer pointers to given struct type with default alignment (4)

Usage examples:

```glsl
DAXA_DECL_BUFFER_REFERENCE MyBufferReference
{
    uint field;
};

struct MyStruct { uint i; };
DAXA_DECL_BUFFER_PTR(MyStruct)

...
void main()
{
    // You can also get the address of a buffer id inside all shaders: daxa_u64 daxa_id_to_address(BUFFER_ID)
    daxa_u64 address = ...;
    MyBufferReference my_ref = MyBufferReference(address);
    my_ref.field = 1;
    daxa_BufferPtr(MyStruct) my_readonly_ptr = daxa_BufferPtr(MyStruct)(address);
    uint read_value = deref(my_readonly_ptr).i;
    daxa_RWBufferPtr(MyStruct) my_readwrite_ptr = daxa_RWBufferPtr(MyStruct)(address);
    deref(my_readwrite_ptr).i = 1;
}
```

In c++ the `daxa_BufferPtr(x)` and `daxa_RWBufferPtr` makros simply become `daxa::types::BufferDeviceAddress`, so you can put them into structs, push constants and or buffer blocks. `DAXA_DECL_BUFFER_PTR_ALIGN` and `DAXA_DECL_BUFFER_PTR` become a blank line in c++. This makes them usable in shared files. 

So generally it is recommended to declare structs in shared files and then declare buffer pointers to the structs. Using structs and buffer pointers reduces redundancy and is less error prone. The pointer like syntax with structs is also quite convenient in general, as you gain value semantics to the pointee with the `deref(ptr)` makro.

Sometimes it is nessecary to use glsl annotations/ qualifiers for fields within buffer blocks or to use glsl features that are not available in c++. For example the coherent annotation or unbound arrays are not valid in c++ or in glsl/c++ structs, meaning in order to use those features, one must use a buffer reference instead of a buffer pointer.

> The Daxa buffer ptr types are simply buffer references containing one field named `value` of the given struct type.
For the `BufferPtr` makro, the field is annotated with `readonly`, while it is not with `RWBufferPtr`.

# Buffer Access in HLSL

As Hlsl has poor BufferReference support, daxa relies on StructuredBuffer and ByteAddressBuffer for buffers in Hlsl.

These are constructed similarly to texture handles in hlsl with a construction makro: `daxa_ByteAddressBuffer(BUFFER_ID)`, `daxa_RWByteAddressBuffer(BUFFER_ID)`, `daxa_StructuredBuffer(STRUCT_TYPE, BUFFER_ID)`.

Note that in order to use StructuredBuffer for a given struct type you must use the `DAXA_DECL_BUFFER_PTR` makro for that struct. This is due to limitations of hlsl and backwards compatibility reasons with glsl.

Example:
```hlsl
#include <daxa.hlsl>
...
struct MyStruct
{
    uint field;
};
DAXA_DECL_BUFFER_PTR(MyStruct)

void main()
{
    daxa::BufferId buffer_id = ...;
    ByteAddressBuffer b = daxa_ByteAddressBuffer(buffer_id);
    b.Store(0, 1);
    uint read_value0 = b.Load<MyStruct>(0).field;
    StructuredBuffer<MyStruct> my_readonly_buffer = 
        daxa_StructuredBuffer(MyStruct, buffer_id);
    uint read_value1 = my_readonly_buffer[0].field;
}
```

> Note: Hlsl provides atomic ops for ByteAddressBuffer as well as a sizeof operator for structs.