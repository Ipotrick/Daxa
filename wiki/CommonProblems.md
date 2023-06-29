# General Platform support:

* supports windows 10 and up
* supports most major linux versions (x11 and wayland)
* supports nvidia gpus from pascal up
* supports all amd gpus from RDNA-1 up

# List of bugs that can be encountered with daxa:
## NVIDIA DRIVER BUG: WINDOWS10 + BAR MEMORY + BDA + BDA CAPTURE DEBUG PLAYBACK + PASCAL AND LOWER ARCHITECTURES
when enabling -buffer device address, -buffer device address capture replay AND 
allocating ANY bar (base address register) memory (device local, host visible, host coherent) (for vma that is VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT),
will cause the driver to have random problems. 

Encountered issued are:
* All buffer modifications result in zeros in the buffer
* Vulkan objects loose their debug names
* Timeline semaphores, binary semaphores and fences randomly wont get signaled.

When on the mentioned platform, either avoid using any bar memory (buffers with mem flag HOST_ACCESS_SEQUENTIAL_WRITE_BIT), 
or debug your application on another device.

# Crashes
If you get a crash that has something to do with some VMA function not being loaded, the most likely culprit is your driver is out of date. We are using some pretty new features of Vulkan.

#### Cmake build errors
Make sure your vcpkg installation is up to date if you already had vcpkg installed before.

# Shaders

Make sure you define DAXA_ENABLE_BUFFER_PTR on all `struct`s you use in buffer pointers.

make sure to always dereference the pointers to access the fields of the pointed too value.

When using buffer pointers, if you pass the wrong type to a function, it will perform an implicit cast
```glsl
void a(daxa_BufferPtr(daxa_u32) my_buffer)
{
    // ...
}

void b(daxa_RWBufferPtr(daxa_u32) my_buffer)
{
    a(my_buffer);
}
```
This can cause a crash in the NVIDIA driver for unknown reasons. Most of the time, explicitly casting will do the trick
```glsl
void b(daxa_RWBufferPtr(daxa_u32) my_buffer)
{
    a(daxa_BufferPtr(daxa_u32)(my_buffer));
}
```
