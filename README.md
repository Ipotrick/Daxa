# This repo is the home of the daxa GPU api

## What is daxa:
Daxa is a GPU api build on Vulkan.

When I started to learn Vulkan, I realized that Vulkan was too verbose for my liking. Things like descriptor sets, renderpasses, resource destruction and the general lack of defaults annoyed me.

So I started to make daxa with the intention to combat the problems I have with raw Vulkan by a lot, while maintaining the low overhead and access to modern features Vulkan provides. It is also a research project for me personally to learn more about API and library design.

Daxa is generally stable and tests for the included tests and my (and other Projects using daxa) but there is no guarantee for stability or future maintenance from me. But I usually read issues and accespt pull requests.

That said, daxa is not meant to be the "ultimate Vulkan can-do-anything abstraction"!
It esposes the Vulkan features that I personally (and a few other folks using daxa) use.
This also means that features that I didnt use or needed yet personally in the projects that use daxa are not exposed, glaring examples would be that raster pipelines only have a vertex and fragment shader.
The abstraction level is not too high, synchronization for example is still manual without the use of the task lists (a simple rendergraph).

## List of notable features:
* Easy initialization: Lots of defaults in creation of any resource. Much less boilderplate code than raw Vulkan.
* Low overhead: daxa is is intended to impose minimal overhead over raw Vulkan (for my usecases). As a trade-off, usability and performance are favoured over memory usage.
* Thread safety: it is designed with multithreading in mind from the beginning. All device calls are fully threadsafe and can be called on multiple threads. All other objects like the swapchain or command list should only be accessed by on thread at a time (this is deliberate to reduce overhead on high frequency functions like drawcalls). This threadsafety (and the runtime overhead it generates) can be turned off by building daxa with the define DAXA_NO_THREADSAFETY.
* Bindless resource model by default: daxa binds all resources to one mega descriptor set.
  The user never needs to interact with any binding, they can simply pass the resource id to the shader and use it. No more annoying descriptor set management!
* Deferred resource destruction: daxa will automatically detect when you free a resource that is still in use on the gpu and defer that destruction until it is safe to do so.
* Shader build system: Shaders in daxa are written in HLSL or directly provided in SPIRV. Daxa provides functionality like a preprocessor for shaders and also convenience tools like hot reloading for pipelines.
* Task graph(TODO): daxa also provides a simple task graph that automaticaly generates pipeline barriers for the user. This taskgraph is relatively simple: it does not reoder commands, but it does allow aliasing transient resources and batching pipeline barriers.

# Building
You must have the following things installed to build the repository
 * A C++ compiler
 * CMake (3.12 or higher)
 * Ninja build
 * vcpkg (plus the VCPKG_ROOT environment variable)
 * The Vulkan SDK

All dependencies are managed via vcpkg.
The library and the tests can be built using CMake.
