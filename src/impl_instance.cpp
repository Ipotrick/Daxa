#include "impl_core.hpp"
#include "impl_instance.hpp"
#include "impl_device.hpp"

#include <chrono>
#include <utility>

namespace daxa
{
    auto create_instance(InstanceInfo const & info) -> Instance
    {
        return Instance{ManagedPtr{new ImplInstance(info)}};
    }

    Instance::Instance(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}

    auto Instance::create_device(DeviceInfo const & device_info) -> Device
    {
        auto & impl = *as<ImplInstance>();

        u32 physical_device_n = 0;
        vkEnumeratePhysicalDevices(impl.vk_instance, &physical_device_n, nullptr);
        std::vector<VkPhysicalDevice> physical_devices;
        physical_devices.resize(physical_device_n);
        vkEnumeratePhysicalDevices(impl.vk_instance, &physical_device_n, physical_devices.data());

        auto device_score = [&](VkPhysicalDevice physical_device) -> i32
        {
            VkPhysicalDeviceProperties vk_device_properties;
            vkGetPhysicalDeviceProperties(physical_device, &vk_device_properties);
            if (vk_device_properties.apiVersion < VK_API_VERSION_1_3)
            {
                // NOTE: Found device with incompatible API version. Skipping this device...
                return 0;
            }
            return device_info.selector(*reinterpret_cast<DeviceProperties *>(&vk_device_properties));
        };

        auto device_comparator = [&](auto const & a, auto const & b) -> bool
        {
            return device_score(a) < device_score(b);
        };
        auto best_physical_device = std::max_element(physical_devices.begin(), physical_devices.end(), device_comparator);

        DAXA_DBG_ASSERT_TRUE_M(device_score(*best_physical_device) != -1, "no suitable device found");

        // TODO: check for every possible device if it has the required features and if not dont even consider them.

        VkPhysicalDevice physical_device = *best_physical_device;

        return Device{ManagedPtr{new ImplDevice(device_info, this->make_weak(), physical_device)}};
    }

    ImplInstance::ImplInstance(InstanceInfo a_info)
        : info{std::move(a_info)}
    {
        std::vector<char const *> enabled_layers{};
        std::vector<char const *> extension_names{};
        {
            extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

            if (enable_debug_names)
                extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#if defined(VK_USE_PLATFORM_WIN32_KHR)
            extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
            extension_names.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
            // no surface extension
#else
            // no surface extension
#endif
        }

        {
            auto check_layers = [](auto && required_names, auto && layer_props) -> bool
            {
                for (auto & required_layer_name : required_names)
                {
                    [[maybe_unused]] bool found = false;
                    for (auto & existing_layer_prop : layer_props)
                    {
                        if (!strcmp(required_layer_name, existing_layer_prop.layerName))
                        {
                            found = true;
                            break;
                        }
                    }
                    DAXA_DBG_ASSERT_TRUE_M(found, std::string{"Cannot find layer"} + required_layer_name);
                }
                return true;
            };

            std::vector<VkLayerProperties> instance_layers = {};
            u32 instance_layer_count = {};
            vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr);
            instance_layers.resize(instance_layer_count);
            vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.data());
            check_layers(enabled_layers, instance_layers);
        }

        {
            const VkApplicationInfo app_info = {
                .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pNext = nullptr,
                .pApplicationName = "Daxa Vulkan App",
                .applicationVersion = 0,
                .pEngineName = "daxa",
                .engineVersion = 1,
                .apiVersion = VK_API_VERSION_1_3,
            };
            VkInstanceCreateInfo const instance_ci = {
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .pApplicationInfo = &app_info,
                .enabledLayerCount = static_cast<u32>(enabled_layers.size()),
                .ppEnabledLayerNames = enabled_layers.data(),
                .enabledExtensionCount = static_cast<u32>(extension_names.size()),
                .ppEnabledExtensionNames = extension_names.data(),
            };
            vkCreateInstance(&instance_ci, nullptr, &vk_instance);
        }

        if (enable_debug_names) {
            this->vkSetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetInstanceProcAddr(vk_instance, "vkSetDebugUtilsObjectNameEXT"));
            this->vkCmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetInstanceProcAddr(vk_instance, "vkCmdBeginDebugUtilsLabelEXT"));
            this->vkCmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetInstanceProcAddr(vk_instance, "vkCmdEndDebugUtilsLabelEXT"));
        }
    }

    ImplInstance::~ImplInstance() // NOLINT(bugprone-exception-escape)
    {
        vkDestroyInstance(vk_instance, nullptr);
    }
} // namespace daxa
