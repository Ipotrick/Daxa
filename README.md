<p align="center">
  <!-- <a href="https://github.com/Ipotrick/Daxa"> -->
	<img src="misc/daxa-logo.png" width="800" alt="Daxa logo">
  <!-- </a> -->
</p>

# Daxa

Daxa is my opinionated GPU API abstraction over Vulkan.

Daxa's interface is quite close to Vulkan, but much simpler. Some central abstractions make Daxa much more convenient to use.

The feature that sets Daxa apart from other abstractions is its fully bindless shader resources. At this point in time, in my opinion, it is clear that bindless is the future and binding-based models will only be used to support low-end mobile GPUs going forward. Bindless is simpler, easier, more powerful, and sometimes much more performant than conventional approaches like the "normal" descriptor set workflow in Vulkan or other abstractions like WebGPU.

Daxa’s goal is to be as convenient to use as OpenGL while being very modern. Daxa gives access to the latest features and trends, while not being held down by years of tech debt.
I am happy to say that for me (and others) it achieves that goal.

## Biggest features and abstractions:
- [x] Greatly simplified API surface compared to Vulkan. Daxa is only surfacing those concepts that are crucial for good control and performance. [No more 1k LOC files to get a triangle](https://github.com/Ipotrick/Daxa/tree/master/wiki/Tutorial.md)
- [x] Bindless shader resource model. Resources are exclusively accessed via an ID (or address) instead of descriptor set bindings in shaders. This drastically simplifies the API while also being a very powerful tool for GPU-driven rendering. (https://github.com/Ipotrick/Daxa/tree/master/wiki/Bindless.md)
- [x] Powerful and flexible render graph. While manual sync is exposed in Daxa for best control, it also provides an optional render graph. (https://github.com/Ipotrick/Daxa/tree/master/wiki/TaskGraph.md)
- [x] C++ and shader code sharing utilities. This, combined with other simplifications, means you will never need shader reflection for convenience. (https://github.com/Ipotrick/Daxa/tree/master/wiki/ShaderIntegration.md)
- [x] Shader build system, including features like `#include` management and shader hot-reloading. (https://github.com/Ipotrick/Daxa/tree/master/wiki/PipelineManager.md)

List of other noteworthy features: [Other Features](https://github.com/Ipotrick/Daxa/tree/master/wiki/Features.md)

## Getting started

Daxa has a wiki explaining how to get set up compiling [here](https://github.com/Ipotrick/Daxa/tree/master/wiki/Building.md).

The GitHub wiki also contains pages on all bigger Daxa features, like TaskGraph and the shader integration.

Daxa also has a [tutorial page](https://github.com/Ipotrick/Daxa/tree/master/wiki/Tutorial.md) detailing the basics of the API.

In addition to that, the Daxa repository has a set of example projects using Daxa in the tests folder.

If you prefer to interact in Discord over GitHub issues, Daxa has a [Discord server](https://discord.gg/MJPJvZ4FK5).

For more detailed information on the abstractions and features of Daxa, take a look at the [wiki](https://github.com/Ipotrick/Daxa/tree/master/wiki).

## Design Philosophy

- Daxa should be convenient and easy to use. Iteration times are king. The programmer should be empowered by convenient and modern feature, not hindered by boilerplate and legacy. It is important to make Daxa simple and easy while maintaining exceptional performance at the same time.

- While other Vulkan abstractions aim to be overly generalized and backward compatible, Daxa aims to be focused on modern GPUs and features. This focus allows Daxa to make tradeoffs better suited for modern GPUs and techniques while staying simple.

- Daxa is made for GPU-driven applications. This is due to me [Patrick](https://www.github.com/Ipotrick) and [Gabe](https://www.github.com/GabeRundlett) being very interested in these types of GPU workloads. It is also the modern general approach to most problems regarding GPU programming. Other abstractions simply did not satisfy our needs in this area.

- Daxa is meant to be internally very simple and should not have multiple ways of doing the same thing. If a feature is used very little, it will be axed. Having less code means fewer bugs while also making it easier to understand and maintain.

- Daxa's internals are meant to be efficient and impose very low overhead. Its design is with CPU performance in mind. Daxa does very little tracking in the API itself and is also very restrictive on quickly accumulating costs like frequent hash map lookups or spamming reference count in/decrements.
Daxa currently imposes a minimal overhead in most parts of the API, compared to raw Vulkan it is practically always under 1% overhead compared to raw Vulkan calls in the tested applications.

- Daxa is made in two layers. The core API and the utils. The utils contain things like the ImGui integration, PipelineManager, and TaskGraph. This split is made to keep the responsibilities of the different parts of Daxa strict
