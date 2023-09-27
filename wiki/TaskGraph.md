# TaskGraph

As Vulkan and Daxa require manual synchronization, using Daxa and Vulkan can become quite complex and error prone.

A common way to abstract and improve synchronization with low level APIs is the use of a render graph. Daxa provides a render graph called TaskGraph.

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

## Task Resource Views

Many times it can be quite convenient to refer to only a part of an image or buffer. For example to specify specific mip levels in an image in a mip map generator.

For this purpose daxa has TaskImageViews. A TaskImageView similarly to an ImageView contains of a slice of the TaskImage, specifying the subresource.

All Tasks take in views instead of the resources themselves. Resources implicitly cast to the views, but also have the explicit conversion function `.view()`. Views themselves also have a `.view()` function to create a new view from an existing one.

# Task

The core part about any render graph is the nodes in the graph. In the case of Daxa these nodes are called tasks.

A task is a unit of work. It mainly consists of the used task resources (akin to parameters in a function) and a callback (akin to a function body). So one could think of a task as a meta function inside the graph.

To construct a task graph, you define a set of task resources (virtual representatives for the real resources when recording the graph) and a list of tasks using those task resources.

Each time you add a task, it is like a function call where you pass in the virtual task resources into the task's uses. Task graph will use the input tasks and resources to analyze and generate optimal execution order and synchronization for a given graph.

Importantly the graph works in two phases, the recording and the execution. The callbacks of tasks are only ever called in the execution of the graph, not the recording.

There are two ways to declare a task. You can declare tasks inline, directly inside the add_task function:
```cpp
using namespace daxa::task_resource_uses; // For ImageTransfer(Read|Write)
daxa::TaskImage src = ...;
daxa::TaskImage dst = ...;
int blur_width = ...;
graph.add_task({
  .uses = {
    ImageTransferRead<>{src},
    ImageTransferWrite<>{dst},
  },
  .task = [=](daxa::TaskInterface ti)
  {
    auto cmd = ti.get_command_list();
    copy_image_to_image(ti.uses[src].image(), ti.uses[dst].image(), blur_width);
  },
  .name = "example task",
});
```
This is convenient for smaller tasks or quick additions that don't necessarily need shaders.

And the general way, the persisstent declaration:

```cpp
struct MyTask
{
  // The 'uses' field will be statically reflected by daxa. 
  struct Uses {
    ImageTransferRead<> src;
    ImageTransferWrite<> dst;
  } uses;  
  // The 'name' will be statically reflected by daxa. 
  static constexpr std::string_view name = "example task"; 
  int blur_width = {};
  void callback(daxa::TaskInterface ti)
  {
    auto cmd = ti.get_command_list();
    copy_image_to_image(uses.src.image(), uses.dst.image(), blur_width);
  }
};
```

Daxa uses limited static reflection to analyze task structs with templates and concepts. The Uses field must be called uses and the struct type needs to be called Uses as well.
Optionally daxa can also reflect the name field to give the task a name.

## Task Resource Uses

The use of a resource is declared with either a TaskImageUse or a TaskBufferUse. Daxa predefines a set of shortened names for these uses under the namespace `daxa::task_resource_uses`. It is adivsed to use these.

A declared resource use describes a pipeline stage, the access (read and/or write) and for images optionally an image view type.
Uses may be predeclared for persistent task structs or listed in a vector for inline tasks.

When a task is added to the graph, a resource must be assigned to each use of the added task.
For inline tasks these are immediately assigned as they are listed in the uses. For persistent resources, the uses struct must be instantiated, then all resources asigned to the uses and finally passed to the add task function.

Inside the tasks callback, the uses provide an interface to access the underlying resource at execution time as well as metadata about it.
For inline tasks, the uses can be retrieved with the task interface:
```cpp
  .task = [=](daxa::TaskInterface ti)
  {
    ImageTransferRead<> const & img_use = ti.uses[src];
  },
```
While with persistent tasks, the uses are stored in the `Uses` struct and can be directly accessed in the callback:
```cpp
  void callback(daxa::TaskInterface ti)
  {
    ImageTransferRead<> const & img_use = uses.src;
  },
```
ImageUse's provide a runtime interface:
```cpp
auto access()             const -> TaskImageAccess;
auto view_type()          const -> ImageViewType;
auto image(u32 index = 0) const -> ImageId;
auto view(u32 index = 0)  const -> ImageViewId;
auto layout()             const -> ImageLayout;
```
If the slice and image view type of the use do not fit the default view, daxa will create and cache image views that exactly match the image view type and slice of the use.
As the layout can change at any time between tasks it is hard to know in what layout the image slice of the use currently is.
Daxa allows you to query the image layout via the interface. The image layout is guaranteed to stay the same for the given use-slice for the duration of the task on the gpu timeline. 

BufferUse's have a similar interface:
```cpp
auto access() const -> TaskBufferAccess;
auto buffer(usize index = 0) const -> BufferId;
```

With this interface, you can query all nessecary information about the images and buffers inside the callbacks with a similar interface for inline and persistently declared tasks.

## TaskInterface

The task interface is the main way to interact with the task graph from within a task callback.

It provides access to task uses, command lists, a linear memory allocator, and a few other graph related resources:

```cpp

```

## Task Graph Valid Use Rules
- A task may use the same image multiple times, as long as the TaskImagView's slices dont overlap.
- A task may only ever have one use of a TaskBuffer
- All task uses must have a valid TaskResource or TaskResourceView assigned to them when adding a task.
- All task resources must have valid image and buffer ids assigned to them on execution.

