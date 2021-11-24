Daxa is a simple Game Engine build with C++ and Vulkan.

It has a complete abstraction layer for vulkan, reducing the complexity of writing graphics code dramaticly, while still utalizing the big performance benefits of vulkan.

The Abstraction is still very thin and allows for very high performance, things like descriptor indexing are completely exposed and easily implemented.

With this abstraction layer i build a renderer, wich is general in design and allows for the creation of many rendering tecniques.

The Engine also makes sue of multithreadding via a threadpool. This threadpool will be expaneded to also utalize co routines so that asyncronous operations can be executed.

The Engine has minimal dependencies, all mayor systems (like renderer, ecs and maths library) are self written, as this engine is made for learning purposes and a tool to explore engine design.
