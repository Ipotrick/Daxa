# Daxa

<p align="center">
  <!-- <a href="https://github.com/Ipotrick/Daxa"> -->
    <img src="misc/daxa-logo.png" width="800" alt="Daxa logo">
  <!-- </a> -->
</p>

## What is Daxa?
Daxa is a GPU api built on Vulkan.

Daxa is not meant to be the "ultimate Vulkan can-do-anything abstraction"! It exposes the Vulkan features that I personally (and a few other folks) use. This also means that features that I didn't use or need yet are not exposed, glaring example of this would be that raster pipelines only support the vertex and fragment shader stages.

The general design goal is to make a minimal and simple api for modern desktop gpus. 
While beeing simple daxa also imposes a minimal cpu runtime overhead over vulkan.
In my opinion this makes an application build on daxa much nicer to maintain and use on the platforms i care about, compared to raw vulkan.

## Why is Daxa?

When I started to learn Vulkan, I realized that Vulkan was too verbose and explicit for my liking. Things like descriptor sets, render-passes, resource destruction, and the general lack of defaults annoyed me.
Using raw vulkan with little abstraction also led to hard to adapt and error prone code in my experience. 

Aside from these problems i really like vulkan! The  api design is much improved over older apis like DirectX11 and OpenGl. 

So i started to make daxa. Daxa is meant to be a simple api that combines the best attributes of other apis specifically for my likeing. I saw other people taking a liking in my code and the api, so i made it opensource for anyone to use and contribute!

## How is Daxa?

Daxa is actively worked on. The main branch is unstable from time to time but the releases should be stable versions.

## Who is Daxa?

Gabe Rundlett and I (Patrick Ahrens) are the main contributors to daxa. While I mainly work on the deep internals of the API, Gabe usually works on making sure everything builds and the user experience is good. There are also some other contributors like Matěj Sakmary, who greatly aid in development by bikeshedding on the right API and adding missing features.

## List of notable features:
* Easy initialization: Lots of defaults in the creation of any resource. Much less boilerplate code compared to raw Vulkan.
* A bindless resource model by default: Probably the feature that sets daxa apart from other abstractions the most is the shader resource binding model. In daxa all shader resources can be accessed via id or address (buffer device address). The user never needs to interact with any management, updating or binding of descriptor sets! They can simply pass the resource ID to the shader and use it. This allows for very simple GPU-driven programming and lowers the CPU runtime overhead of command recording dramatically in some cases. Detailed explanation: https://github.com/Ipotrick/Daxa/wiki/Bindless-in-daxa.
* Shader integration: daxa provides headers for code sharing between c++, glsl and hlsl (daxa.inl) wich allows the user to define structs for data transfer once for all languages! Daxa also provides headers for glsl (daxa.glsl) and hlsl (daxa.hlsl) for integration with other daxa features like bindless.
The bindless shader integration specifically is inspired by the GL_ARB_bindless_texture extension for opengl. Detailed explanation: https://github.com/Ipotrick/Daxa/wiki/Shader-Integration.
* Deferred resource destruction: Daxa will automatically detect when you free a resource that is still in use on the GPU and defer that destruction until it is safe to do so (This only applies to resources that are used in commands that have already been submitted to a queue. Attempting to destroy any resource that is used by a command in an unsubmitted yet recorded command list is illegal.
* Low overhead: Daxa is intended to impose minimal overhead over raw Vulkan. As a trade-off, usability and performance are favored over memory usage. "High-frequency" operations will generally avoid doing anything potentially costly, such as memory allocations or locking sync primitives in the background. 
* Thread safety: Daxa is designed with multithreading in mind from the beginning. All device calls are fully threadsafe and can be called on multiple threads. All other objects like the swapchain or command list should only be accessed by one thread at a time (this is deliberate to reduce overhead on high-frequency functions like draw calls). This thread safety (and the runtime overhead it generates) can be turned off by building Daxa with the #define DAXA_NO_THREADSAFETY. Generally, the needed external sync of Daxa objects maps directly to the needed external sync of their Vulkan counterparts.

* Utilities: Daxa as an api stays pretty minimal. Things like gpu sync is still manual. So Daxa provides a set of small helpher headers in the util folder. These build on the api and give example implementations that can be used directly or as reference for own implementations. These utils consist of:
  * PipelineManager: a shader build system that allows for runtime hotloading, filesystem interaction, full prerpocessor and more.
  * task_list: a rendergraph implementation that efficiently generates optimized synchronization for a renderer.
  * mem: simple staging-ring-buffer allocator implementation for upload and download to and from gpu.
  * imgui renderer

# Building
You must have the following things installed to build the repository
 * A C++ compiler
 * CMake (3.12 or higher)
 * Ninja build
 * vcpkg (plus the VCPKG_ROOT environment variable)
 * The Vulkan SDK

All dependencies are managed via vcpkg.
The library and the tests can be built using CMake.

Check out our [GitHub Wiki](https://github.com/Ipotrick/Daxa/wiki) for more detailed instructions!
