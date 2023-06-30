# TaskGraph

As vulkan and daxa require manual synchronization, using daxa and vulkan can become quite complex and error prone.

A common way to abstract and improve synchronization with low level apis is the use of a rendergraph. Daxa provides a rendergraph called TaskGraph.

With TaskGraph you can create task resource handles and names for resources you have in your program. You can then list a series of tasks.
Each task contains a graph of used resources and a callback to the operations the task should perform.

A core idea of task graph and other rendergraphs is, that you record a high level description of a series of operations and execute these operations later. In task graph, you record tasks, then complete and later execute it. The callbacks in each task are called during execution.

This "two phase" design allows rendergraphs to optimize the operations not unlike how a compiler would optimize a program before execution. It also allows rendergraphs to automatically determine quite optimal synchronization, based on the declared resource uses in each task.
In addition, task graphs are reusable. You can, for example record your main render loop as a task graph and let task graph optimize the tasks only once and then reuse the optimized execution plan every frame. 
All in all, this allows for automatically optimized, low cpu cost synchronization generation.

# Quick Overview
Steps of working with a task graph:
* Create task resources
* Create task graph
* Record tasks into graph
* Complete task graph
* Execute task graph

Here the task graph has two phases, the record and exectue phase. In the record phase the task graph is mutable, new tasks can be added to the graph. After completing a graph, it can no longer record new tasks. Completing a graph is an important step, as it "compiles" the graph. In the task graph compilation, the tasks are analyzed for dependencies, reordered for optimal barrier placement and optimized sync between tasks are generated next to the optimized execution plan. 

Importantly completed task graphs can be reused! All resources within a task graph are virtualized (as seen later), meaning you can change the represented image/buffer for a task image/buffer between executions of a completed task graph (for example change the swapchain image or a temporal resource).
Re-using completed task graphs can significantly reduce cpu overhead by avoiding redundant computations every frame.

# Task Resources

Task graph has virtual resource handles (`TaskImageId` and `TaskBufferId`. This is important for the following reasons.

If we would use the plain resource ids we would have a problem achieving high reusability. This is because after completing a graph its contents are constant, so all resource ids are backed and cant change. This forces us to rerecord any graph that can change its resource ids between executions. 

As this makes it very difficult to reuse graphs daxa uses "fake" handles that represent resources used in task graphs. What resources these task resources actualy represent can be changed between execuctions. This allows you to reuse task graphs in many more cases.

Another reason for these virtual ids is that tracking of resource state between task graph executions is possible. This allows you to use resources in multiple task graphs in any order freely. This is possible as daxa can track the state the resources are in and can correcttly transition states between graphs.

An example for this is recording a graph for most of the rendering and accessing the swapchain. The swapchain image changes every frame, so we must make sure that we must have a placeholder handle for the swapchain at record time to be able to reuse the same graph without recreating it every frame.

There are only two shader resource types that require any synchronization in daxa, buffers and images. There are two corresponding types in task graph: `daxa::TaskBuffer` and `daxa::TaskImage`. 

These act as placeholders for the resources that are used in the graph at execution when recording the graph. What resources they represent can be set either in creation of the task resource with the fields `initial_buffer`, `initial_images` or set with the functions `set_buffers` and `set_images`. Each task resource can actualy represent multiple resources at once! This is useful as it may be that some resources have the exact same access patter. Using one task handle to represent multiple resources makes the recording more efficient and more readable.

As said earlier the represented buffers and images can be set between executions of task graphs that access said resources. So for example a task image representing a swapchain could get a new `ImageId` from the swapchain every frame before executing the task graph.

## Handles

In essence handles are simply a TaskResourceId + a subresource range of the given id. 

All tasks take handles as parameters to their uses and not ids themselves. This is because with handles you can specify a more specific part of a resource, like a mip level or array layer of an image.

Task resources implicitly cast to handles. A task handle is used to identify a part of a task resource. In the case of a buffer it is just identifying the whole buffer. But for images it identifies a slice of the image. 

An example where this is very useful is mip mapping. In mip mapping you would have two uses on the same image that specify different slices of that image. This is easily achieved with handles.

# Task

The core part about any rendergraph is the nodes in the graph. In the case of daxa these nodes are called tasks.

Each task as a set of used resources (the tasks uses), a callback that describes rendering operations on those resources at execution time and a name.

Each task therefore is practically describing a meta function as a part of a renderer. The uses beeing the function parameters and the callback beeing the function body.

To construct a task graph, you define a set of tasks, each describing a small portion of your renderer.

You then record a high level describtion of your renderer in form of a task graph. To record a task graph you add tasks, to extend the graph.

Each time you add a task it is like a function call where you pass in the virtual task resources into the tasks uses. Task graph will use the input tasks and resources to analyze and generate optimal execution order and synchronization for a given graph.

There are two ways to declare a task. You can declare tasks inline, directly inside the add_task function:
```cpp
using namespace daxa::task_resource_uses;
graph.add_task({
  .uses = {
    ImageTransferRead{task_image0},
    ImageTransferWrite{task_image1},
  },
  .task = [=](daxa::TaskInterface ti)
  {
    auto cmd = ti.get_command_list();
    // perform some transfer operation
  },
  .name = "example task",
});
```
This is convenient for smaller tasks or quick additions that dont nessecarily need shaders.

And the second way:
TODO

Tasks can also optionally have a name. This name is used in error messages in task graph.

## Task uses

TODO

## Task interface

TODO
