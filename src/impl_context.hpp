#pragma once

#include <vulkan/vulkan.h>

#include <daxa/context.hpp>

namespace daxa
{
    struct ImplContext
    {
        VkInstance vk_instance_handle;

        ImplContext(ContextInfo const & info);
        ~ImplContext();
    };
} // namespace daxa
