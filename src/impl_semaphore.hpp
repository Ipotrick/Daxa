#pragma once

#include <daxa/semaphore.hpp>

#include "impl_core.hpp"

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

        ManagedWeakPtr impl_device = {};
        VkSemaphore vk_semaphore = {};
        BinarySemaphoreInfo info = {};

        ImplBinarySemaphore(ManagedWeakPtr a_impl_device, BinarySemaphoreInfo const & a_info);
        ~ImplBinarySemaphore();

        void initialize(BinarySemaphoreInfo const & a_info);
        void reset();
    };

    struct ImplTimelineSemaphore final : ManagedSharedState
    {
        ManagedWeakPtr impl_device = {};
        VkSemaphore vk_semaphore = {};
        TimelineSemaphoreInfo info = {};

        ImplTimelineSemaphore(ManagedWeakPtr a_impl_device, TimelineSemaphoreInfo a_info);
        ~ImplTimelineSemaphore();
    };
} // namespace daxa
