<p align="center">
  <!-- <a href="https://github.com/Ipotrick/Daxa"> -->
	<img src="misc/daxa-logo.png" width="800" alt="Daxa logo">
  <!-- </a> -->
</p>

# Daxa

Daxa is my opinionated GPU API abstraction over Vulkan.

Why would you want to use Daxa? These are some of the aspects of Daxa that set it apart from most other Vulkan abstractions:

- Specifically designed for GPGPU programming and GPU driven rendering. Daxa makes writing modern renderers simple and easy. It does not compromise for old hardware or old API concepts. Modern GPUs are the target (for Nvidia this is >=Turing, for amd this is >=GCN3).

- Very high convenience with near zero boilerplate. Daxa makes the simple things as easy as possible. It abstracts over unnecessarily explicit parts of Vulkan. Tons of default values and common abstractions for renderers such as a render-graph and shader build system are provided by Daxa.

- Shader utilities and shader feature integration. Daxa provides custom GLSL and HLSL headers, host/shader code sharing utilities and even a shader build system featureing hot reloading and include management. Especially the shader integration for many features is very convenient.

- Efficient safety checks. An often neglected part of RHIs and rendering libraries is safety and debugging validation. Daxa performs many validation checks at very low overhead. Especially the render-graph has very detailed error messages detailing misuse.

- Effective abstractions with predictable performance. In contrast to many other abstractions, core Daxa never caches or lazily created pipelines, layouts or other objects. It should have performance as predictable as Vulkan. This makes profiling intuitive.

### Getting started

The GitHub [wiki](https://github.com/Ipotrick/Daxa/tree/master/wiki) contains pages on all bigger Daxa features, like TaskGraph (the render-graph), PipelineManager (shader build system) and shader integration.

Building and including Daxa in your project is explained [here](https://github.com/Ipotrick/Daxa/tree/master/wiki/Building.md).

Daxa also has a [tutorial page](https://github.com/Ipotrick/Daxa/tree/master/wiki/Tutorial.md) detailing the basics of the API.

Additionally, the Daxa repository has a set of example projects in the tests folder.

If you prefer to interact in Discord over GitHub issues, Daxa has a [Discord server](https://discord.gg/MJPJvZ4FK5).

### List of biggest features:
- defaulted, named parameters for most functions via c++20 designated struct initialization
- ergonomic, simplified object creation
- integrated debug utilities such as debug names and range labels
- feature rich render-graph, handling automatic synchronization and more
- shader integration for render-graph
- shader/host code shading utilities
- fully abstracted descriptor sets
- automated, deeply integrated bindless system
- shader bindless integration
- a shader build system featuring hot reloading and include handling
- designed for threading, each object has its threadsafety described
- no concept of render-pass or frame-buffer
- typesafe simplified command recording and submission
- automated memory allocation for buffers and images using VMA (manual allocation also possible)
- swapchain that handles frames in flight pacing and wsi problems like resizing
- always enabled dynamic state for any option that is cost free on target hardware
- simpler pipelines, no pipeline layout and no descriptor set layouts
- a full C API, may be used to create API wrappers for other languages
- extensive low overhead validation checks
- integrates most commonly used Vulkan extensions
- integration of modern API features, such as: `BufferDeviceAddress, MeshShaders, RayTracing, DescriptorIndexing, DynamicRendering`
- ergonomic explicit synchronization (optional, you can also use the render-graph instead)
- stores queriable metadata for all objects, similar to dx11
- simplified, semi-managed resource lifetimes
- resource destruction deferred until all currently submitted commands finish execution
- unified IDs in host and shader code for buffers, images etc.
- automatic default image view integrated into all images
- controllable parent-child lifetime semantics
- fully abstracts Vulkan, allows Daxa to have more control and be more convenient
- deadImGui backend
- transient memory pool utility object
