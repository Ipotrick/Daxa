This page will explain how to create a basic app in daxa.

The daxa repo provides several samples that show daxa use and also serve as tests. This wiki page is meant as a tutorial to those who are already familiar with apis like vulkan or dx12 and want to see how to write an app with daxa. This tutorial will go into specific detail on daxa's special features and design decisions.

# 1. Setup

![image](images/pink-screen.png)

In the fist part of this tutorial you will see how to setup daxa. 

This part will introduce the most basic parts of the api up to the point of having a loop and clearing the window to a certain color.  
## Creating a daxa::Context

To do any Daxa API calls, one needs to create a `daxa::Context`.

```cpp
#include <daxa/daxa.hpp>

int main()
{
    daxa::Context context = daxa::create_context({});
}
```

A curious thing you will notice is that most function calls take a struct as a parameter.
This is done to emulate named parameters and also to enable out-of-order default argument values using C++20"s designated initializers.

Daxa is a relatively explicit API, but has a lot of defaults via struct default member values. This makes it much nicer to use in many cases.

Nearly all Daxa objects can be assigned a debug name in creation. This name is used in the error messages we emit, and is also displayed in tools like RenderDoc.

## Choosing a daxa::Device

To use a GPU, you must create a daxa::Device. This object represents the GPU and is used to issue commands to it.

As it is possible to have multiple GPUs in a system, one can create multiple `daxa::Device`s.
To choose a device from the available devices, one can optionally provide Daxa with a callback rating a device's suitability.

```cpp
daxa::Device device = context.create_device({
    .selector = [](daxa::DeviceProperties const & device_props) -> daxa::i32
    {
        daxa::i32 score = 0;
        switch (device_props.device_type)
        {
        case daxa::DeviceType::DISCRETE_GPU: score += 10000; break;
        case daxa::DeviceType::VIRTUAL_GPU: score += 1000; break;
        case daxa::DeviceType::INTEGRATED_GPU: score += 100; break;
        default: break;
        }
        score += static_cast<daxa::i32>(device_props.limits.max_memory_allocation_count / 100000);
        return score;
    },
    .debug_name = "my device",
});
```

A notable thing here is that Daxa defines integer types to its own naming convention. What these types represent is very straightforward, the first letter is the datatype and the number is the bit width, so i32 is an integer with 32 bits, and f64 would be a floating point value with 64 bit width. These are identical to the primitive types in languages such as Rust. You can make them globally accessible without the namespace by `using namespace daxa::types;`

## Creating a daxa::Swapchain

To be able to render to a window, Daxa requires the user to create a swapchain for a system window.

Daxa itself is only a GPU API, so it can not create windows on its own. Creating a window must be handled by a windowing API like GLFW or SDL.

To create a Swapchain, Daxa needs a native window handle and native platform. Optionally, the user can provide a debug name, surface format type selector, the additional image uses (image uses will be explained later), and present mode (this controls sync):

```cpp
daxa::Swapchain swapchain = device.create_swapchain({
    // this handle is given by the windowing API
    .native_window = native_window_handle,
    // The platform would also be retrieved from the windowing API,
    // or by hard-coding it depending on the OS.
    .native_window_platform = native_window_platform,
    .surface_format_selector = [](daxa::Format format)
    {
        switch (format)
        {
        case daxa::Format::R8G8B8A8_UINT: return 100;
        default: return daxa::default_format_score(format);
        }
    },
    .present_mode = daxa::PresentMode::MAILBOX,
    .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
    .debug_name = "my swapchain",
});
```

Now with the swapchain, the user can request a swapchain image to render to:

```cpp
daxa::ImageId swapchain_image = swapchain.acquire_next_image();
```

If all swapchain images are used in queued submissions to the GPU, the present call will block. `daxa::Swapchain::acquire_next_image()` will always immediately return The Swapchain will also control frames in flight. It controls the acquire, present and frames in flight semaphores. Each of those can be queried with a function from the swapchain.

## Creating a daxa::CommandList

To execute commands on the GPU, one needs to record them into a daxa::CommandList and submit them to the GPU.

