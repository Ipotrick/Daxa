# TaskGraph

As Vulkan and Daxa require manual synchronization, using Daxa and Vulkan can become quite complex and error prone.

A common way to abstract and improve synchronization with low level APIs is the use of a render-graph. Daxa provides a render-graph called TaskGraph.

With TaskGraph you can create task resource handles and names for the resources you have in your program. You can then list a series of tasks.
Each task contains a list of used resources, as well as a callback to the operations the task should perform.

A core idea of TaskGraph (and other render graphs) is that you record a high-level description of a series of operations and execute these operations later. In TaskGraph, you record tasks, "complete" (compile) and later execute it. The callbacks in each task are called during execution.

This "two phase" design allows render graphs to optimize the operations not unlike how a compiler would optimize a program before execution. It also allows render graphs to automatically determine quite optimal synchronization, based on the declared resource uses in each task.
In addition, task graphs are reusable. You can, for example record your main render loop as a task graph and let task graph optimize the tasks only once and then reuse the optimized execution plan every frame. 
All in all, this allows for automatically optimized, low CPU cost synchronization generation.

Overview of the workflow for task graph:
* Create tasks
* Create task resources
* Add tasks to graph
* Complete task graph
* Execute task graph
* (optional) Repeatedly reassign resources to task resources
* (optional) Repeatedly execute task graph

# Task Resources

When construction a task graph its important to not use the real resource ids used in execution but virtual; representatives at record time. This has the simple reason that the task graph is reusable between executions. Making the reusability viable is only possible when resources can change between executions. To do this the graph takes virtual resources, TaskImage and TaskBuffer. ImageId and BufferIds can be assigned to these TaskImages and Taskbuffers and changed between executions of task graphs.

# Task

The core part about any render-graph is the nodes in the graph. In the case of Daxa these nodes are called tasks.

A task is a unit of work. It mainly consists of the used task resources (akin to parameters in a function) and a callback (akin to a function body). So one could think of a task as a meta function inside the graph.

To construct a task graph, you define a set of task resources (virtual representatives for the real resources when recording the graph) and a list of tasks using those task resources.

Each time you add a task, it is like a function call where you pass in the virtual task resources into the task's uses. Task graph will use the input tasks and resources to analyze and generate optimal execution order and synchronization for a given graph.

Importantly the graph works in two phases, the recording and the execution. The callbacks of tasks are only ever called in the execution of the graph, not the recording.

## Task Attachments

Tasks operate on buffers and images. In order for the task graph to isnert proper synchronization and task ordering it must know what resources are used and how they are used. Task Attachments are used to describe the used resources within a task. 

## Task Buffer/Image View

A task attachment is just a description of *how* a resource might be used. When adding a task to a graph, one must also set a *view* of a task resource to the attachments of each task. With the view and the attachment information, the graph is able to to its thing. 

A view is simply a task resource id + a slice of that resource.

## Task declarations

### Inline Tasks

There are two ways to declare a task. You can declare *inline tasks*, directly inside the add_task function:
```cpp
int blur_width = ...;
graph.add_task({
  .attachments = {
    daxa::TaskImageAttachment{.access = daxa::TaskImageAccess::TRANSFER_READ, .view=src},
    daxa::TaskImageAttachment{.access = daxa::TaskImageAccess::TRANSFER_WRITE, .view=dst},
  },
  .task = [=](daxa::TaskInterface ti)
  {
    // The views `dst` and `src` are used to access the runtime information for the attachments.
    copy_image_to_image(ti.recorder, ti.img(src).ids[0], ti.img(dst).ids[0], blur_width);
  },
  .name = "example task",
});
```
This is convenient for smaller tasks or quick additions that don't necessarily need shaders.

### Static Tasks

Usualy tasks are more complex then this. In general *static task declarations* should be prefered. An example of a *statically declared task*:

```cpp
struct MyTask : daxa::PartialTask<2 /*number of attachments*/, "MyTask" /*name of tasks*/>
{
  // The delcared indices can later be used to access the attachments via name directly.
  // add_attachment appends the given attachment info into an array of attachments.
  daxa::TaskImageAttachmentIndex const src = add_attachment(daxa::TaskImageAttachment{
    .access = daxa::TaskImageAccess::TRANSFER_READ,
  });
  daxa::TaskImageAttachmentIndex const dst = add_attachment(daxa::TaskImageAttachment{
    .access = daxa::TaskImageAccess::TRANSFER_WRITE,
  });
  int blur_width = {};
  virtual void callback(daxa::TaskInterface ti) const override
  {
    // Now TaskImageAttachmentIndex is used to access the images data instead of the view.
    copy_image_to_image(ti.recorder, ti.img(src).ids[0], ti.img(dst).ids[0], blur_width);
  }
};
```

