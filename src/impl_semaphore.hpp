#pragma once

#include <daxa/semaphore.hpp>

#include "impl_core.hpp"
#include "impl_device.hpp"

namespace daxa
{
    struct ImplBinarySemaphore
    {
        std::shared_ptr<ImplDevice> impl_device = {};
        VkSemaphore vk_semaphore_handle = {};
        BinarySemaphoreInfo info = {};

        ImplBinarySemaphore(std::shared_ptr<ImplDevice> a_impl_device, BinarySemaphoreInfo const & info);
        ~ImplBinarySemaphore();
    };
} // namespace daxa
