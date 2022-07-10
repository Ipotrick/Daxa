#include <volk.h>
#define VK_NO_PROTOTYPES

#include "impl_context.hpp"

namespace daxa
{
    auto create_context(ContextInfo const & info) -> Context
    {
        Context ret;
        ret.backend = std::make_shared<ImplContext>(info);
        return ret;
    }

    ImplContext::ImplContext(ContextInfo const & info)
    {
        volkInitialize();

        const VkApplicationInfo app_info = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "Daxa Vulkan App",
            .applicationVersion = 0,
            .pEngineName = "daxa",
            .engineVersion = 1,
            .apiVersion = VK_API_VERSION_1_0,
        };

        std::vector<const char *> enabled_layers, extension_names;

        if (info.enable_validation)
        {
            enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
        }
        extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined(WIN32)
        extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__linux__)
        extension_names.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#else
// no surface extension
#endif

        {
            auto check_layers = [](auto && required_names, auto && layer_props) -> bool
            {
                for (auto & required_layer_name : required_names)
                {
                    bool found = false;
                    for (auto & existing_layer_prop : layer_props)
                    {
                        if (!strcmp(required_layer_name, existing_layer_prop.layerName))
                        {
                            found = true;
                            break;
                        }
                    }
                    DAXA_DBG_ASSERT_TRUE_M(found, "Cannot find layer: TODO(grundlett)");
                }
                return true;
            };

            std::vector<VkLayerProperties> instance_layers;
            u32 instance_layer_count;
            vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr);
            instance_layers.resize(instance_layer_count);
            vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.data());
            check_layers(enabled_layers, instance_layers);
        }

        VkInstanceCreateInfo instance_ci = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .pApplicationInfo = &app_info,
            .enabledLayerCount = static_cast<u32>(enabled_layers.size()),
            .ppEnabledLayerNames = enabled_layers.data(),
            .enabledExtensionCount = static_cast<u32>(extension_names.size()),
            .ppEnabledExtensionNames = extension_names.data(),
        };
        vkCreateInstance(&instance_ci, nullptr, &vk_instance_handle);

        volkLoadInstance(vk_instance_handle);
    }

    ImplContext::~ImplContext()
    {
        vkDestroyInstance(vk_instance_handle, nullptr);
    }

    auto Context::create_device(DeviceInfo const & info) -> Device
    {
        
    }
} // namespace daxa
