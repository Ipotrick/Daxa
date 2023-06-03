<p align="center">
  <!-- <a href="https://github.com/Ipotrick/Daxa"> -->
    <img src="misc/daxa-logo.png" width="800" alt="Daxa logo">
  <!-- </a> -->
</p>

# Daxa

Daxa is my personal opinionated gpu api abstraction over vulkan.

Daxa is close to vulkan with some key abstractions that make it much more convenient to use without losing flexibility or performance on modern desktop gpus. 

The original goal of daxa was to have a gpu library that was as easy or easier to use then OpenGL while retaining the good aspects of the vulkan api. 
I am happy to say that for me it achieves that goal now.

## Key features and abstractions
- [x] Simple setup and object creation, no more 1000 loc file to get a triangle to draw.
- [x] Automatic deferring of resource destruction to wait on possible use on the gpu.
- [x] Powerfull and flexible rendergraph implementation. (https://github.com/Ipotrick/Daxa/wiki/TaskList)
- [x] Bindless shader resource model. (https://github.com/Ipotrick/Daxa/wiki/Bindless-in-daxa)
- [x] C++ and shader code sharing utilities. (https://github.com/Ipotrick/Daxa/wiki/Shader-Integration)
- [x] Shader build system, including features like include management and shader hot reloading. (PipelineManager)

## Getting started

Daxa has a github wiki explaining how to build daxa https://github.com/Ipotrick/Daxa/wiki/Building.

The github wiki also contains pages on all bigger daxa features like TaskList and the shader integration.

In addition to that, the daxa repository has a set of example projects using daxa in the tests folder.

Daxa also has a tutorial wiki page https://github.com/Ipotrick/Daxa/wiki/Tutorial.

## Design Goals

Daxa is meant to be internally very simple and clean. This is important as i and the other maintainers (Gabe and Saky) want to be able to easily keep it clean bug free and maintainable. Having a smaller codebase makes all these things much easier. Because of this, daxa rutinely completely removes unused features and reworks parts to keep clean.

Backwards compatibility is a low priority. Reworks are frequent but are getting much less now. We release some stable versions from time to time.

Daxa imposes as minimal overhead in most parts of the api and tries to stick to vulkan.

Daxa is made in two layers. The core api and the utils. The utils contain things like the ImGui integration, PipelineManager and TaskList. This split is made to keep the responsibilities of the different parts of daxa strictly separated.

