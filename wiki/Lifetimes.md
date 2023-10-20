# Objects

Daxa splits all objects into two categories. shader resource objects and management objects.

Shader resource objects are buffers, images, image views and samplers. 

Shader resource objects are represented with 64bit ids in daxa. These ids can be used on the cpu AND gpu! The ids are similar to entity ids in an ecs.

Any other obejct is reference counted. 

# ids vs reference counting

Why have two different lifetime systems in daxa?

In the past daxa had only ref counted objects. This was a great convenience, ref counting is very simple, solves all lifetime tracking issues itself and makes use and implementation very easy.

The downsides of reference counting are performance and unclear lifetimes. 

It is very easy to forget about a reference somewhere and start to hog a ton of memory accidentally by never releasing temporary resources.

Daxa tracks all used objects in command recording for validation. The overhad of using ref counting for this is around 5-15%! On top of that the ref counting can cause perf problems on the user side too.

Object types that are high in numbers per application make the issues of ref counting much worse. 

In opposition to that is using ids for onject references. These have the upside that they are trivial to copy and have complely rigit lifetimes. 

The downside is that they require some extra checking, this checking is VERY cheap on average, it is far less then a ref count increment.

Daxa strikes a compromise. "low frequency" objects, objects that are in relatively low numbers, have unintresting lifetimes and or are not passed around often are refcounted for convenience.
Any object that typically appears in very high frequency, (shader resource objects) are managed by ids.

# Parent child dependencies

Per default daxa will track parent child dependencies between objects and keep the parents alive until all the children die.

This can be very convenient but also makes the problem of unclear lifetimes even worse.

So daxa also has an optional instance flag (`DAXA_INSTANCE_FLAG_PARENT_MUST_OUTLIVE_CHILD`) that forces parents to outlive their children, the children will NOT keep parents alive but ERROR.

# Deferred destruction - Zombies??

When an objects reference count drops to 0 or gets manually destroyed, it is not actually destroyed yet it is zombiefied.

A zombie object is no longer usable on the user side. But zombies are still valid in daxas internals as well as on the gpu.

Daxa defers the destruction of zombies until the gpu catches up with the cpu at the time of zombiefication.

The real object destructions exclusively happen in `Device::collect_garbage`. This function will check all zombies zombification time point, compare it with the gpu timeline and if the gpu cought up, it will destroy the zombie.

> Note: the functions `Device::submit_commands` and `Device::present` as well as any non-completed `CommandList`s hold a shared read lock on object lifetimes. `Device::collect_garbage` will lock the object lifetimes exclusively, meaning it will block until all shared locks get unlocked! Make sure to not have open command lists while calling `Device::collect_garbage`.