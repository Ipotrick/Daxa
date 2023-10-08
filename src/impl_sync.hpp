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
    daxa_Device device = {};
    BinarySemaphoreInfo info = {};
    std::string info_name = {};
    VkSemaphore vk_semaphore = {};
};

struct daxa_ImplTimelineSemaphore final : daxa_ImplHandle
{
    daxa_Device device = {};
    TimelineSemaphoreInfo info = {};
    std::string info_name = {};
    VkSemaphore vk_semaphore = {};
};

struct daxa_ImplEvent final : daxa_ImplHandle
{
    daxa_Device device = {};
    EventInfo info = {};
    std::string info_name = {};
    VkEvent vk_event = {};
};