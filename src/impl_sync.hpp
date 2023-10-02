#pragma once

#include "impl_core.hpp"
#include "impl_device.hpp"
#include <daxa/c/sync.h>

struct SemaphoreZombie
{
    VkSemaphore vk_semaphore = {};
};

struct EventZombie
{
    VkEvent vk_event = {};
};

struct daxa_ImplBinarySemaphore final : daxa::ManagedSharedState
{
    daxa_BinarySemaphoreInfo info = {};
    std::string info_name = {};
    daxa_Device device = {};
    VkSemaphore vk_semaphore = {};
    static auto create(daxa_Device device, daxa_BinarySemaphoreInfo a_info) -> daxa_ImplBinarySemaphore;
};

struct daxa_ImplTimelineSemaphore final : daxa::ManagedSharedState
{
    daxa_TimelineSemaphoreInfo info = {};
    std::string info_name = {};
    daxa_Device device = {};
    VkSemaphore vk_semaphore = {};
    static auto create(daxa_Device device, daxa_TimelineSemaphoreInfo a_info) -> daxa_ImplTimelineSemaphore;
};

struct daxa_ImplEvent final : daxa::ManagedSharedState
{
    daxa_EventInfo info = {};
    std::string info_name = {};
    daxa_Device device = {};
    VkEvent vk_event = {};
    static auto create(daxa_Device device, daxa_EventInfo a_info) -> daxa_ImplEvent;
};