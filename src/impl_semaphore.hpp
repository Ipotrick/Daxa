#pragma once

#include <daxa/semaphore.hpp>

#include "impl_core.hpp"

namespace daxa
{
    struct ImplDevice;

    struct ImplBinarySemaphore final : ManagedSharedState
    {
        using InfoT = BinarySemaphoreInfo;

        ManagedWeakPtr impl_device = {};
        VkSemaphore vk_semaphore = {};
        BinarySemaphoreInfo info = {};

        ImplBinarySemaphore(ManagedWeakPtr a_impl_device);
        ~ImplBinarySemaphore();

        void initialize(BinarySemaphoreInfo const & info);
        void reset();

        auto managed_cleanup() -> bool override final;
    };

    struct ImplTimelineSemaphore final : ManagedSharedState
    {
        ManagedWeakPtr impl_device = {};
        VkSemaphore vk_semaphore = {};
        TimelineSemaphoreInfo info = {};

        ImplTimelineSemaphore(ManagedWeakPtr a_impl_device, TimelineSemaphoreInfo const & a_info);
        ~ImplTimelineSemaphore();

        auto managed_cleanup() -> bool override final;
    };
} // namespace daxa
