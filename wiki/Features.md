## Easy Resource Lifetimes:
- Most resources are reference counted in daxa (Instance, Device, Swapchain, Semaphore, ...) for convenience.
- For shader resources (Buffer, Image, ImageView, Sampler) however, daxa does not reference count, because  :
  - Bindless shader resources ids/addresses can be stored in buffers and other memory, making daxa unable to consistently track them.
  - In api calls, they are mentioned in much higher frequency compared to other objects, making their tracking with reference counts much more expensive.
  - In contrast to other objects it is very common to build other more specialized lifetime management for shader resources (eg. static, cached, streamed, dynamically created, ...).
- The destruction of any resource is deferred until all submits, currently running at the time of the destroy call, are finished. This makes it safe to call destroy on ressources in most cases.
- Extensive debug checks for any number of cases with sensible error messages detailing what could have gone wrong.
- Thread safety that can optionally be disabled via a preprocessor makro define.
  - Calls to CommandRecorder from multiple threads is NOT threadsafe.
  - Utils generally are not threadsafe.
  - All Other objects are generally threadsafe to access from multiple threads simultaneously.

## Ergonomic Swapchain:
- Swapchain contains its own semaphores for timing:
  - acquire semaphores, one for each frame in flight,
  - present semaphores, one per swapchain image,
  - a timeline semaphore, used to limit frames in flight.
- The correct set of seamphores can be queryied with member functions, they are changed after every acquire call.
- The swapchain integrates frame in flight limiting, as it is natural for it to handle this.
- Easy to use interface for recreating the swapchain.

## Simple Renderpasses:
- Daxa does not have the concept of a precreated renderpass nor framebuffers.
- To render, you begin and end a renderpass section directly within a command list.
- This can be done very efficiently, daxa does not cache or lazy create anything for this behind the users back, as vulkan 1.3 has a feature mapping directly to this interface.
- This drastically simplifies rendering and removes a lot of object coupling. Pipelines are completely decoupled of framebuffers or renderpass objects.

## Powerful CommandRecorders:
- The vulkan command pool is abstracted. Daxa maintains a pool pool inside each device.
- CommandRecorders make command buffers and command pools easier to use and much safer per design.
- Each recorder is a pool + buffer. Command buffer can be completed at any time, a new command buffer is used with the same pool after completing current commands.
- PipelineBarrier api is made more ergonomic:
  - CommandRecorder does not take arrays of memory barriers and image memory barriers, it takes them individually.
  - As long as only pipeline barrier commands are recorded in sequence, they will be batched into arrays.
  - As soon as a non-pipeline barrier command is recorded, all memory barriers are recorded to the command buffer.
  - This effectively is mostly syntax suggar, but can be quite nice to reduce vulkan api calls if the barrier insertion is segmented into multiple functions.
- As mentioned above in the lifetime section, CommandRecorders can record a list of shader resources to destroy after the command list has finished execution on the gpu.
- CommandRecorder changes type in renderpasses to ensure correct command use by static typesafety!
- CommandRecorders have a convenience function to defer the destruction of a shader resource until after the command list has finished execution on the gpu.
  - Very helpful to destroy things like staging or scratch buffers.
  - Nessecary as it is not legal to destroy objects that are used in commands that are not yet submitted to the gpu.

## Tiny Pipelines:
- Pipelines are much simpler in daxa compared to normal vulkan, they are created with parameter structs that have sensible defaults set for all parameters.
- Because of the full bindless interface and full descriptor set and descriptor set layout abstraction, you do not need to specify anything about descriptors at pipeline creation.
- Daxa again does this very efficiently with no on the fly creation, caching or similar, all cost is fixed and on creation time.
- As Renderpasses and framebuffers are abstracted, those also do not need to be mentioned, pipelines are decoupled.
- The pipeline manager utility makes pipeline management very simple and convenient.

## Effective use of modern Language features:
- Nearly all functions take in structs. Each of these info structs has defaults. This allows for named parameters and out of order defaults!

## Safety and Debuggability:
- ids and refcounted handles are fully threadsafe
- all vulkan function return values are checked and propagated
- lots of custom checks are performed and propagated
- lots of internal validation to ensure correct vulkan use
- preventing wrong use by design
- Using typesafety for correct command recording
- Integrated object naming (used by tooling validation layers and daxa itself).

## Other Features:
- Automatically managed memory allocations with VMA. Optionally exposed manual management.
- Daxa stores and lets you get the info structs used in object creation. Very useful, as you do not need to store the objects metadata yourself.
- Abstraction of image aspect. In 99% of cases (daxa has no sparse image support) daxa can infer the perfect image aspect for an operation, which is why its abstracted to reduce boilerplate.
- Next to bindless, daxa also provides simplified uniform buffer bindings, as some hardware can still profit from these greatly. 
- FSR 2.1 integration
- ImGUI backend
- Staging memory allocator utility. Provides a fast linear staging memory allocator for small per frame uploads.
- A kick ass logo
