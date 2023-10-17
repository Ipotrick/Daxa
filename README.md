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

> Daxa focuses on desktop gpus. Mobile (and to some extent intergrated) gpu concerns are usually neglected for a simpler interface with less compromise.

## Biggest features and abstractions:
- [x] Greatly simplified API surface compared to Vulkan. Daxa is only surfacing those concepts that are crucial for good control and performance. [No more 1k LOC files to get a triangle](https://github.com/Ipotrick/Daxa/tree/master/wiki/Tutorial.md)
- [x] Bindless shader resource model. Resources are exclusively accessed via an ID (or address) instead of descriptor set bindings in shaders. This drastically simplifies the API while also being a very powerful tool for GPU-driven rendering. (https://github.com/Ipotrick/Daxa/tree/master/wiki/Bindless.md)
- [x] Powerful and flexible render graph. While manual sync is exposed in Daxa for best control, it also provides an optional render graph. (https://github.com/Ipotrick/Daxa/tree/master/wiki/TaskGraph.md)
- [x] C++ and shader code sharing utilities. This, combined with other simplifications, means you will never need shader reflection for convenience. (https://github.com/Ipotrick/Daxa/tree/master/wiki/ShaderIntegration.md)
- [x] Shader build system, including features like `#include` management and shader hot-reloading. (https://github.com/Ipotrick/Daxa/tree/master/wiki/PipelineManager.md)
- [x] A full c api. The c++ api and all functionality is implemented in a c api. The c++ api fully abstracts over this c api. The c api also has full direct vulkan interop and binary compatibility with all daxa c++ objects. Be free to use the c api to create bindigns to other langauges!

List of other noteworthy features: [Other Features](https://github.com/Ipotrick/Daxa/tree/master/wiki/Features.md)

## Getting started

Daxa has a wiki explaining how to get set up compiling [here](https://github.com/Ipotrick/Daxa/tree/master/wiki/Building.md).

The GitHub wiki also contains pages on all bigger Daxa features, like TaskGraph and the shader integration.

Daxa also has a [tutorial page](https://github.com/Ipotrick/Daxa/tree/master/wiki/Tutorial.md) detailing the basics of the API.

In addition to that, the Daxa repository has a set of example projects using Daxa in the tests folder.

If you prefer to interact in Discord over GitHub issues, Daxa has a [Discord server](https://discord.gg/MJPJvZ4FK5).

For more detailed information on the abstractions and features of Daxa, take a look at the [wiki](https://github.com/Ipotrick/Daxa/tree/master/wiki).

## Design Philosophy

### No Boilerplate!
Daxa has defaults for every parameter, all simple things should be easy and straightforward to do. Anything small and simple should be super easy and quick to implement with daxa. Utilities should provide great convenience and "battieries included"-style experience. Setup should be very easy.

### Future looking, gpgpu inspired API
Daxa is meant to be used on modern desktop gpus. These gpus are getting increasingly generalized and support more and more convenient gpgpu features. Daxa is inspired by cudas ease of use and api in many aspects. Anything that does not present a significant performance advantage on these gpus is NOT exposed in daxa. Any new powerful concepts like bindless or gpu device pointers are the default in the api and not an afterthought.

### Features on demand
As the daxa team is quite small, we need to focus on what is implemented and what is not. If there is no demand for a feature of anyone, daxa will not have it. The internals of daxa are to be keept very tidy, so that outside contribution stays reasonable.

### Simple, readable and close to Vulkan.
Daxa should be as close as possible to vulkan naming wise if the concept fits. Structure objects and their use should be close to vulkan. This should make use of daxa much easier for anyone already knowing vulkan. Daxa will not overabstract or rename things, as that can cause confusion and lack of intent.