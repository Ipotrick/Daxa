<p align="center">
  <!-- <a href="https://github.com/Ipotrick/Daxa"> -->
    <img src="misc/daxa-logo.png" width="800" alt="Daxa logo">
  <!-- </a> -->
</p>

# Daxa

Daxa is my opinionated GPU API abstraction over Vulkan.

Daxa's interface is quite close to Vulkan but much simplified. Some central abstractions make it much more convenient to use. 

The two most intresting features that set daxa appart from other abstractions the fully bindless shader ressources and its render graph.

The original goal of Daxa was to have a GPU library that was as easy (or easier) to use than OpenGL, while gaining access the great power of the Vulkan API for gpu driven work.
I am happy to say that for me (and others) it achieves that goal.

## Key features and abstractions
- [x] Simple setup and object creation. No more 1000 LOC file to get a triangle drawn. (https://github.com/Ipotrick/Daxa/tree/master/wiki/Tutorial.md)
- [x] Much simplified api surface compared to vulkan. Daxa abstracts away much of the vulkan api, only exposing those concepts that are crucial for good control and performance.
- [x] Automatic deferred resource destruction. Daxa deferres resource destruction until all work in flight at the time of the destruction call is finished.
- [x] Powerful and flexible render-graph. While manual sync is exposed in daxa for best control, it also provides an optional render-graph. (https://github.com/Ipotrick/Daxa/tree/master/wiki/TaskGraph.md)
- [x] Bindless shader resource model. Resources are accessed via an id (or address) in shaders instead of descriptor set bindings. This drastically simplifies the api while also beeing a very powerful tool for gpu driven rendering. (https://github.com/Ipotrick/Daxa/tree/master/wiki/Bindless.md)
- [x] C++ and shader code sharing utilities. This combined with other simplifications means you will never need shader reflection for convenience. (https://github.com/Ipotrick/Daxa/tree/master/wiki/ShaderIntegration.md)
- [x] Shader build system, including features like `#include` management and shader hot-reloading. (https://github.com/Ipotrick/Daxa/tree/master/wiki/PipelineManager.md)
      
List of other noteworthy features: [Other Features](https://github.com/Ipotrick/Daxa/tree/master/wiki/Features.md)

## Getting started

Daxa has a GitHub wiki explaining how to build Daxa [here](https://github.com/Ipotrick/Daxa/tree/master/wiki/Building).

The GitHub wiki also contains pages on all bigger Daxa features like TaskGraph and the shader integration.

In addition to that, the Daxa repository has a set of example projects using Daxa in the tests folder.

Daxa also has a [tutorial wiki page](https://github.com/Ipotrick/Daxa/tree/master/wiki/Tutorial.md).

If you prefer to interact in discord over github issues, daxa has a [discord server](
https://discord.gg/MJPJvZ4FK5).

## Design Philosophy

- Daxa should be convenient and easy to use. Iteration times are King, the peogrammer should be empowered and not hindered by boilderplate and legacy. It is important to make Daxa simple and easy ro use even if that has a small performance cost.

- While other vulkan abstactions aim to be overly generalized, daxa is aiming to be more focused on modern desktop gpus. This allows daxa to make tradeoffs better suited for modern gpus and modern techniques while also staying simpler.

- Daxa is made for gpu driven applications. This is due to me [Patrick](https://www.github.com/Ipotrick) and [Gabe](https://www.github.com/GabeRundlett) beeing very intrested in these types of gpu workloads. Other abstractions simply did not satisfy us for our needs in this area.

- Daxa is meant to be internally very simple and should not have multiple ways of doing the same thing. If a feature is used very little it will be axed. Having less code means less bugs while also making it easier to understand and maintain.

- Daxa's internals are meant to be efficient and impose very low overhead. Its design is with cpu performance in mind. Daxa does very little tracking in the api itself and is also very restrictive on quickly accumulating costs like frequent hash map lookups or spamming reference count in/decrements.
Daxa currently imposes as minimal overhead in most parts of the API, compared to raw Vulkan it is practically always under 1% overhead compared to raw vulkan calls in the tested applications.

- Daxa is made in two layers. The core API and the utils. The utils contain things like the ImGui integration, PipelineManager and TaskGraph. This split is made to keep the responsibilities of the different parts of Daxa strict
