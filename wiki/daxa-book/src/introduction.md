# Introduction

Welcome to the *daxa api book*. This book serves as the documentation for the daxa vulkan abstraction library.

Daxa is currently available to be used in c++ and c but there will also be a rust api in the future. However the api does not significantly differ between languages, therefore this book predomenantly uses the c++ api for explanations and examples.

# Why use daxa?

There are plenty other vulkan abstraction libraries like: [vuk](https://github.com/martty/vuk), [phobos](https://github.com/NotAPenguin0/phobos-rs), [magma](https://github.com/vcoda/magma), [autovk](https://github.com/cg-tuwien/Auto-Vk) among others. So why is there a need for daxa to exist in the first place?

Compared to most of these apis daxa has some unique abstractions and design that is not present in most other apis, these are (among others):
- Bindless shader resource model. Resources are exclusively accessed via an ID (or address) instead of descriptor set bindings in shaders. This drastically simplifies the API while also being a very powerful tool for GPU-driven rendering. (https://github.com/Ipotrick/Daxa/tree/master/wiki/Bindless.md)
- Host and shader code sharing utilities. This makes shader development and interop much simpler with no need for convoluted complex systems like shader reflection. (https://github.com/Ipotrick/Daxa/tree/master/wiki/ShaderIntegration.md)
- focus on modernity. Daxa will abstract most older gpu api concepts that complicate and limit the programmer with little to no performance benefit on modern gpus. This makes daxa simpler compared to other apis while beeing as powerful.
- A full c api. The c++ api and all functionality is implemented in a c api. The c++ api fully abstracts over this c api. The c api also has full direct vulkan interop and binary compatibility with all daxa c++ objects. Be free to use the c api to create bindigns to other langauges!
- High debugability. Daxa has extensive and strict checks with very low overhead. Most input is validated. Daxa will return detailed error messages on missuse, it attempts to prevent most potential driver crashes and corruptions.
> It is made sure that the validation overhead over the plain vulkan function calls stays below 5% in release builds!

Apart from these, daxa provides important common vulkan abstraction features:
- Powerful and flexible render graph. While manual sync is exposed in Daxa for best control, it also provides an optional render graph. (https://github.com/Ipotrick/Daxa/tree/master/wiki/TaskGraph.md)
- Shader build system, including features like `#include` management and shader hot-reloading. (https://github.com/Ipotrick/Daxa/tree/master/wiki/PipelineManager.md)
- Greatly simplified API surface compared to Vulkan. Daxa is only surfacing those concepts that are crucial for good control and performance. [No more 1k LOC files to get a triangle](https://github.com/Ipotrick/Daxa/tree/master/wiki/Tutorial.md)
- Simplified and semi managed object lifetimes to prevent destroying objects still in use on the gpu.

Daxa has many more smaller features that will be explained in the book.

# Daxas Abstraction Level

Daxa is not "high" or "low" level. Some vulkan concepts are explicit and others are abstracted. The choice of what is abstracted is made on experience and modern hardware recommendations. If a vulkan concept does not benefit modern hardware but increases complexity its abstracted.

For example sync is fully explicit in daxa as there are many cases where manual sync can be very important for performance. On the other side daxa FULLY abstracts `VkDescriptorSet`, `VkDescriptorSetLayout`, `VkDescriptorPool` and all the related functions. This is because there is no gain in the user manually managing these as modern GPUs prefer [bindless](LINK ME).

> Dont worry you DO NOT have to write manual sync in daxa! Daxa has a very powerful render graph utility (called Task Graph). It will handle all resource transition and synchronization between user defined actions. Additionally Task Graph also attemps to optimize the order of task submission to minimize the barriers and resource transitions.

In general you can expect writing daxa code is much easier and more productive then raw vulkan and simpler then other apis like OpenGL. 

There might be important vulkan features missing in daxa. That is because daxa only implements what is actually used. If noone requests a certain thing, daxa wont have it! Keeping it simple and NOT overgeneralizing is important.