# Device

The device in daxa is essentially a `VkDevice` in vulkan but it carries a lot more responsibility.

## Grahics Queue

As all applications that would use daxa will have a gpu with a graphics queue `daxa::Device` also represents the graphics queue. This is not a problem in terms of multi queue as no modern gpu has multiple hardware graphics queues, there is no benefit in having multiple `VkQueue`s of the graphics queue family. 

So in daxa, instead of choosing creating and then using a `VkQueue` for graphics command submission you use the device directly.
Example of using the device for submitting commands:
```c++
daxa::Device device = ...;
daxa::ExecutableCommandList cmd_list = ...;
daxa::BinarySemaphore sema = ...;
daxa::TimelineSemaphore tsema = ...;
uint64_t tsema_value = ...;
device.submit_commands({
    .command_lists = std::array{std::move(cmd_list)},
    .wait_binary_semaphores = std::array{sema},
    .signal_timeline_semaphores = std::array{std::pair{tsema, tsema_value}},
});
```

This generally reduces boilerplate and turns out to be a convenient merge of the `VkDevice` and `VkQeueue` concept.

> Note that daxa will support async compute and async transfer queues in the future. As noone requested these up to this point daxa does not implement them.

Aside from submitting commands `daxa::Device` is also responsible for presenting frames to a swapchain:

```c++
daxa::Device device = ...;
daxa::Swapchain swapchain = ...;
daxa::BinarySemaphore sema = ...;
device.present_frame({
    .wait_binary_semaphores = std::array{sema},
    .swapchain = swapchain,
});
```

## Collecting Garbage

As mentioned in the lifetimes section, daxa defers destructions of objects until collecting garbage. 

```c++
daxa::Device device = ...;
device.collect_garbage();
```
This is a very important function. It scans throu the array of queued object destructions, it checks if the gpu cought up to the point on the timeline where the resource was "destroyed". If the gpu cought up, they get destroyed for real, if not they stay in the queue. It is advised to call it once per frame in the end of the render loop. 

> Note that collect garbage is called automatically on device destruction, do not worry about potential leaking here!

This function has another important caveat! It "locks" the lifetimes of SROs. What this means is that you may NOT record commands for this device in parallel to calling collect garbage. This is not a limitation in typical applications.

> Note if fully async command recording is truely nessecary, I have plans for "software" command recorders that will work fully parallel with collect garbage among other features.

Daxa will not error if you try to record commands at the same time as collecting garbage. Instead it will simply block the thread. Each existing `daxa::CommandRecorder` holds a read lock on resources and calling collect garbage will aquire an exclusive write lock.

This is nessecary to ensure efficiency of validation in command recording, consider this case of daxas internals when recording a command:

```c++
auto daxa_cmd_copy_buffer_to_buffer(
    daxa_CommandRecorder self, 
    daxa_BufferCopyInfo const * info) 
-> daxa_Result
{
    ...
    // We check the validity of the ids here.
    _DAXA_CHECK_AND_REMEMBER_IDS(self, info->src_buffer, info->dst_buffer)            
    // After we checked the ids another thread could destroy the used buffers
    // and call collect garbage, fully deleting the data for the buffers internally.
    // This would cause the ids to dangle, we would read garbage past here.
    auto vk_buffer_copy = std::bit_cast<VkBufferCopy>(info->src_offset);    
    vkCmdCopyBuffer(
        self->current_command_data.vk_cmd_buffer,
        // We read the internal data of the buffers here.
        self->device->slot(info->src_buffer).vk_buffer,                                 
        self->device->slot(info->dst_buffer).vk_buffer,
        1,
        vk_buffer_copy);
    return DAXA_RESULT_SUCCESS;
}
```

By locking the lifetimes on `daxa::CommandRecorder` creation and unlocking when they get destroyed we only need to perform two atomic operations per command recorder instead of per recorded command.

## Metadata

The device stores all kinds of queriable metadata of the `VkPhysicalDevice` as well as itself and its SROs.

These can all be queried with device member functions. Here are some examples:
```c++
daxa::BufferId buffer = ...;
daxa::Device device = ...;
daxa::DeviceInfo const & device_info = device.info();
daxa::BufferInfo const & buffer_info = device.info_buffer(buffer);
daxa::DeviceProperties const & properties = device.properties();
```