#pragma once

#include <daxa/semaphore.hpp>

#include "impl_core.hpp"
#include "impl_device.hpp"

namespace daxa
{
    struct ImplBinarySemaphore
    {
        std::weak_ptr<ImplDevice> impl_device = {};
        VkSemaphore vk_semaphore_handle = {};
        BinarySemaphoreInfo info = {};

        ImplBinarySemaphore(std::weak_ptr<ImplDevice> a_impl_device);
        ~ImplBinarySemaphore();

        void initialize(BinarySemaphoreInfo const & info);
        void reset();
    };
} // namespace daxa