```cpp
daxa::CommandList command_list = device.create_command_list({.debug_name = "my command list"});
```

## Sending commands to the gpu

The first thing we will do on the GPU using Daxa is simply set all pixels of the window to a pink color.

The following is an example of a render-loop.

```cpp
daxa::ImageId swapchain_image = swapchain.acquire_next_image();
if (swapchain_image.is_empty())
{
    continue;
}
```

The first thing that is done is retrieving the current swapchain image. This is the image we are going to clear. If swapchain image acquisition failed, then the result of this acquire function will be an "empty image", and thus we want to skip this frame. If we were inside a while loop, this means just saying `continue`.

```cpp
daxa::ImageMipArraySlice swapchain_image_full_slice = device.info_image_view(swapchain_image.default_view()).slice;
```

Directly after, we define an image slice. As images can be made up of multiple layers, memory planes, and mip levels, Daxa takes slices in many calls to specify a slice of an image that should be operated upon.

In Daxa, an image view of any image's full range can be retrieved with the `daxa::ImageId::default_view()` member function. From an image view, we can query the slice it represents. In the case of the default view, its slice is the whole image, which is what we want.

```cpp
daxa::CommandList command_list = device.create_command_list({.debug_name = "my command list"});
```

We then use the command list to record our commands. 

```cpp
command_list.pipeline_barrier_image_transition({
    .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
    .before_layout = daxa::ImageLayout::UNDEFINED,
    .after_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
    .image_slice = swapchain_image_full_slice,
    .image_id = swapchain_image,
});
```

We insert a pipeline barrier that also performs an image layout transition. Pipeline barriers are nearly identical to vulkan pipeline barriers. The image layouts also correspond closely to vulkan image barriers. The Access and stage are merged into daxa::Access. This is don't to simplify synchronization. Each Stage has a write, a read and a read write access.

Also do not be concerned that barriers must be issued with multiple function calls. Daxa will queue all back to back pipeline barrier calls and do a single vulkan pipeline barrier command. The idea here is that its simpler for the user to not group up barriers manually and submit them once, but that daxa does that automatically for the user.

```cpp

command_list.clear_image({
    .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
    .clear_value = std::array<daxa::f32, 4>{1.0f, 0.0f, 1.0f, 1.0f},
    .dst_image = swapchain_image,
    .dst_slice = swapchain_image_full_slice,
});

command_list.pipeline_barrier_image_transition({
    .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
    .before_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
    .after_layout = daxa::ImageLayout::PRESENT_SRC,
    .image_slice = swapchain_image_full_slice,
    .image_id = swapchain_image,
});

command_list.complete();
```

We then issue a image clear. It is possible to also set the image layout in this command, but there is a default image layout, which is `TRANSFER_DST_OPTIMAL`, the one we transition the layout to earlier.
After that, we transition the image layout to present optimal, as we will present the image after these commands are done.
In the end we complete the command list. After this the command list is considered constant and no more commands can be recorded into that list. It is required that all submitted command lists are completed before submission.

```cpp
auto const & acquire_semaphore = swapchain.get_acquire_semaphore();
auto const & present_semaphore = swapchain.get_present_semaphore();
auto const & gpu_timeline = swapchain.get_gpu_timeline_semaphore();
auto const cpu_timeline = swapchain.get_cpu_timeline_value();

device.submit_commands({
    .command_lists = {command_list},
    .wait_binary_semaphores = {acquire_semaphore},
    .signal_binary_semaphores = {present_semaphore},
    .signal_timeline_semaphores = {{gpu_timeline, cpu_timeline}},
});
device.present_frame({
    .wait_binary_semaphores = {present_semaphore},
    .swapchain = swapchain,
});
```

In daxa, the swapchain handles the number of frames in flight and related frame to frame image acquisition and present synchronization.

The after each frame the swapchain provides an acquire semaphore, a present semaphore, a gpu timeline semaphore and a cpu timeline value.

The first submit in a frame, that uses the swapchain image MUST wait on the acquire semaphore signaling, as the swapchain image is only valid after this semaphore is signaled.

