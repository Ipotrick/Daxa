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

struct daxa_ImplBinarySemaphore final : ImplHandle
{
    daxa_Device device = {};
    BinarySemaphoreInfo info = {};
    VkSemaphore vk_semaphore = {};

    static void zero_ref_callback(ImplHandle const * handle);
};

struct daxa_ImplTimelineSemaphore final : ImplHandle
{
    daxa_Device device = {};
    TimelineSemaphoreInfo info = {};
    VkSemaphore vk_semaphore = {};

    static void zero_ref_callback(ImplHandle const * handle);
};

struct daxa_ImplEvent final : ImplHandle
{
    daxa_Device device = {};
    EventInfo info = {};
    VkEvent vk_event = {};

    static void zero_ref_callback(ImplHandle const * handle);
};