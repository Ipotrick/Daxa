#pragma once

#include <daxa/instance.hpp>

#include "impl_core.hpp"

namespace daxa
{
    struct ImplInstance final : ManagedSharedState
    {
        InstanceInfo info;
        VkInstance vk_instance = {};
        VkDebugUtilsMessengerEXT vk_debug_utils_messenger = {};

#if defined(DEBUG)
        bool enable_debug_names = true;
#else
        bool enable_debug_names = false;
#endif

        PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = {};
        PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT = {};
        PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT = {};

        explicit ImplInstance(InstanceInfo a_info);
        virtual ~ImplInstance() override final;
    };
} // namespace daxa
