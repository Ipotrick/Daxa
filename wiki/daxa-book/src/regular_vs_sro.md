# Id vs Handle

There are two daxa object categories: shader resource objects (SRO) and regular objects.

Shader resource objects are daxas buffer, image, image view and sampler type. Any other object is a regular object.

These two object categories are treated differently as is explained here.
## Regular Object

Most daxa objects are regular objects.

Regular objects are represented by atomically reference counted handles. Because reference counting...
* prevents classes of errors
    * reduces the need for much internal validation
    * simplifies validation and internals
    * makes debugging less frequent and easier
* is convenient
    * removes the need to think about the end of lifetime
    * well known comfortable concept for most programmers

But there are also downsides to reference counting:
* copying handles has significant overhead. This can lead to unnessecary performance loss for objects that are high in number and or are used a lot in typical applications.
* lifetimes become less clear. It can lead to hogging memory and a general uncertainty on when objects get destroyed.

These downsides are NOT a problem for most objects. Specifically for objects that are used in large numbers, these downsides become much more problematic. Because of this I decided to split daxa objects into the two classes, based on their typical number and usage frequency.

An example of a regular object:

```c++
daxa::Device device = instance.create_device({.name = "example device"});
// All daxa objects store metadata that can be queried with an info function:
daxa::DeviceInfo const& device_info = device.info();    
```

## Shader Resource Object

What makes SROs different from regular objects?
* they are all accessable directly in shaders
* typically occur in much larger numbers
* typically accessed and used in greater frequency
* having clear lifetimes has bigger importance

As buffers, images and samplers typically come in much larger numbers then other objects, the previously mentioned downsides of ref counting become a problem. Buffers and images are also tied to typically large regions of memory which makes potential memory hogging much worse as well. Because of this, daxa does NOT reference count SROs! Instead daxa gives the user manual lifetime management over these objects with a create and destroy function.

SROs are represented by an id on the user side. When creating, using and destroying SROs, they are explusivey refered to by their ids. These ids are trivically copyable, they all have weak reference lifetime semantics. The ids work very similarly to how entity ids work in an ecs. There is no way to use the object without the id AND another daxa object, verifying its use. For example getting information of an object like the name MUST be done via a device function taking in the id. 

These ids are much safer then for example a raw pointer to the resource. I wont go into specifics here but these are some key advantages:
* user has no way to access object without device
* ability to efficiently verify object access
* threadsafety and validation even in the case of missuse
* close to zero overhead for validation

There is another big advantage of ids for SROs. As daxa uses descriptor indexing to access SROs in shaders, the ids can be used in cpu AND gpu shader code! This simplifies the api even more.

Examples of SROs:
```c++
daxa::BufferId buffer_id = device.create_buffer({
    .size = 64, 
    .name = "example buffer",
});

// Device also stores metadata about its SROs that can be queried:
daxa::BufferInfo const & buffer_info = device.info_buffer(buffer_id);

daxa::ImageId image_id = device.create_image({
    .format = daxa::Format::R8G8B8A8_SRGB,
    .size = {1024, 1024, 1},
    .usage = daxa::ImageUsageFlagBits::SHADER_SAMPLED | 
        daxa::ImageUsageFlagBits::TRANSFER_DST,
    .name "example texture image",
});

daxa::ImageViewId image_view_id = device.create_image_view({
    .format = Format::R8G8B8A8_SRGB;
    .image = image_id;
    .slice = {};
    .name = "example image view";
});

daxa::SamplerId sampler_id = device.create_image_sampler({});
```