The last submit in a frame that uses the swapchain image MUST signal the present semaphore, as the gpu must know when the last access happens until it can present the image.

The cpu and gpu timeline values are used internally to limit the frames in flight. the gpu timeline value must be signaled with the last submission with the current cpu timeline value.

Note that the acquire and present semaphores can change every frame, so do NOT store them once and reuse. You must call the get functions after each image acquisition.

To see a full implementation go ahead to the daxa samples and look into the 4_hello_daxa folder. For every chapter in this tutorial there is a fully working encapsulated program with all the necessary code. This code is also fully commented out, so it can be used as an alternative to this tutorial entirely if preferred.

# 2. Triangle

![image](images/triangle.png)

In the next step we will render a triangle.

We will also introduce a few utils: the PipelineManager and the TaskList.

## Pipelines

Daxa currently provides two types of pipelines, compute pipelines and raster pipelines. To render triangles with the graphics pipeline we need the raster pipeline.

```cpp
daxa::RasterPipeline pipeline = device.create_raster_pipeline(daxa::RasterPipelineInfo{
    .vertex_shader_info = {
        .byte_code = {...},
        .entry_point = "...", // This is a std::optional. It defaults to "main"
    },
    .fragment_shader_info = {
        .byte_code = {...},
        .entry_point = "...", // This again defaults to "main"
    },
    .color_attachments = {...},
    .depth_test = {...},
    .raster = {...},
    .push_constant_size = {...},
    .debug_name = {...},
});
```

The User must provide at least a vertex shader and a fragment shader info.
The shader infos are simply spirv bytecode and the name of the entry point. 
How the shader bytecode is generated is up to the user.

Daxa provides a util to handle pipelines, the PipelineManager. 
This manager handles file loading, compilation and even extra features like live hot reloading for the user. 
We will use this PipelineManager util.

```cpp
auto pipeline_manager = daxa::PipelineManager({
    .device = device,
    .shader_compile_options = {
        .root_paths = {
            DAXA_SHADER_INCLUDE_DIR,
            "./tests/4_hello_daxa/2_triangle",
        },
        .language = daxa::ShaderLanguage::GLSL,
        .enable_debug_info = true,
    },
    .debug_name = "my pipeline manager",
});
```

`DAXA_SHADER_INCLUDE_DIR` is a macro provided by Daxa when the target is linked to with CMake. This macro is automatically the value of the path to the daxa shader headers.

```cpp
auto result = pipeline_manager.add_raster_pipeline({
    .vertex_shader_info = {.source = daxa::ShaderFile{"main.glsl"}},
    .fragment_shader_info = {.source = daxa::ShaderFile{"main.glsl"}},
    .color_attachments = {{.format = swapchain.get_format()}},
    .depth_test = {},
    .raster = {},
    .push_constant_size = sizeof(MyPushConstant),
    .debug_name = "my pipeline",
});
```

The pipeline manager returns a `daxa::Result`, as the compilation of shaders can fail. The result contains an error message in that case. 

```cpp
auto result = pipeline_manager.add_raster_pipeline({
    // ...
});

if (result.is_err())
{
    std::cout << result << std::endl;
    std::terminate();
}

auto pipeline = result.value();
```

## Shaders

Now we will take a look at shaders and how they integrate with daxa.
This section will only be a brief overview of what is needed to be able to complete the tutorial, for a more detailed explanation of Daxa's shader integration look at the dedicated wiki page for shader integration.

```glsl
// draw.glsl:

// Includes the daxa api to the shader
#include <daxa/daxa.inl>

#include <shared.inl>

// The first argument mentions the struct you want as your push constant,
// the second argument will be the name of it.
DAXA_USE_PUSH_CONSTANT(MyPushConstant, p)

#if DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_VERTEX

layout(location = 0) out daxa_f32vec3 v_col;
void main()
{
    MyVertex vert = deref(p.my_vertex_ptr[gl_VertexIndex]);
    gl_Position = daxa_f32vec4(vert.position, 1);
    v_col = vert.color;
}

#elif DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_FRAGMENT

layout(location = 0) in daxa_f32vec3 v_col;
layout(location = 0) out daxa_f32vec4 color;
void main()
{
    color = daxa_f32vec4(v_col, 1);
}

#endif
```

