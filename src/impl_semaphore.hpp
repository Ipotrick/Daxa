#pragma once

#include <daxa/sync.hpp>

#include "impl_core.hpp"
#include <daxa/c/device.h>

namespace daxa
{
    struct ImplDevice;

    struct SemaphoreZombie
    {
        VkSemaphore vk_semaphore = {};
    };

    struct ImplBinarySemaphore final : ManagedSharedState
    {
        using InfoT = BinarySemaphoreInfo;

        daxa_Device device = {};
        VkSemaphore vk_semaphore = {};
        BinarySemaphoreInfo info = {};

        ImplBinarySemaphore(daxa_Device a_device, BinarySemaphoreInfo const & a_info);
        ~ImplBinarySemaphore();

        void initialize(BinarySemaphoreInfo const & a_info);
        void reset();
    };

    struct ImplTimelineSemaphore final : ManagedSharedState
    {
        daxa_Device device = {};
        VkSemaphore vk_semaphore = {};
        TimelineSemaphoreInfo info = {};

        ImplTimelineSemaphore(daxa_Device a_device, TimelineSemaphoreInfo a_info);
        ~ImplTimelineSemaphore();
    };
} // namespace daxa
