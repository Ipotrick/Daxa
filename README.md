# This repo is the home of the daxa GPU api

## What is daxa?
Daxa is a GPU api build on Vulkan.

Daxa is not meant to be the "ultimate Vulkan can-do-anything abstraction"!
It exposes the Vulkan features that I personally (and a few other folks) use.
This also means that features that I didnt use or need yet are not exposed, glaring example of this would be that raster pipelines only have a vertex and fragment shader. 

## Why is daxa?

When I started to learn Vulkan, I realized that Vulkan was too verbose for my liking. Things like descriptor sets, renderpasses, resource destruction and the general lack of defaults annoyed me.

So I started to make daxa with the intention to combat the problems I have with raw Vulkan, while maintaining the low cpu overhead and access to modern features that Vulkan provides. It is a research project for me personally to learn more about API, library design and low level api usage. I think i have made a very nice api thus far so i have it public to share it with others to enjoy.

## How is daxa?

Daxa is generally stable and tests for the included tests and my (and other Projects using daxa) but there is no guarantee for stability or future maintenance from me. It is under steady development and i am still very moticated to work on it. I am happy to accept pull requests for additional features, as long as they fit with the general design of daxa.

## Who is daxa?

Gabe Rundlett and I are the main contributors to daxa. While i mainly work on the deep internals of the api, gabe usually works on making sure everything builds and the user experience is good. There are also some other contributors like Matěj Sakmary, wich greatly aid in development by bikeshedding on the right api and adding missing features.

## List of notable features:
* Easy initialization: Lots of defaults in creation of any resource. Much less boilderplate code than raw Vulkan.
* Low overhead: daxa is is intended to impose minimal overhead over raw Vulkan. As a trade-off, usability and performance are favoured over memory usage. "high-frequency" operations will generally avoid doing anything potentially costly like memory allocations or locking sync primitives in the background. Anything more frequent then a pipeline barrier and or split barrier is considered a high frequency operation. For example most command list operations are considered potential high frequency operations so they will have very low overhead.
One of the consequences of this is that sync in daxa is fully explicit, so the user has to handle it (more about that later).
* Thread safety: it is designed with multithreading in mind from the beginning. All device calls are fully threadsafe and can be called on multiple threads. All other objects like the swapchain or command list should only be accessed by on thread at a time (this is deliberate to reduce overhead on high frequency functions like drawcalls). This threadsafety (and the runtime overhead it generates) can be turned off by building daxa with the define DAXA_NO_THREADSAFETY. Generally the needed external sync of daxa objects maps directly to the needed external sync of their vulkan counterparts.
* Bindless resource model by default: The user never needs to interact with any binding, they can simply pass the resource id to the shader and use it. No more annoying descriptor set management! This allows for very simple gpu driven resource management and lowers cpu run time overhead of command recording dramatically in some cases.
* Deferred resource destruction: daxa will automatically detect when you free a resource that is still in use on the gpu and defer that destruction until it is safe to do so (This only applies to resources wich are used in commands that have allready been submited to a queue. Attempting to destror any resource that is used by a command in a recorded command list wich is not yet submitted is (like with vulkan command buffers) illegal and will lead to undefined behavior).
* Shader build system: Shaders in daxa are written in GLSL, HLSL or directly provided in SPIRV. Daxa provides functionality like a preprocessor for shaders and also convenience tools like hot reloading for pipelines. Daxa also provides shader headers (daxa.glsl, daxa.hlsl) for seamless bindless resource integration and some utils to share files between c++, glsl and hlsl more easily.
* TaskList (Rendergraph): Probably one of the biggest headaches in vulkan is synchronization and as daxa has explicit sync, this is still a major headache that remains unaddressed. Daxa provides a rendergraph called TaskList wich automatically handeles all synchronization for the user.
* Utils: Daxa as an api does not have native support for things like imgui or autosync. Daxa contains some helpers for things like FSR, ImGUI-backend, TaskList rendergraph and other things under the utils namespace. These utils are optional but in some cases greatly enhance daxas usability (TaskList) and its setup speed (eg ImGui, FSR).

For people that are not familiar with rendergraphs: 
A task list is like a higher level command list with automatic synchronization. Tasks are recorded similar to commands in a command list. The recorded Tasks is transformed into a graph, analyzed for resource execution and memory dependencies and needed memory barriers and resource transitions are inserted automatically. TaskList also reorderes tasks (while preserving effective recording execution order) to reduce the needed barriers, resource state transitions and pipeline stalls to a minimum.

# Building
You must have the following things installed to build the repository
 * A C++ compiler
 * CMake (3.12 or higher)
 * Ninja build
 * vcpkg (plus the VCPKG_ROOT environment variable)
 * The Vulkan SDK

All dependencies are managed via vcpkg.
The library and the tests can be built using CMake.

Check out our GitHub wiki for more detailed instructions!
