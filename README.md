# This repo is the home of the Daxa GPU API

## What is Daxa?
Daxa is a GPU api build on Vulkan.

Daxa is not meant to be the "ultimate Vulkan can-do-anything abstraction"! It exposes the Vulkan features that I personally (and a few other folks) use. This also means that features that I didn't use or need yet are not exposed, glaring example of this would be that raster pipelines only have a vertex and fragment shader.

## Why is Daxa?

When I started to learn Vulkan, I realized that Vulkan was too verbose for my liking. Things like descriptor sets, render-passes, resource destruction, and the general lack of defaults annoyed me.

So I started to make Daxa, combatting the problems I have with raw Vulkan while maintaining the low CPU overhead and access to modern features that Vulkan provides. It is a research project for me personally to learn more about API, library design, and low-level API usage. I think I have made a very nice API thus far so I'll keep it public to share with others to enjoy.

## How is Daxa?

Daxa is generally stable and there are tests and samples which should all build, but there is no guarantee for stability or future maintenance from me. It is under steady development and I am still very motivated to work on it. I am happy to accept pull requests for additional features, as long as they fit with the general design of Daxa.

## Who is Daxa?

Gabe Rundlett and I are the main contributors to Daxa. While I mainly work on the deep internals of the API, Gabe usually works on making sure everything builds and the user experience is good. There are also some other contributors like Matěj Sakmary, who greatly aid in development by bikeshedding on the right API and adding missing features.

## List of notable features:
* Easy initialization: Lots of defaults in the creation of any resource. Much less boilerplate code than raw Vulkan.
* Low overhead: Daxa is intended to impose minimal overhead over raw Vulkan. As a trade-off, usability and performance are favored over memory usage. "High-frequency" operations will generally avoid doing anything potentially costly, such as memory allocations or locking sync primitives in the background. Anything more frequent than a pipeline barrier and/or split barrier is considered a high-frequency operation. For example, most command list operations are considered potential high-frequency operations, so they will have very low overhead. One of the consequences of this is that sync in Daxa is fully explicit, so the user has to handle it (more about that later).
* Thread safety: Daxa is designed with multithreading in mind from the beginning. All device calls are fully threadsafe and can be called on multiple threads. All other objects like the swapchain or command list should only be accessed by one thread at a time (this is deliberate to reduce overhead on high-frequency functions like draw calls). This thread safety (and the runtime overhead it generates) can be turned off by building Daxa with the #define DAXA_NO_THREADSAFETY. Generally, the needed external sync of Daxa objects maps directly to the needed external sync of their Vulkan counterparts.
* A bindless resource model by default: The user never needs to interact with any binding, they can simply pass the resource ID to the shader and use it. No more annoying descriptor set management! This allows for very simple GPU-driven resource management and lowers the CPU runtime overhead of command recording dramatically in some cases.
* Deferred resource destruction: Daxa will automatically detect when you free a resource that is still in use on the GPU and defer that destruction until it is safe to do so (This only applies to resources that are used in commands that have already been submitted to a queue. Attempting to destroy any resource that is used by a command in an unsubmitted yet recorded command list is illegal.
* A Shader build system: Shaders in Daxa are written in GLSL, HLSL, or directly provided in SPIRV. Daxa provides functionality like a preprocessor for shaders and also convenience tools like hot reloading for pipelines. Daxa also provides shader headers (daxa.glsl, daxa.hlsl) for seamless bindless resource integration and some utilities to allow shared code between c++, GLSL, and HLSL more easily.
* A "TaskList" (render-graph): Probably one of the biggest headaches in Vulkan is synchronization and as Daxa has explicit sync, this is still a major headache that remains unaddressed. Daxa provides a render graph called TaskList which automatically handles all synchronization for the user.
* Utilities: Daxa as an API does not have native support for things like ImGUI or auto sync. Daxa contains some helpers for things like FSR, ImGUI-backend, a TaskList render-graph, and other things under the utils folder. These utils are optional, but in some cases greatly enhance Daxa's usability (TaskList) and its setup speed (eg ImGui, FSR).

For people that are not familiar with render graphs: A task list is like a higher-level power list with automatic synchronization. Tasks are recorded similarly to commands in a command list. The recorded Tasks are transformed into a graph, analyzed for resource execution and memory dependencies, and needed memory barriers and resource transitions are inserted automatically. TaskList also reorders tasks (while preserving effective recording execution order) to reduce the needed barriers, resource state transitions, and pipeline stalls to a minimum.

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
