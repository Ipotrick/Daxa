# Resource Lifetimes in Daxa

Daxa resources are reference counted and when dropped, their destuction is automatically deferred until the gpu is done with current work.

A user of daxa does not need to be concerned about lifetimes in general, the system is quite hard to misuse.
But for some, it is important to know what exactly daxa is doing for resource lifetimes, especially for those that want to write a wrapper in their own language, binding the c api.

## Reference Counters

All objects in daxa have a strong and a weak reference count. 
Technically not all resource need both a strong and weak counter, but for simplicity and forward compatibility, daxa provides them for all resources.

When an objects is created, it is returnd to the user with a strong count of 1 and a weak count of 0. 

The strong count is used to express an owning relationship. The c api provides functions to in and decrement this strong count, expressing owning handles.
To end the lifetime of an object, simply decrement the strong count to 0.

The weak count is used to track implicit dependencies between objects internally. 

For example a BinarySemaphore has an implicit parent->child relationship with Device. 
Creating a BinarySemaphore will increment the weak count of the device by one, when the semaphore is destroyed the weak count of the device will be decremented.

This weak count can be used in two modes. 

As a default daxa will treat these weak references as owning, meaning weak counts will keep objects alive.
This can be quite useful and makes the api simpler and easier to use, for example accidentally destroying an image before an image view pointing to it is completely fine in this mode.
The image view will keep the image alive, as it incremented the weak count to the image on its creation. When the image view ends its lifetime it will decrement the weak count, ending the lifetime of its parent image.

The secondary mode `PARENTS_MUST_OUTLLIVE_CHILDREN` makes daxa treat weak references as a simple debug utility, ensuring that parents outlive children.
So when an image is destroyed before all views to it are destroyed, the strong count to the image is 0 but the weak count is > 0. In this mode daxa will error as a child outlived a parent.
This mode is very useful if the user does NOT want objects to linger around potentially hogging memory and other resources. This way daxa will find any and all dependency management errors the user might do with daxa objects.

> There is one exception to this, the gpu shader resources (buffer, image, image view, sampler) MUST always outlive the parent (device), as the device is required to destroy these resources. They can never outlive the device.

This mode can be set as an flag when creating a daxa instance.

> In the c++ api these reference counters are automatically managed by the c++ wrapper types. 

## Deferred destruction

In daxa and vulkan it is possible for the gpu to run asynchronously with the cpu.

This has lifetime implications for all gpu objects held by daxa and vulkan. For example a buffer must not be destroyed while the gpu is executing commands that potentially use this buffer.

There are multiple ways to avoid this problem, in daxa i decided to use a timeline based destruction model.

This model allows the user to treat the lifetime of resources as if the gpu would always be in sync with the cpu (but have them actually work asynchonously), making lifetimes much simpler to manage.

There is no need to wait on semaphores or fences to ensure proper destruction, daxa does that all for you.

### How does it work?

The daxa device holds the main graphics queue, a cpu timeline value and a gpu timeline value.

On a submit to the device the cpu timeline value is incremented, and when the gpu execution finishes, the gpu timeline value will be incremented as well.

With these two values (cpu and gpu timeline) we can determine how much the gpu is behind the cpu. 

In daxa, when an object drops to 0 references, instead of immediately destoying it, it becomes a zombie. 

Zombies "stay alive" until the gpu catches up to the cpu timeline at the point of zombiefication of that object.

This ensures that all currently submitted commands can still use the resource until it is done.

When an object becomes a zombie they get the current cpu timeline value as a marker. 

In the `collect_garbage` function, a daxa device reads the current cpu and gpu timeline values, then goes throu all zombies.

When a zombies marker timeline value is greater or equal to the gpu timeline value, that means that the gpu is done executing all commands that were submitted at the time of the zombiefication of that object.

Hence we can safely destroy the zombie at that point.

Doing this daxa batches all resource destructions by this zombification mechanism.