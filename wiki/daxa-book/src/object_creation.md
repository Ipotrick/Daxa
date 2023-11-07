# Initialization

For nearly all functions, daxa uses structs as parameters. They follow the naming convention of `<Thing>Info`. 

There are several big advantages of struct parameters in conjunction with c++20 designated initialization:
* default function parameters
* out of order default function parameters
* named function parameters

Here is an example of the creation of a `daxa::Instance`:

```c++
daxa::Instance instance = daxa::create_instance(daxa::InstanceInfo{
    .app_name = "example instance",
});
```

InstanceInfo looks like this:
```c++
struct InstanceInfo
{
    InstanceFlags flags =
        InstanceFlagBits::DEBUG_UTILS |
        InstanceFlagBits::PARENT_MUST_OUTLIVE_CHILD;
    SmallString engine_name = "daxa";
    SmallString app_name = "daxa app";
};
```

As you can see, all fields have default values. In the example above we only initialize app_name. This reduces the boilerplate of initialization. Daxa also infers as much as possible for vulkan object creation, making the info structs smaller and initialization easier.