To access any of Daxa's functionality in shaders, one must include the daxa.inl. This header is used for shaders and shared files (those will be discussed later).

In daxa all resource access in shader is bindless. To read a buffer, one must enable buffer pointers for their type with DAXA_ENABLE_BUFFER_PTR, then pass the buffer device address in a push constant, to then access it with the deref operator in the shader. In this particular case, we also use the [] operator to perform pointer arithmetic, adding the vertex offset onto the base pointer. `BufferPtr`s act like normal pointers in that case, the syntax is just a bit different.

Now we go over the shared file.

```cpp
// shared.inl:
#pragma once

// Includes the daxa api to the shader
#include <daxa/daxa.inl>

struct MyVertex
{
    daxa_f32vec3 position;
    daxa_f32vec3 color;
};

// Allows the shader to use pointers to MyVertex
DAXA_ENABLE_BUFFER_PTR(MyVertex)

struct MyPushConstant
{
    daxa_BufferPtr(MyVertex) my_vertex_ptr;
};
```

The shared file curiously uses `daxa_` prefixed type names. Daxa provides renamed types for all primitive GLSL types. These type-names translate to either corresponding GLSL or C++ types, depending on where the inl file is included.
This allows for code sharing between C++ and GLSL, as many things like `struct`s only need to be declared once instead of multiple times.

In this instance the shared file allows us to define the vertex and push constants `struct`s once and then use them in our shader AND C++ code.

In the shared file we declare the MyVertex struct and enable buffer pointers to it via `DAXA_ENABLE_BUFFER_PTR`. We also use the daxa provided type `daxa_f32vec4` here which will translate to a `vec4` in GLSL and `daxa::f32vec4` in C++.

Then lastly we declare a strut for the push constant. This struct contains a buffer pointer. This buffer pointer will become a `daxa::BufferDeviceAddress` in C++.

To learn more about this i again refer to the shader integration wiki page.

## Renderpass

To use a raster pipeline we need a renderpass. Within a renderpass the user can bind pipelines and issue draw calls.

Setting up a renderpass in daxa is very simple, there is a command to begin a renderpass and one command to end it.

Renderpasses can clear the screen as a load operation, this means we do not need the manual clear anymore.

```cpp
cmd_list.begin_renderpass({
    .color_attachments = {
        daxa::RenderAttachmentInfo{
            .image_view = swapchain_image.default_view(), 
            .load_op = daxa::AttachmentLoadOp::CLEAR, 
            .clear_value = std::array<f32, 4>{0.1f, 0.0f, 0.5f, 1.0f},
        },
    },
    .render_area = {.x = 0, .y = 0, .width = size_x, .height = size_y},
});
cmd_list.set_pipeline(*raster_pipeline);
cmd_list.push_constant(MyPushConstant {
    .vertex_buffer_ptr = device.get_device_address(vertex_buffer),
});
cmd_list.draw({.vertex_count = 3});
cnd_list.end_renderpass();
```

We begin a renderpass which clears the screen as a load op. We then set the pipeline to the raster pipeline we created earlier. Setting the push-constant uses the struct from the shared file. Finally we issue a draw-call and end the renderpass.

## Buffer upload

We also need to upload the vertex buffer to the gpu. We could just hardcode the vertex positions and colors in the shader but for demonstration we will upload the vertex buffer.

The command list has functionality to make this easy.

Let's start by creating a function which will upload the vertex data to a buffer:
```cpp
void upload_vertex_data_task(
    daxa::Device & device,
    daxa::CommandList & cmd_list,
    daxa::BufferId buffer_id);
```
We'll give it three parameters, the device, a command list, and the buffer to upload to. Inside this function, we'll create an array of data to send to the GPU. This array consists of just 3 elements of `MyVertex`, which is a struct defined in our `shared.inl` file.
```cpp
auto data = std::array{
    MyVertex{.position = {-0.5f, +0.5f, 0.0f}, .color = {1.0f, 0.0f, 0.0f}},
    MyVertex{.position = {+0.5f, +0.5f, 0.0f}, .color = {0.0f, 1.0f, 0.0f}},
    MyVertex{.position = {+0.0f, -0.5f, 0.0f}, .color = {0.0f, 0.0f, 1.0f}},
};
```

