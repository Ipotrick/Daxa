# PipelineManager

As Daxa is designed to be a GPU-driven centric API, we provide some code which is meant to be used within your shaders. This is in the form of `.inl` files, such as the core `daxa/daxa.inl`, as well as the newer `daxa/utils/task_graph.inl`.

As such, Daxa provides the PipelineManager util which is meant to be used in the development phase of your app, in order to very quickly iterate on your GPU code.

PipelineManager is mainly designed on top of [Khronos' glslang library](https://github.com/khronosGroup/glslang), providing GLSL to SPIR-V compilation. However, PipelineManager doesn't generate SPIR-V for you to feed to Daxa's Pipeline API. PipelineManager completely manages the pipelines for you. It does this because this way it can do much more for you.

## Hot Reloading
The major feature of PipelineManager is the hot-reloading. When the shader code is changed and saved, it will recompile the pipeline. You can even `#include` headers in your shaders, and when the code in those files is updated, the PipelineManager will automatically recompile the affected pipelines for you.
