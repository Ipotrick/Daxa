<p align="center">
  <!-- <a href="https://github.com/Ipotrick/Daxa"> -->
    <img src="misc/daxa-logo.png" width="800" alt="Daxa logo">
  <!-- </a> -->
</p>

# Daxa

Daxa is my personal opinionated GPU API abstraction over Vulkan.

Daxa's interface is close to but simpler then Vulkan. Some central abstractions make it much more convenient to use. 

Probably the two most intresting abstractions that set daxa appart from other abstractions is the fully bindless shader resource model and the c++/glsl shader integration.

The original goal of Daxa was to have a GPU library that was as easy (or easier) to use than OpenGL, while gaining access the great power of the Vulkan API for gpu driven work.
I am happy to say that for me (and others) it achieves that goal.

## Key features and abstractions
- [x] Simple setup and object creation. No more 1000 LOC file to get a triangle drawn (https://github.com/Ipotrick/Daxa/tree/master/wiki/Tutorial.md).
- [x] Much simplified api surface compared to vulkan. Daxa abstracts away vulkan concepts like VkDescriptorSet, VkDescriptorSetLayout, VkPipelineLayout and others completely. 
- [x] Automatic deferred resource destruction, automatic waiting on possible use from the GPU.
- [x] Powerful and flexible render-graph implementation (automatic synch and frame sheduling). (https://github.com/Ipotrick/Daxa/tree/master/wiki/TaskGraph.md)
- [x] Bindless shader resource model (much simpler and naturally gpu driven friendly descriptor management). (https://github.com/Ipotrick/Daxa/tree/master/wiki/Bindless.md)
- [x] C++ and shader code sharing utilities (no need for shader reflection, simply share source files). (https://github.com/Ipotrick/Daxa/tree/master/wiki/ShaderIntegration.md)
- [x] Shader build system, including features like `#include` management and shader hot-reloading. (https://github.com/Ipotrick/Daxa/tree/master/wiki/PipelineManager.md)

## Getting started

Daxa has a GitHub wiki explaining how to build it [here](https://github.com/Ipotrick/Daxa/wiki/Building).

The GitHub wiki also contains pages on all bigger Daxa features like TaskGraph and the shader integration.

In addition to that, the Daxa repository has a set of example projects using Daxa in the tests folder.

Daxa also has a [tutorial wiki page](https://github.com/Ipotrick/Daxa/tree/master/wiki/Tutorial.md).

If you prefer to interact in discord over github issues, daxa has a [discord server](
https://discord.gg/MJPJvZ4FK5).

## Design Philosophy

- While other vulkan abstactions aim to be quite generalized, daxa is (while still very general for a vulkan abstraction) aiming to be a desktop first abstraction.

- Daxa is made for gpu driven applications. This is due to me [Patrick](https://www.github.com/Ipotrick) and [Gabe](https://www.github.com/GabeRundlett) beeing very intrested in these types of gpu workloads. This is reflected in Daxa's shader integration, render graph api and bindless resource model, all of which make gpu driven much simpler and easier to employ over any other vulkan abstraction we tried.

- Daxa is meant to be internally very simple and clean. This is important as I and the other maintainers ([Gabe](https://www.github.com/GabeRundlett) and [Saky](https://github.com/MatejSakmary)) want to be able to easily keep it bug free and maintainable. Having a smaller, cleaner codebase makes for much more beautiful code in my opinion.

- Daxa's internals are very efficient and impose very low overhead. Its whole design is with cpu performance in mind. Daxa does very little tracking in the api itself and is also very restrictive on quickly accumulating costs like frequent hash map lookups.
Daxa imposes as minimal overhead in most parts of the API, compared to raw Vulkan it is practically always under 1% overhead compared to raw vulkan calls in the tested applications.

- Daxa is made in two layers. The core API and the utils. The utils contain things like the ImGui integration, PipelineManager and TaskGraph. This split is made to keep the responsibilities of the different parts of Daxa strictly separated.
