# TaskList

As vulkan and daxa require manual synchronization, using daxa and vulkan can become quite complex and error prone.

A common way to abstract and improve synchronization with low level apis is the use of a rendergraph. Daxa provides a rendergraph called TaskList.

With TaskList you can create task resource handles and names for resources you have in your program. You can then list a series of tasks.
Each task contains a list of used resources and a callback to the operations the task should perform.

A core idea of task list and other rendergraphs is, that you record a high level description of a series of operations and execute these operations later. In task list, you record tasks, then complete and later execute it. The callbacks in each task are called during execution.

This "two phase" design allows rendergraphs to optimize the operations not unlike how a compiler would optimize a program before execution. It also allows rendergraphs to automatically determine quite optimal synchronization, based on the declared resource uses in each task.
In addition, task lists are reusable. You can, for example record your main render loop as a task list and let task list optimize the tasks only once and then reuse the optimized execution plan every frame. 
All in all, this allows for automatically optimized, low cpu cost synchronization generation.

# Quick Overview
Steps of working with a task list:
* Create task resources
* Create task list
* Record tasks into list
* Complete task list
* Execute task list

Here the task list has two phases, the record and exectue phases. In the record phase it is mutable and can record commands. After completing it can no longer record new commands. Completing a list compiles and optimizes tasks and their sync. When completed a list can be executed. A list can be executed multiple times and reused.

# Task Resources

In daxa i want task lists to be reusable. This means recording the list once and then executing the compiled list multiple times. This is very beneficial, as the recording and completion costs cpu time. 

If we would use the plain resource ids we would have a problem achieving high reusability. This is because after completing a list its contents are constant, so all resource ids are backed and cant change. This forces us to rerecord any list that can change its resource ids between executions. 

As this makes it very difficult to reuse lists daxa uses "fake" handles that represent resources used in task lists. What resources these task resources actualy represent can be changed between execuctions. This allows you to reuse task lists in many more cases.

Another reason for these virtual ids is that tracking of resource state between task list executions is possible. This allows you to use resources in multiple task lists in any order freely. This is possible as daxa can track the state the resources are in and can correcttly transition states between lists.

An example for this is recording a list for most of the rendering and accessing the swapchain. The swapchain image changes every frame, so we must make sure that we must have a placeholder handle for the swapchain at record time to be able to reuse the same list without recreating it every frame.

There are only two shader resource types that require any synchronization in daxa, buffers and images. There are two corresponding types in task list: `daxa::TaskBuffer` and `daxa::TaskImage`. 

These act as placeholders for the resources that are used in the list at execution when recording the list. What resources they represent can be set either in creation of the task resource with the fields `initial_buffer`, `initial_images` or set with the functions `set_buffers` and `set_images`. Each task resource can actualy represent multiple resources at once! This is useful as it may be that some resources have the exact same access patter. Using one task handle to represent multiple resources makes the recording more efficient and more readable.

As said earlier the represented buffers and images can be set between executions of task lists that access said resources. So for example a task image representing a swapchain could get a new `ImageId` from the swapchain every frame before executing the task list.

## Handles

Task uses take handles as parameters to identify what part of a resource they actually use.

Task resources implicitly cast to handles. A task handle is used to identify a part of a task resource. In the case of a buffer it is just identifying the whole buffer. But for images it identifies a slice of the image. 

This is very useful when declaring that a task only uses a part of an image. Daxa can use this information to generate more optimal synchronization. 

Another example where this is very useful is mip mapping. In mip mapping you would have two uses on the same image that specify different slices of that image. This is easily achieved with handles.

# Task

A task represents a small portion of your program that does not require any synchronization within it. For example a part of a renderer that would write and then read to an image in two different pipelines will require a pipeline barrier. This means these two parts of the renderer must be in different tasks. Any task must only have disjoint and unique accessing to any particular resource.

As daxa must infer synchronization and ordering requirements from tasks, you must declare all resources that are accessed witing the task. Note that this excludes all purely constant resources, as those have no synchronization requirements. 

So a task in task list might look like this:
```cpp
using namespace daxa::task_resource_uses;
list.add_task({
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

Tasks can also optionally have a name. This name is used in error messages in task list.

## Task uses

TODO

## Task interface

TODO