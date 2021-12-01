## Home of the DAXA application-/gameengine.

Daxa is a simple Game Engine build with C++ and Vulkan.

Daxa exposes its own graphics api (in the gpu namespace) that is build on top of vulkan.
Design goals for the gpu abstaction:
 * hard to missuse
 * fast to get things done
 * automatic (ref counted) lifetime management of common types like buffers
 * high performance and low overhead on high-frequency commands
 * thread save parallel command encoding
 * beeing relatively close to vulkan in spirit (things like descriptor sets are still exposed in an abstracted way)
 * beeing a lot less explicit than vulkan and having default values for everything

The Engine also makes sue of multithreadding via a threadpool. This threadpool will be expaneded to also utalize co-routines so that asyncronous operations can be executed.

The Engine has minimal dependencies, all major systems (like renderer, ecs and math library) are self written, as this engine is made for learning purposes and a tool to explore engine design.

Outside of the Daxa-Folder there will be many examples and Demos utalizing the Daxa engine.
