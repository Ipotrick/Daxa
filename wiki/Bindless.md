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

### GLSL Annocations For Images

In glsl, it is possible to annotate image variables with custom [qualifiers](https://www.khronos.org/opengl/wiki/Type_Qualifier_(GLSL)). Such annotations can be for example: coherent or readonly.
These are really useful and can provide better performance and more possibilities in some cases. 

To provide the image accessor makros, daxa predefines image tables without any annotations. These make the access makros such as `daxa_image2D` possible to use.

Predefining all possible permutations of qualifiers for all image types would be thousands of loc, destroying compile time perf. Because of this daxa only predeclares the tables used in the makros without qualifiers.

To still give a nice way to gain access to daxa image views with the benefits of the annotations, daxa goes the middleground and allows the declaration of new accessors for images with annotations.
These custom accessors declare a new table with these annotations. 

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

## Buffer Shader Access

Daxa only allows buffers to be accessed either as fixed bind-point bound uniform buffers or via buffer device address.

Binding a buffer to a uniform buffer slot:
```c++
command_list.set_uniform_buffer({.slot = 0, .buffer=buffer_id});
```

Uniform buffers are globally declared in shaders with a specific binding slot with the macro `DAXA_DECL_UNIFORM_BUFFER(SLOT)`:
```glsl
DAXA_DECL_UNIFORM_BUFFER(0) UniformBufferBlock
{
    uint field;
};
```

Very importantly, I note here that these bindings are updated and get visible on the GPU **ONLY** when a new pipeline is bound. So you can NOT change them between draw calls or similar. This is intentional as binding is slow and against Daxa's bindless philosophy. Yet some hardware really benefits from direct uniform buffer bindings like NVIDIA. In Daxa, uniform buffers are meant to be used **ONLY** for larger uniformly accessed data across all invocations in the shader. For any changes between dispatches and draws, use push constants!

## Buffer References In Shaders

Daxa required Buffer References to be declared in a very specific way. You must use the Daxa provided macro (`DAXA_DECL_BUFFER_REFERENCE(ALIGNMENT)`) to set the layout of the reference:

```glsl
DAXA_DECL_BUFFER_REFERENCE(4) BufferReferenceBlock
{
    uint field;
};
```

Alternatively, Daxa provides a macro (`DAXA_DECL_BUFFER_PTR_ALIGN(STRUCT, ALIGNMENT)` and `DAXA_DECL_BUFFER_PTR(STRUCT)`) to declare a set of buffer references from a struct. This is very helpful for shader integration and code sharing, as you can declare the shared struct and then simply apply the macro to generate a set of buffer references to that struct automatically. Used in a shared file, a `daxa_BufferPtr(STRUCT)` macro will be translated to `daxa::types::BufferDeviceAddress` in C++ code, making code sharing easy.

Example:
```glsl
struct MyStruct
{
    daxa_u32 field;
};
DAXA_DECL_BUFFER_PTR(MyStruct)
```

These macros generate buffer references under the following names:
* `daxa_BufferPtr(MyStruct)`
* `daxa_BufferPtrMyStruct`
* `daxa_RWBufferPtr(MyStruct)`
* `daxa_RWBufferPtrMyStruct`

> Note there are no coherent buffer references in daxa. This is because daxa uses the vulkan memory model (vmm). The vmm deprecates buffer and image coherent annotations and replaces them with fine atomic and memory barrier calls, see [the glsl extension](https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_memory_scope_semantics.txt).

The Daxa buffer ptr types are simply a buffer reference containing one field named `value` of the given struct type.
Daxa also provides the `deref(BUFFER_PTR)` macro, which simply translates to `BUFFER_PTR.value`. This is some optional syntax sugar for those who prefer to be clear and want to avoid using the name `value` here.

```c
struct MyStruct
{
    daxa_u32 field;
};
DAXA_DECL_BUFFER_PTR(MyStruct)

...

daxa_BufferPtr(MyStruct) ptr = ...;
MyStruct strct = ptr.value;
// alternatively:
MyStruct strct = deref(ptr);
// As this is just a buffer reference, you can also just access the fields directly:
uint i = ptr.value.field;
// alternatively:
uint i = deref(ptr).field;
```

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