In order to send the data to the GPU, we can create a staging buffer, which has host access, so that we can then issue a command to copy from this buffer to the dedicated GPU memory.
```cpp
auto staging_buffer_id = device.create_buffer({
    .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
    .size = sizeof(data),
    .debug_name = "my staging buffer",
});
```
We can also ask the command list to destroy this temporary buffer, since we don't care about it living, but we DO need it to survive through its usage on the GPU (which won't happen until after these commands are submitted), so we tell the command list to destroy it in a deferred fashion.
```cpp
cmd_list.destroy_buffer_deferred(staging_buffer_id);
```

> Note: Instead of doing this manually, we could use one of Daxa's other useful utilities, "Mem", but it's simple enough for now.

We then get the memory mapped pointer of the staging buffer, and write the data directly to it.
```cpp
auto * buffer_ptr = device.get_host_address_as<std::array<MyVertex, 3>>(staging_buffer_id);
*buffer_ptr = data;
```

And finally, we can just copy the data from the staging buffer to the actual buffer.
```cpp
cmd_list.copy_buffer_to_buffer({
    .src_buffer = staging_buffer_id,
    .dst_buffer = buffer_id,
    .size = sizeof(data),
});
```

## TaskList

Manual synchronization is very error prone and difficult to optimize and maintain. Writing some automatic synchronization abstraction is mandatory for any complex program using vulkan.

Daxa provides a util called TaskList. TaskList allows us to compile a list of GPU tasks and their dependencies into a synchronized set of commands.
This simplifies your code by making different tasks completely self-contained, while also reordering and generating optimized synchronization for the given tasks automatically.
TaskList is a simple auto-synch abstraction that can be used "as is" or as a reference to design your own auto-synch on top of daxa.

We are going to use TaskList here, as a showcase of how it is used and to keep this tutorial shorter.

```cpp
enum class TaskCondition
{
    VERTICES_UPLOAD,
    COUNT,
};

std::array<bool, static_cast<daxa::usize>(TaskCondition::COUNT)> task_condition_states{};

auto loop_task_list = daxa::TaskList({
    .device = device,
    .swapchain = swapchain,
    .permutation_condition_count = static_cast<daxa::usize>(TaskCondition::COUNT),
    .debug_name = "my task list",
});
```

TaskList works by recording tasks, completing the list and then repeatedly using this completed list over multiple frames.
Alternatively the user can re record the task list every frame when the rendering changes. Re recording takes time, so it is better to record once and reuse TaskLists as much as possible.

Because reuse is desired, a single TaskList can have permutations. 
Permutations allow for runtime conditions to trigger different outcomes. Permutations are generated for a set of conditionals, that can be set at runtime between executions. 

For our example we only have one condition, which is whether or not its the first frame or not. In the first frame we want to do some initialization, which we do not want to do every frame.

```cpp
auto task_swapchain_image = loop_task_list.create_task_image({.swapchain_image = true, .debug_name = "my task swapchain image"});
auto swapchain_image = daxa::ImageId{};
loop_task_list.add_runtime_image(task_swapchain_image, swapchain_image);

auto task_buffer_id = loop_task_list.create_task_buffer({.initial_access = daxa::AccessConsts::VERTEX_SHADER_READ, .debug_name = "my task buffer"});
loop_task_list.add_runtime_buffer(task_buffer_id, buffer_id);
```

In TaskList we need "virtual" resources at record time, they are called TaskBufferId and TaskImageId. The reason for this is, that between executions the images and buffers might get recreated or reassigned. To avoid having to rerecord the whole task list, task list will take TaskResources instead which are backed by runtime resources on execution.

