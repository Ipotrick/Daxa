# This repo is the home of the Daxa engine

Daxa is a small library, that is used to make renderers and games.

It provides it's own gpu abstraction library (daxa::gpu) that is build on vulkan

Goals for daxa::gpu are:

* Beeing easy to set up
* automatic reference counted lifetime management of most types
* low overhead
* less redundancy, it uses shader reflection for example
* abstracting less usefull concepts like framebuffers and renderpasses
