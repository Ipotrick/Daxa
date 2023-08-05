# Bindless Shader Resources

## Why Bindless?

As GPUs become more generalized and programmable, they deviated from the old OpenGL style binding slot model which limited the user to only a handful of resources per shader.

In modern APIs like Vulkan/D3D12/Metal/CUDA, it is common to have abstractions that can bind thousands of shader resources at once. Better yet, in many of these APIs it is trivial to bind a massive table of resources - say all your model textures at once - and then dynamically index this table in the shader.

This way of accessing resources by an index, a plain pointer, or other handle is commonly called "bindless" resource access. Bindless allows for much more flexible and powerful shaders, freeing developers from old burdens.

In addition to the additional freedom and flexibility, bindless can make resource management much simpler as well. Anyone that has ever dealt with Vulkan descriptor sets, layouts, and pools knows how mentally taxing it can be to try to write an efficient and easy to use abstraction in Vulkan. Bindless makes all this much simpler.

For the latest GPUs bindless resource access is very efficient and has practically no overhead on modern GPUs. It is also where all APIs are headed to and I confidently believe it is the future. This is why Daxa forces the user to use bindless resource handles only.

In addition to the benefits in flexibility and convenience, bindless can have great CPU side performance benefits. Relatively speaking descriptor set updates and binds are quite expensive compared to draw and compute dispatch calls. With bindless it is possible to eliminate most descriptor set updates and reducing the binding call count to the number of pipeline binding calls. This reduction in set update and bind calls can greatly benefit CPU performance.

## Images In Daxa

In Daxa you can create images, image views, and samplers from a device like so:

```c++
daxa::ImageId image_id = device.create_image({...});
daxa::ImageViewId image_view_id = device.create_image_view({...});
daxa::SamplerId sampler_id = device.create_image_sampler({...});
```

Daxa only supports separate images and samplers, so no combined image samplers. This is for simplicity and also possible future D3D12 compatibility (D3D12 only supports separated images and samplers).

Notably, all images are created with a default image view, that can be retrieved like this:

```c++
daxa::ImageViewId image_view_id = image_id.default_view();
```

This is very handy, as most of the time one only really needs one view of the whole image. But Daxa image views are a bit special compared to Vulkan image views. In Vulkan, an image view is either a sampled image OR a storage image. Daxa always creates two image descriptors for each daxa::ImageViewId, one for storage image (in layout GENERAL) and one for sampled image (in layout SHADER_READ_ONLY_OPTIMAL), as long as the image type and format allow for each access.

### Image Shader Access

Now comes the magical part. All the created image, image view, and sampler IDs can be used in CPU code **AND** in GPU shader code! This means you can put a daxa_ImageViewId into a shared file struct, upload it, and use it in the shader directly!

You never need to bind any resources, they are simply always available and accessible in shaders.
> Keep in mind that you still need to make sure you have the correct image layout and sync for each resource!

The shader access works by transforming a `daxa_ImageViewId` with or without a `daxa_SamplerId` into a GLSL `texture`, `image`, or `sampler` locally.

Example:

```glsl
...
daxa_ImageViewId img = ...;
daxa_SamplerId smp = ...;
ivec4 v = texture(daxa_isampler3D(img,smp), vec3(...));
...
daxa_ImageViewId img = ...;
imageStore(daxa_image2D(img), ivec2(...), vec4(...));
...
daxa_ImageViewId img = ...;
uvec2 size = textureSize(daxa_texture1DArray(img));
...
```

> Daxa default enables the glsl extension [GL_EXT_samplerless_texture_functions](https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GL_EXT_samplerless_texture_functions.txt). This ext defines overloads for all texture access functions that do not require a sampler to only take in a textureDIMENSION instead.

For each image access, the image view ID must be cast to the corresponding GLSL type with Daxa's macros. Daxa defines macros for all normal image/texture/sampler types and even for some commonly used extensions like 64-bit images.

You do NOT need to interact with any binding logic. No descriptor sets, descriptor layouts, pipeline layouts binding, set numbers, or shader reflection. Daxa Ddes all the descriptor management behind the scenes.

When an image or image view is created, the image views are immediately added to the bindless table. When an image (-view) gets destroyed it is removed from the table. Note that resource destruction is deferred to the end of all currently running GPU work, so you do not need to write a zombie queue or similar in most cases.

### GLSL Annotations For Images