Here we create a `task_swapchain_image` to represent the swapchain image. We also create a TaskBuffer that represents our vertex buffer.

TaskList only needs to have virtual handles for resources involved in any synchronization. This means when you are sure that you only ever read from a resource in tasks, you will not need to mention them to task list. A common example of this would be to have a separated texture system that does its own synchronization and provides constant images that don't change. As these images never change they will never require any synchronization, they do not need to be mentioned in the main task list. 

We also bind a "read" Buffer and Image Id to the task `resources with add_runtime_buffer` and `add_runtime_image`. All task resources must have at least one runtime buffer/image on execution, but they may be changed between executions.

```cpp
loop_task_list.conditional({
    .condition_index = static_cast<daxa::u32>(TaskCondition::VERTICES_UPLOAD),
    .when_true = [&loop_task_list, &task_buffer_id, &task_condition_states]()
    {
        loop_task_list.add_task({
            .used_buffers = {
                {task_buffer_id, daxa::TaskBufferAccess::TRANSFER_WRITE},
            },
            .task = [&task_buffer_id, &task_condition_states](daxa::TaskRuntimeInterface task_runtime)
            {
                auto cmd_list = task_runtime.get_command_list();
                upload_vertex_data_task(task_runtime.get_device(), cmd_list, task_runtime.get_buffers(task_buffer_id)[0]);
                task_condition_states[static_cast<daxa::usize>(TaskCondition::VERTICES_UPLOAD)] = false;
            },
            .debug_name = "my upload task",
        });
    },
});
```

> Note: the function put into when_true will immediately be called in the conditional function and is not stored anywhere. This makes it safe to have reference capture in that lambda in any case.

As mentioned earlier we have a conditional for the first frame initialization. In this conditional, we upload the vertex buffer to the gpu.

We also see the first task here. Each task has a list of used task resources, in this case it is only the vertex buffer. It is important to mention the correct access to that resource within the task. 

Each resource can only have ONE use withing a task! If you need multiple different `TaskAccess`s to the same resource, you must split these accesses into multiple tasks!

This also applies to ImageSlices. There can never be overlap in the image slices in the accesses to an image.

Other then the list of used resources a task has a task callback which will be called at execution time and a debug name.

The task callback takes in a `daxa::TaskRuntimeInterface` which provides access to all needed resources like the device, command buffers etc.

The runtime can also provide the runtime resource of the virtual task resource ids at runtime with `daxa::TaskRuntimeInterface::get_buffers` and `daxa::TaskRuntimeInterface::get_images`, as seen above.

```cpp
loop_task_list.add_task({
	.used_buffers = {
		{task_buffer_id, daxa::TaskBufferAccess::VERTEX_SHADER_READ_ONLY},
	},
	.used_images = {
		{task_swapchain_image, daxa::TaskImageAccess::COLOR_ATTACHMENT, daxa::ImageMipArraySlice{}},
	},
	.task = [task_swapchain_image, task_buffer_id, &pipeline, &window_info](daxa::TaskRuntimeInterface task_runtime)
	{
		auto cmd_list = task_runtime.get_command_list();
		// Here we can just get the buffer and image IDs from the runtime
		// and just pass them directly.
		draw_to_swapchain_task(
			task_runtime.get_device(), cmd_list, pipeline,
			task_runtime.get_images(task_swapchain_image)[0],
			task_runtime.get_buffers(task_buffer_id)[0],
			window_info.width, window_info.height);
	},
	.debug_name = "my draw task",
});
```

We then unconditionally record a task to draw to the screen. The used resources here are the vertex buffer and the swapchain.
For used images you also need to provide the accessed image slice. Having this granularity allows daxa to understand subresource access synchronization for things like mip map generation or down-sampling.

```cpp
loop_task_list.submit({});
loop_task_list.present({});
loop_task_list.complete({});
```

Now we record submit, present and complete the list. As task list handles all synchronization, we DO NOT have to specify any semaphores here, not even for the swapchain!

> Note: If you want to inject your own semaphores or other synch in these commands, you can by providing pointers to vectors for the relevant primitives in the info `struct`s of these functions.
