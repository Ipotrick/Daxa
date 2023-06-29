# Bindless Shader Resources

As gpus become more generalized and programmable, they deviated from the old OpenGl style binding slot model limiting the user to only a handfull of resources per shader. 

In modern APIs like vulkan/dx12/metal/CUDA it is common to have abstractions that can bind thousands of shader resources at once. Better yet in many of these apis its trivial to bind a massive table of resources like all your model textures at once and then dynamically index this table in the shader.

This way of accessing resources by an index, a plain pointer or other handle is commonly called "bindless" resource access. Bindless allows for much more flexible and powerfull shaders, freeing developers from old burdens.

In addition to the additional freedom and flexibility, bindless can make resource management much simpler as well. Anyone that has ever dealt with vulkan descriptor- sets, layouts and pools knows how braindraining it can be to try to write an efficient and easy to use abstraction in vulkan. Bindless makes all this much simpler.

For the latest gpus bindless resource access is very efficient and has practically no overhead. It is also where all apis are headed to and i confidently believe it is the future. This is why daxa forces the user to use bindless resource handles only.

## Images In Daxa

In daxa you can create images, image views and samplers from a device like so:

```c++
daxa::ImageId image_id = device.create_image({...});
daxa::ImageViewId image_view_id = device.create_image_view({...});
daxa::SamplerId sampler_id = device.create_image_sampler({...});
```

Daxa only supports seperate images and samplers, so no combined image samplers. This is for simplicity and also possible future dx12 compatibility (dx12 only supports sepatated images and samplers).

Intrestingly all images are created with a default image view, that can be retrieved like this:

```c++
daxa::ImageViewId image_view_id = image_id.default_view();
```

This is very handy, as most of the time one only really needs one view of the image. But daxa image views are a bit special compared to vulkan image views. In vulkan an image view is either a sampled image OR a storage image. Daxa always creates two image descriptors for each daxa::ImageViewId, one for storage image (in layout GENERAL) and one for sampled image (in layout SHADER_READ_ONLY_OPTIMAL), as long as the image type and format allows for each access.

### Image Shader Access

Now comes the magical part. All the created image, image view and sampler ids can be used in cpu code AND in gpu shader code! This means you can put a daxa_ImageViewId into a shared file struct, upload it and use it in the shader directly!

You never need to bind any resources, they are simply always available and accessible in shaders. 
> Keep in mind that you still need to make sure you have the correct image layout and sync for each resource!

The shader acceess works by transforming an `daxa_ImageViewId` with or without a `daxa_SamplerId` into a glsl `texture`, `image` or `sampler` locally. 

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

For each image access the image view id must be cast to the corresponding glsl type with daxas makros. Daxa defines makros for all normal image/texture/sampler types and even for some commonly used extensions like 64 bit images.

## Buffers In Daxa

Buffers are created similarly to images:
```c++
daxa::BufferId buffer_id = device.create_buffer({...});
```

Each buffer is created with a buffer device address and a mapped host pointer, as long as the memory requirements allow for it.

The host and device pointers can be retrieved:
```c++
void* host_ptr                              = device.get_buffer_host_address(buffer_id);
daxa::types::BufferDeviceAddress device_ptr = device.get_buffer_device_address(buffer_id);
```

## Buffer Shader Access

Daxa only allows buffers to be accessed either as fixed bindpoint bound uniform buffers or via buffer device address.

Binding a buffer to a uniform buffer slot:
```c++
command_list.set_uniform_buffer({.slot = 0, .buffer=buffer_id});
```

Uniform buffers are globally declared in shaders with a specific binding slot with the makro `DAXA_DECL_UNIFORM_BUFFER(SLOT)`:
```glsl
DAXA_DECL_UNIFORM_BUFFER(0) UniformBufferBlock
{
    uint field;
};
```

Very importantly i note here that these bindings are updated and get visible on the gpu ONLY when a new pipeline is bound. So you can NOT change them between drawcalls or similar. This is intentional as binding is slow and against daxas bindless philosophy. Yet some hardware really benefits from direct uniform buffer bindings like nvidia. In daxa uniform buffers are meant to be usedONLY for larger uniformly accessed data across all invocations in the shader. For any any changes between dispatches and draws use push constants!

## Buffer References In Shaders

Daxa required Buffer References to be declared in a very specific way. You must use the daxa provided makro (`DAXA_DECL_BUFFER_REFERENCE(ALIGNMENT)`) to set the layout of the reference:

```glsl
DAXA_DECL_BUFFER_REFERENCE(4) BufferReferenceBlock
{
    uint field;
};
```

Alternatively daxa provides a makro (`DAXA_DECL_BUFFER_PTR_ALIGN(STRUCT, ALIGNMENT)` and `DAXA_DECL_BUFFER_PTR(STRUCT)`) to declare a set of buffer references from a struct. This is very helpfull for shader integration and code sharing, as you can declare the shared struct and then simply apply the makro to generate a set of buffer references to that struct automatically. Used in a shared file, a `daxa_BufferPtr(STRUCT)` makro will be translated to `daxa::types::BufferDeviceAddress` in c++ code, making code sharing easy.

Example:
```glsl
struct MyStruct
{
    daxa_u32 field;
};
DAXA_DECL_BUFFER_PTR(MyStruct)
```

These makros generate buffer references under the following names:
* `daxa_BufferPtr(MyStruct)`
* `daxa_BufferPtrMyStruct`
* `daxa_RWBufferPtr(MyStruct)`
* `daxa_RWBufferPtrMyStruct`
* `daxa_CoherentRWBufferPtr(MyStruct)`
* `daxa_CoherentRWBufferPtrMyStruct`

The RW variants are decalred to be read and writable, the non-rw variants are READ ONLY. The coherent variant declare coherent memory access.

The daxa buffer ptr types are simply a buffer reference containing one field names `value` of the given struct type.
Daxa also provides the `deref(BUFFER_PTR)` makro, that simply translates to `BUFFER_PTR.value`. This is some optional syntax suggar for those who prefer to be clear and want to avoid using the name `value` here.
