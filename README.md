# This repo is the home of the Daxa GPU-library

Daxa is a gpu rendering interface library. It is meant to be modern and easy to use.

Aside from the gpu abstraction, daxa also includes a few generalized helper abstractions, like an ImGui backend or a simple asset manager. These are included mainly to get things going in a new project.

## Features:

* easy debugability.
* easy shader resource model (bindless):
  * no descriptor sets
  * no binding slots
  * no ressource binding
  * resources are passed to and accessed in a shader via an id
  * allows for much easier access in gpu driven rendering and raytracing
* little boilerplace and many defaults:
  * optional automatic staging buffer usage for data up- and download
  * easy ressource creation
  * no renderpasses
  * no framebuffers
  * dynamic state
* simple shader build system:
  * dxc compiler integration for vulkan-hlsl
  * hot reloading
  * full preprocessor integration
* threadsafety:
  * TODO 
* explicit synchronization:
  * direct control
  * no forced high-overhead auto-sync
  * cpu cost in recording commands is in place and not defered to submission
* rendergraph:
  * TODO

While this library is build on vulkan, it will not expose all vulkan features. The main goal is beeing modern, fase and still reasonably simple.

# Examples
Examples are given as DaxaXYZ, besides the daxa library itself in this repository. There are also two full projects currently residing in this repository:
DaxaMinecraft, which is a raytraced voxel engine implemented mainly by Gabe Rundlett and also me.
DaxaMeshview, which is a general mesh view utility in which i implement various rendering techniques for normal triangle meshes.

# DaxaMeshview and DaxaMinecraft

There are two projects in this repository which are built upon Daxa. These are used for testing the api but also toy projects of me and a friend (Gabe Rundlett).

# Building
The library and the examples can be built using cmake with vcpkg. There is an included vcpkg.json, which should pull all the nessecary dependencies for everything in this repository.
The library is tested using the msvc stl, with clang and the msvc compiler. I recommend building the project with cmake and ninja.
