# This repo is the home of the Daxa engine

Daxa is a small library, that can be used to render 3d worlds and make games.

## Daxa GPU

The core part of this library is the gpu abstraction. The initial purpose of this project was to learn the vulkan api, but i quickly realized that vulkan can not be used as easily as opengl. Coming from OpenGL, many things like ressource lifetimes or initialization suddenly become much more complicatied in vulkan. So decided to write an abstraction layer for vulkan called daxa::gpu.

The main goals and features of daxa::gpu are:
* automatic lifetime management via ref counting.
* easy initialization of ressources like the device, instance, images and buffers.
* automatic pooling and reuse of ressources.
* automatic shader reflection and descriptor set (they are called binding sets in daxa::gpu) creation.
* not making too many compromises on exposed vulkan capabilities or valid usages.
* reducing boilderplate synchronization while still beeing very explicit.

A brief explanation for each of these goals:
* in vuklkan ressources can have complex lifetimes, also ressources must not be free#d when they are still used by the gpu. This entails complex problems that are usually adressed with things like zombie lists or delaying destruction by N frames. Most of these solutions are very dissatisfying to me and often entail a lot of restrictions esspecially with multithreadding. I make the compromise that i give up some cpu performance by ref counting ressource in use in command buffers. This works in pretty much all scenarios and doesnt have the weird problems with complex lifetimes.
* initialization in vulkan is very explicit and there are no default values for anything. This means one has great controll over what is happening but also creates a lot of boilerplate code. In daxa::gpu i've decided that i want to use as many default values as possible and also use shader reflection to reduce the boilderplate and redundancy of ressource creation while not limiting the expressiveness.
* many vulkan objects like command buffers, command pools or descriptor sets can be reused wich saves cpu time. In daxa::gpu the ref counting handles automatically recycle vuklkan objects like descriptor sets, so that the user doesnt need to bother writing their own pooling for these objects.
* descriptor sets are a big pain in plain vulkan in my opinion. DescriptorSetLayout, DescriptorPool, redundancies in shader code and making sure every set is compatible makes them a big hurdle to use. Shader reflection and other abstractions help a great deal in reducing the redundancy of descriptor set usage.
* As i said before, many abstractions make compromises on ressource lifetimes like deleting things with N render frames delay. These and other constrains constrain the user to a specific application model that i personally don't like. These things can also cause very obscure bugs that are hard to debug. So Things like queues, descriptor sets and synchronization are fully exposed in daxa::gpu.
* automatic synchronization like barrier generation can create hughe cpu overhead, many abstractions like wgpu need to make many hash map accesses for the barrier generation. Syncrhonization may be a lot of boilderplate but usually dont make up too much trouble in my opinion. Automatic sync also causes a much more complex implementation wich i am not willing to make. Therefore semaphores and barriers are completely exposed in the api.

Daxa also includes other rendering and application specific classes and other abstractions.
Many of them are rendering related like an ImGui backend or Camera controllers.

# Examples
This repository also includes many examples of daxa usage as well as projects that i and a friend of mine (Gabe) are working on. The library itself is developed by me, but Gabe is actively using the gpu api as an opengl replacement, so he pushes updates and projects that he made or ported to daxa. 

# Building
The library and the examples can be build using cmake with vcpkg. There is an included vcpkg.json, wich should pull all the nessecary dependencies for everything in this repository.
The library is tested using the msvc stl, with clang and the msvc compiler. I recommend building the project with cmake and ninja.
