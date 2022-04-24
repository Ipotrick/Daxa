# This repo is the home of the Daxa GPU-library

Daxa is not aimed to expose all vulkan features or even stay very close to it api wise. 

Its primary goal is be easy to use, very low cpu overhead and modern gpu feature access.

Aside from the gpu abstraction, daxa also includes a few generalized helper abstractions, like an ImGui backend or a simple asset manager. These are included mainly to get things going in a new project.

## Features:

* easy debugability.
* easy shader resource model (bindless):
  * no descriptor sets
  * no binding slots
  * no ressource binding
  * resources are passed to and accessed in a shader via an id
* easy gpu driven rendering:
  * binding model allows for very easy gpu side manipulation of resources
  * (TODO) more
* little boilerplace with many defaults:
  * optional automatic staging buffer usage for data up- and download
  * easy ressource creation
  * no renderpasses
  * no framebuffers
  * dynamic state with defaults
* simple shader build system:
  * dxc compiler integration for vulkan-hlsl
  * shaderc compiler integration for glsl
  * hot reloading
  * full preprocessor integration
* threadsafety:
  * all functions can be called in any context without external synchronization.
  * this usually does not affect performance except for resource creation which is serialized.
* efficient automatic lifetime management of resources:
  * reference counting to enforce shared ownership
  * resources are freed after all potential uses in command lists are over
* explicit synchronization:
  * auto sync usually comes with a high price in cpu overhead
  * allows for implementation of custom rendergraph
  * cpu cost in recording commands is in place and not defered to submission
  * makes fun times when debugging
## Notable exposed vulkan features:
* rasterization graphics pipeline
* rasterization dynamic state
* compute pipeline
* multiqueue

# Examples
Examples are given as DaxaXYZ, besides the daxa library itself in this repository. There are also two full projects currently residing in this repository:
DaxaMinecraft, which is a raytraced voxel engine implemented mainly by Gabe Rundlett and also me.
DaxaMeshview, which is a general mesh view utility in which i implement various rendering techniques for normal triangle meshes.

# DaxaMeshview and DaxaMinecraft

There are two projects in this repository which are built upon Daxa. These are used for testing the api but also toy projects of me and a friend (Gabe Rundlett).

# Building
The library and the examples can be built using cmake with vcpkg. There is an included vcpkg.json, which should pull all the nessecary dependencies for everything in this repository.
The library is tested using the msvc stl, with clang and the msvc compiler. I recommend building the project with cmake and ninja.
