#pragma once

#include <daxa/semaphore.hpp>

#include "impl_core.hpp"

namespace daxa
{
    struct ImplDevice;

    struct ImplBinarySemaphore
    {
        using InfoT = BinarySemaphoreInfo;

        std::weak_ptr<ImplDevice> impl_device = {};
        VkSemaphore vk_semaphore = {};
        BinarySemaphoreInfo info = {};

        ImplBinarySemaphore(std::weak_ptr<ImplDevice> a_impl_device);
        ~ImplBinarySemaphore();

        void initialize(BinarySemaphoreInfo const & info);
        void reset();
    };
} // namespace daxa
