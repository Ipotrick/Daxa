#pragma once

#include "impl_core.hpp"
#include <daxa/c/device.h>
#include <daxa/c/sync.h>

struct SemaphoreZombie
{
    VkSemaphore vk_semaphore = {};
};

struct EventZombie
{
    VkEvent vk_event = {};
};

struct daxa_ImplBinarySemaphore final : daxa_ImplHandle
{
    daxa_BinarySemaphoreInfo info = {};
    std::string info_name = {};
    daxa_Device device = {};
    VkSemaphore vk_semaphore = {};
    static auto create(daxa_Device device, daxa_BinarySemaphoreInfo a_info) -> daxa_BinarySemaphore;
};

struct daxa_ImplTimelineSemaphore final : daxa_ImplHandle
{
    daxa_TimelineSemaphoreInfo info = {};
    std::string info_name = {};
    daxa_Device device = {};
    VkSemaphore vk_semaphore = {};
    static auto create(daxa_Device device, daxa_TimelineSemaphoreInfo a_info) -> daxa_TimelineSemaphore;
};

struct daxa_ImplEvent final : daxa_ImplHandle
{
    daxa_EventInfo info = {};
    std::string info_name = {};
    daxa_Device device = {};
    VkEvent vk_event = {};
    static auto create(daxa_Device device, daxa_EventInfo a_info) -> daxa_Event;
};