In glsl, it is possible to annotate image variables with custom [qualifiers](https://www.khronos.org/opengl/wiki/Type_Qualifier_(GLSL)). Such annotations can be for example: coherent or readonly.
These are really useful and can provide better performance and more possibilities in some cases. 

To provide the image accessor makros, daxa predefines image tables without any annotations. These make the access makros such as `daxa_image2D` possible to use.

Predefining all possible permutations of qualifiers for all image types would be thousands of loc, destroying compile time perf. Because of this daxa only predeclares the tables used in the makros without qualifiers.

To still give a nice way to gain access to daxa image views with the benefits of the annotations, daxa goes the middleground and allows the declaration of new accessors for images with annotations.
These custom accessors declare a new table with these annotations. Each user defined accessor has a unique name `ACCESSOR_NAME`. This name is used to identify the accessor when using it with `daxa_access(ACCESSOR_NAME, image_view_id)`.

```glsl
DAXA_DECL_IMAGE_ACCESSOR(TYPE, ANNOTATIONS, ACCESSOR_NAME) // Declares new accessor.
daxa_access(ACCESSOR_NAME, image_view_id) // Uses the accessor by name to convert an image view id to the given glsl type.
```

Example:

```glsl
DAXA_DECL_IMAGE_ACCESSOR(image2D, coherent restrict, RWCoherRestr)
DAXA_DECL_IMAGE_ACCESSOR(iimage2DArray, writeonly restrict, WORestr)
...
void main() {
    daxa_ImageViewId im0, img1 = ...;
    vec4 v = imageLoad(daxa_access(RWCoherRestr, img0), ivec2(0,0));
    imageStore(daxa_access(WORestr, img1), ivec2(0,0), 0, ivec4(v));
}
```

## Buffers In Daxa

Buffers are created similarly to images:
```c++
daxa::BufferId buffer_id = device.create_buffer({...});
```

Each buffer is created with a buffer device address and optionally a mapped host pointer, as long as the memory requirements allow for it.

The host and device pointers can be retrieved:
```c++
void* host_ptr                              = device.get_buffer_host_address(buffer_id);
daxa::types::BufferDeviceAddress device_ptr = device.get_buffer_device_address(buffer_id);
```

## Buffer References and Buffer Pointers

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
    daxa_u64 address = ...;
    MyBufferReference my_ref = MyBufferReference(address);
    my_ref.field = 1;
    daxa_BufferPtr(MyStruct) my_readonly_ptr = daxa_BufferPtr(MyStruct)(address);
    int read_value = deref(my_readonly_ptr).i;
    daxa_RWBufferPtr(MyStruct) my_readwrite_ptr = daxa_RWBufferPtr(MyStruct)(address);
    deref(my_readwrite_ptr).i = 1;
}
```

In c++ the `daxa_BufferPtr(x)` and `daxa_RWBufferPtr` makros simply become `daxa::types::BufferDeviceAddress`, so you can put them into structs, push constants and or buffer blocks. `DAXA_DECL_BUFFER_PTR_ALIGN` and `DAXA_DECL_BUFFER_PTR` become a blank line in c++. This makes them usable in shared files. 

So generally it is recommended to declare structs in shared files and then declare buffer pointers to the structs. Using structs and buffer pointers reduces redundancy and is less error prone. The pointer like syntax with structs is also quite convenient in general, as you gain value semantics to the pointee with the `deref(ptr)` makro.

Sometimes it is nessecary to use glsl annotations/ qualifiers for fields within buffer blocks or to use glsl features that are not available in c++. For example the coherent annotation or unbound arrays are not valid in c++ or in glsl/c++ structs, meaning in order to use those features, one must use a buffer reference instead of a buffer pointer.

> The Daxa buffer ptr types are simply buffer references containing one field named `value` of the given struct type.
For the `BufferPtr` makro, the field is annotated with `readonly`, while it is not with `RWBufferPtr`.

# Host-Device resource transport

As buffers, images, and samplers are not written to sets that get bounds how do you give the GPU the bindless ids and addresses to the shader in order to use it?

As described in the [shader integration wiki page](https://github.com/Ipotrick/Daxa/tree/master/wiki/ShaderIntegration.md), Daxa still has two other ways to set resources for shaders appart from the bindless access: uniform buffers and push constants. 
Push constants are an extreamly efficient and simple way to transport data to the gpu for any scenario, be it setting daxa for a compute dispatch or giving each drawcall new bindless handles to access, so naturally daxa provides push constants.
Uniform buffers on the other hand are not nessecary, yet they are exposed in daxa, as some hw like nvidia gpus have specialized hardware for dealing with these buffer bindings. For this reason daxa also provides uniform buffer bindings.

To transport ids and buffer device addresses, simply write them to a push constant or buffer and then read them in the shader. Daxa makes it very easy to [declare c++/shader-shared structs](https://github.com/Ipotrick/Daxa/tree/master/wiki/ShaderIntegration.md).
Normally you would declare a struct and push it in a push constant:
```cpp
struct PushData
{
  daxa_ImageViewId image;
};
```
You then write your image id to the push constant and read and access it like described above.
