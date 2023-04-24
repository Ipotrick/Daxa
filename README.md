# Daxa

<p align="center">
  <!-- <a href="https://github.com/Ipotrick/Daxa"> -->
    <img src="misc/daxa-logo.png" width="800" alt="Daxa logo">
  <!-- </a> -->
</p>

## What is Daxa?
Daxa is a GPU api built on Vulkan.

Daxa is a lot simpler and much easier to use then vulkan, but retains the great cpu performance increase of vulkan by not introducing large overhead. It also enforces certain new concepts that are rightfully increasing in popularity like bindless resources or statically generated synchroinization with rendergraphs.

Daxa is not meant to be the "ultimate Vulkan can-do-anything abstraction"! It exposes the Vulkan features that I personally (and a few other folks) use. Daxa and its internals are meant to stay reasonably simple to keep it managable to maintain. But anyone is invited to contribute to daxa and add features they may like after consideration by the current maintainers.

This library i also a way for me to learn about library and interface design in general. This means there are a lot of experimental features that might get axed in the future. Another consequence of that is that the api is really unstable. I regularly break backwards compatibility in many ways. This tends to get less rescently as the api converges to a point where i cant come up with improvements. But be warned that there will come breaking changes.

If you are intrested in daxa check out the wiki page in this repository!

## Why is Daxa?

When I started to learn Vulkan, I realized that Vulkan was too verbose and explicit for my liking. Things like descriptor sets, render-passes, resource destruction, and the general lack of defaults annoyed me. 
Using raw vulkan with little abstraction also led to hard to adapt and error prone code in my experience. 
But other libraries and apis like OpenGl also did not satisfy me. Many of the are quite old and were designed in a time with difference concerns, making them hold onto a lot of legacy cruft.

Aside from these problems i really like vulkan! The api design is much improved over older apis like DirectX11 and OpenGl when building an abstraction on top.

So i started to make daxa. Daxa is meant to be a simple api that combines the best attributes of other apis specifically for my likeing. I saw other people taking a liking in my code and the api, so i made it opensource for anyone to use and contribute!

## How is Daxa?

Daxa is actively worked on. The main branch is unstable from time to time but the releases should be stable versions.

## Who is Daxa?

Gabe Rundlett and I (Patrick Ahrens) are the main contributors to daxa. While I mainly work on the deep internals of the API, Gabe usually works on making sure everything builds and the user experience is good. There are also some other contributors like Matěj Sakmary, who greatly aid in development by bikeshedding on the right API and adding missing features.

## List of notable features:
* Low boilerplate: Defaults in the creation of any resource. Many things are simplified a lot by using modern vulkan extensions and features like dynamic rendering. Overall less code needed to achieve the same things compared to raw Vulkan. For example rendering a triangle, including creating window and application code only takes around 200-300 loc in daxa.
* A bindless resource model by default: Probably the feature that sets daxa apart from other abstractions the most is the shader resource binding model. In daxa all shader resources can be accessed via id or address (buffer device address) in shaders at all times. The user never needs to interact with any vulakan descriptor management. No more allocating, writing or binding of descriptor sets! You can simply pass the resource ID to the shader and use it. You can even store these handles in buffers, pass and store them any way you like. This allows for very simple GPU-driven programming and lowers the CPU runtime overhead of command recording dramatically in some cases. Detailed explanation: https://github.com/Ipotrick/Daxa/wiki/Bindless-in-daxa.
* Shader integration: daxa provides headers for code sharing between c++, glsl and hlsl (daxa.inl) wich allows the user to define structs for data transfer once for all languages! Daxa also provides headers for glsl (daxa.glsl) and hlsl (daxa.hlsl) for integration with other daxa features like the bindless resource handles.
The bindless shader integration specifically is inspired by the GL_ARB_bindless_texture extension for opengl. Detailed explanation: https://github.com/Ipotrick/Daxa/wiki/Shader-Integration.
Daxa even provides shader integration makros for its own rendergraph implementation "TaskList".
* Deferred resource destruction: Daxa will automatically detect when you free a resource that is still in use on the GPU and defer that destruction until it is safe to do so (This only applies to resources that are used in commands that have already been submitted to a queue. Attempting to destroy any resource that is used by a command in an unsubmitted yet recorded command list is illegal.
* Low overhead: Daxa is intended to impose minimal overhead over raw Vulkan. As a trade-off, usability and performance are favored over memory usage. "High-frequency" operations will generally avoid doing anything potentially costly, such as memory allocations or locking sync primitives in the background. 
In fact daxa will even force the user to do to do perform more efficient vulkan api interactions with its design decisions. For example making resource handless bindless removes most performance pitfalls of descriptor sets in vulkan. 
* Thread safety: Daxa is designed with multithreading in mind from the beginning. All device calls are fully threadsafe and can be called on multiple threads. All other objects like the swapchain or command list should only be accessed by one thread at a time (this is deliberate to reduce overhead on high-frequency functions like draw calls). This thread safety (and the runtime overhead it generates) can be turned off by building Daxa with the #define DAXA_NO_THREADSAFETY. Generally, the needed external sync of Daxa objects maps directly to the needed external sync of their Vulkan counterparts.
* TaskList (rendergraph): Daxa provides a utility rendergraph implementation. This framework allows you to describe a high level description of your renderer and then compile it into an execution plan. The rendergraph will optimize and generate synch for your recorded tasks. After beeing compiles the lists can be reused. This can greatly reduce the nessecary work to insert correct synchronization for the user and the runtime.
* PipelineManager: A shader build system that allows for runtime hotloading, filesystem interaction, full prerpocessor and more.
* ImGui renderer: a basic imgui rendering backend.

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