The class `daxa::PartialTask` is inherited to declare a *static task*. The only two requirements to such a static task are to override the callback function and to *add the taks attachments*. As shown above this is done with the add_attachment function which is a derived member function from `daxa::PartialTask`. 

## Task Heads

It is also possible to declare a partial task, a so called *task head* within shared files with makros. This allows daxa to generate more code for you, reducing reduncancy.

An example of a task head:

```c
DAXA_DECL_TASK_HEAD_BEGIN(MyTaskHead, 2)
DAXA_TH_BUFFER_ID(COMPUTE_SHADER_READ,  daxa_BufferPtr(daxa_u32), src_buffer)
DAXA_TH_IMAGE_ID( COMPUTE_SHADER_WRITE, REGULAR_2D,               dst_image)
DAXA_DECL_TASK_HEAD_END
```

There are many more `DAXA_TH_BUFFER/IMAGE_XYZ` macros that serve slighly different functions. These are documented within the task graph inl header.

This task head declaration will translate to the following c++ code:

```cpp
struct MyTaskHead : daxa::PartialTask<2, "MyTaskHead">
{
  daxa::TaskBufferAttachmentIndex const src_buffer = add_attachment(daxa::TaskBufferAttachment{
    .access = daxa::TaskBufferAccess::COMPUTE_SHADER_READ,
  });
  daxa::TaskImageAttachmentIndex const dst_image = add_attachment(daxa::TaskImageAttachment{
    .access = daxa::TaskImageAccess::COMPUTE_SHADER_WRITE,
    .view_type = daxa::ImageViewType::REGULAR_2D,
  });
};
```

This partial task struct can now be derived to implement the virtual callback function and potentiall add more fields to the task:

```cpp
struct Task : MyTaskHead
{
  virtual void callback(daxa::TaskInterface ti) const override { ... }
}
```

### Task Head Blob

In order to transport the ids and ptrs of the attachments buffers and images to a shader, one must use a push constant. For that one declares a shared struct that is then filled and send on the host.

Task heads make this simpler. The macro `DAXA_TH_BLOB(TASK_HEAD)` declares a struct containing the given resource types within a shader, and a byte array within c++! It can be directly used within or even directly as a push constant:

```c
struct PushConstant
{
  DAXA_TH_BLOB(MyTaskHead) head;
};

```

In glsl the task head declares a struct containing ids/ pointers to the ressources. This struct can be named with the macro `DAXA_TH_BLOB(MyTaskHead)`. In c++ this macro will declare a byte array in place instead. This makes it possible to share struct containing this blob.

The task interface contains a span pointing to a prepared byte blob for each task that can be used to write the push constant `TaskInterface::attachment_shader_byte_blob`.

## Task Interface

Inside the tasks callback, the uses provide an interface to access the underlying resource at execution time as well as metadata about it.
For inline tasks, the uses can be retrieved with the task interface:
```cpp
  .task = [=](daxa::TaskInterface ti)
  {
    daxa::TaskImageAttachmentInfo const & img_attachment = ti.img(attachment);
  },
```

In ordet to access information on the attachments, the interface provides the functions `buf()`, `img()` and `attach()`. Each of them returns a `daxa::TaskBuffer/ImageAttachmentInfo`.

To refer to a given attachment in these functions one can use either a `TaskBuffer/ImageView` or a `TaskBuffer/ImageAttachmentIndex`.

The attachment info structs contain the previously set attachment data as well as runtime information such as the runtime `daxa::Buffer/ImageId`, `daxa::ImageViewId` and `daxa::ImageLayout`.

The rest of the task interface allows access to a transfer memory pool, a command recorder and the current device.

## Task Graph Valid Use Rules
- A task may use the same image multiple times, as long as the TaskImagView's slices dont overlap.
- A task may only ever have one use of a TaskBuffer
- All task uses must have a valid TaskResource or TaskResourceView assigned to them when adding a task.
- All task resources must have valid image and buffer ids assigned to them on execution.

