<p align="center">
  <!-- <a href="https://github.com/Ipotrick/Daxa"> -->
	<img src="misc/daxa-logo.png" width="800" alt="Daxa logo">
  <!-- </a> -->
</p>

# Why Daxa?
✅ Strong modern GPU focus - no legacy hardware compromises  
🚀 Bindless by default – no descriptor management nor bindings  
🛠️ Shader Build system, shader integration and host/shader code sharing utils  
🧩 Convenient, efficient TaskGraph rendering framework

Daxa is a Vulkan-powered GPU abstraction purpose-built for modern graphics cards (>Turing, >RDNA, >Arc). Its streamlined design is *directly informed by the capabilities and assumptions of modern architectures*—delivering simplicity, deep shader integration, and predictable performance without legacy compromises.

Other than the focus on modern architecture only, what makes Daxa unique?

### Full descriptor/binding abstraction. Full Bindless in shaders and host side.
  * buffers, images, samplers, and acceleration structures are all exclusively accessed via IDs or pointers
  * very efficient on modern hardware
  * dramatically simplifies code as it completely removes one of the largest and headache inducing parts of modern GFX APIs

### Shader Utilities/ Shader Integration.
  * provides full shader build system with #include management, shader hot loading, SPIRV caching and more
  * provides daxa GLSL and SLANG headers allowing for bindless and other daxa feature access in shaders
  * provides daxa.inl file that allows you to write code shareable between host and shader via macros

### TaskGraph acting as full-on rendering framework for your project.
  * automatic synchronization between tasks,  multiple separate TaskGraphs and queues
  * efficient precompilation model: allows you to record graph once and execute it many times, significantly reducing CPU overhead
  * tons of utilities within task callbacks such as linear scratch buffer allocator and automatic image view generation
  * tons of validation checks with detailed error messages explaining the issue and potential solutions
  * automatic graph optimizations: reordering of tasks in order to minimize barriers, memory-aliased transient resources to reduce memory use and more

### Daxa also includes commonly supported Vulkan-middleware features:
* automatic deferring of resource destruction defer past GPU execution
* internal GPU memory allocator that allows for manual user allocations
* default parameters for object construction and simplified object creation
* swapchain synchronization and frame in flight handling
* threadsafe
* access to modern features such as raytracing, mesh shaders and multi queue
* smaller utilities such as a ringbuffer class for staging memory or Dear-ImGui integration

## Getting started

To begin using Daxa, visit [daxa.dev](https://daxa.dev/), where you'll find a comprehensive tutorial and a Wiki containing detailed information about the Daxa API.

- [Daxa Tutorial](https://tutorial.daxa.dev/)
- [Daxa Wiki](https://wiki.daxa.dev/)

Additionally, the Daxa repository includes a collection of example projects located in the **tests** folder

Working on something with Daxa? Whether it's a renderer, a tool, or just a small experiment—feel free to share it or ask questions on the [Discord server](https://discord.gg/MJPJvZ4FK5). It’s a good place to connect with others using Daxa and stay up to date with ongoing work.
