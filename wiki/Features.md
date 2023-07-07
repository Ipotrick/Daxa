## A list of other features of daxa:
* Extensive debug checks for any number of cases with sensible error messages detailing what could have gone wrong.
* Thread safety that can optionally be disabled via a preprocessor makro define.
  * Calls to CommandList from multiple threads is NOT threadsafe.
  * Utils generally are not threadsafe.
  * All Other objects are generally threadsafe to access from multiple threads simultaneously.
* Shader integration for task definitions.
* Automatically managed memory allocations with VMA. Optionally exposed manual management.
* Integrated object naming (used by tooling validation layers and daxa itself)
* Ergonomic Swapchain abstraction. Present, aquire and a timeline semaphore are integrated and owned by the swapchain. This reduces builderplate and groups these connected objects.
* Rasterized rendering is exposed with the dynamic rendering extension. Daxa mostly matches this interface. This removes the need to unergonimoc objects like framebuffers and renderpasses and their annoying coupling with pipelines.
* Common shader extensions are already enabled and integrated into daxa. Shader int64 for images buffers and in general is exposed, buffer references are the default in glsl, etc. .
* Simplified pipeline barrier interface. Multiple memory and image memory barrier calls are merged into single pipeline barrier calls. Access mask and stage masks are combined.
* Daxa stores and lets you get the info structs used in object creation. Very useful, as you do not need to store the objects metadata yourself.
* Full vulkan abstraction. Vulkan.h does not leak into daxas namespace. This has several up and downsides. Biggest upside beeing proper full integration for all exposed features, as well as greater control and freedom in the abstractions daxa has.
* General simplifications everywhere. Many function calls and objects are simpler to manage then in raw vulkan.
* Abstraction of image aspect. In 99% of cases (daxa has no sparse image support) daxa can infer the perfect image aspect for an operation, which is why its abstracted to reduce boilerplate.
* Abstracted Command buffer and pool. Daxa internally reuses command pools for you.
* Simplified pipelines. Pipelines do not need any knowledge of descriptor set layouts, do not need a pipeline layout and also do not need any information about the renderpass it is used in.
* Simplified queue interface. The device contains the (currently) only queue. In the future there will optionally be separate compute and transfer queus.
* Next to bindless, daxa also provides simplified uniform buffer bindings. 
* Simple vector and matrix math library. Integrated with daxa types for shader integration.
* FSR 2.1 integration
* ImGUI backend
* Staging memory allocator utility. Provides a fast linear staging memory allocator for small per frame uploads.
* A kick ass logo