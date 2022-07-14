#pragma once

#include <daxa/context.hpp>

#include "impl_core.hpp"

namespace daxa
{
    struct ImplContext
    {
        ContextInfo info;
        VkInstance vk_instance_handle;
        VkDebugUtilsMessengerEXT vk_debug_utils_messenger;
        bool enable_debug_names = true;

        ImplContext(ContextInfo const & info);
        ~ImplContext();
    };
} // namespace daxa
