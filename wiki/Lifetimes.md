# Resource Lifetimes in Daxa

Daxa resources are reference counted and when dropped, their destuction is automatically deferred until the gpu is done with current work.

A user of daxa does not need to be concerned about lifetimes in general, the system is quite hard to misuse.
But for some, it is important to know what exactly daxa is doing for resource lifetimes, especially for those that want to write a wrapper in their own language, binding the c api.

## Reference Counters

Nearly all daxa objects have an internal reference count. These reference counters are accessable with the c api, the c++ api simply abstracts the refcounting with RAII shared_ptr-like types. The reference counters are meant to count owning references, held by the user. When an object is created, its reference count is 1, when the reference count is decremented to 0 by the user, the object will be zombiefied (more on that later).

> The reference counting is atomic and threadsafe.

Daxa also keeps a secondary weak reference count for most objects. This count is used to count parent-child dependencies between objects. For example creating a binary semaphore is the child of a device, creating one will increment the weak count of the parent device. Per default daxa will use the weak count to keep parents alive if a child dies before its parent in order to ensure proper destruction order. This is very convenient but may lead to unclear resource lifetimes. 

// TODO: Fix inconsistency in parent child relations for memory block.

> Note: To be sure on when an object is a child to another object, check out the functions creating the objects. When an object has a function to create another one it is a child of the other one. For example an ImageView is not a parent to an image but to the device.

> Note: buffer, image(-view) and sampler are excluded from this. They MUST be destroyed before the device as a user side reference is nessecary to destroy those objects at all!

// TODO: Implement better error messaging for this mode!

But daxa has an optional secondary lifetime mode in which it is illegal for children to outlive their parents. This secondart mode makes it nessecary to have two different counters as daxa needs to be able to differentiate the user reference counts and the child-parent dependency reference counts. In this mode daxa will panic if one tries to destroy a parent before having destroyed all its children. This mode is prefered by many, as it will force the user to use clear object lifetimes and daxa will detect any errors. This mode is instance wide and can be enabled with the instacee flag `DAXA_INSTANCE_FLAG_PARENT_MUST_OUTLIVE_CHILD`.

The secondary mode `PARENTS_MUST_OUTLLIVE_CHILDREN` makes daxa treat weak references as a simple debug utility, ensuring that parents outlive children.
So when an image is destroyed before all views to it are destroyed, the strong count to the image is 0 but the weak count is > 0. In this mode daxa will error as a child outlived a parent.
This mode is very useful if the user does NOT want objects to linger around potentially hogging memory and other resources. This way daxa will find any and all dependency management errors the user might do with daxa objects.

## Deferred destruction - Zombies??

As mentioned above, when the reference count of an object drops to zero it is NOT directly destroyed. They become a "zombie". A zombie is not usable by the user anymore. Daxa keeps zombies around because the gpu could potentially still use these objects when the reference count dropped to zero! In order to prevent destroying objects before the gpu is done using them daxa keeps the nessecary parts of an object alive as a zombie until it is safe to really destroy the resources.

Daxa keeps a cpu and gpu timeline to measure how much the gpu is behind the cpu. When a zombie is created it gets assigned the current cpu timeline value. When collect garbage is called on the device, it will check if the gpu reached the zombies cpu timeline value. If so it destroyes them, as the gpu reached past the point in time where the object became a zombie!

> Note: collect garbage can be manually called, but it is also automatically called in submit.