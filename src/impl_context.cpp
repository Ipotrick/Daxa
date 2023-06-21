#include "impl_core.hpp"
#include "impl_context.hpp"
#include "impl_device.hpp"

#include <chrono>
#include <utility>

namespace daxa
{
    VKAPI_ATTR auto VKAPI_CALL debug_utils_messenger_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
        VkDebugUtilsMessageTypeFlagsEXT msg_type,
        VkDebugUtilsMessengerCallbackDataEXT const * p_callback_data,
        void * p_user_data) -> VkBool32
    {
        ContextInfo const & info = *reinterpret_cast<ContextInfo const *>(p_user_data);
        std::string_view const msg = p_callback_data->pMessage;
        info.validation_callback(static_cast<MsgSeverity>(msg_severity), static_cast<MsgType>(msg_type), msg);
        return VK_TRUE;
    }

    auto create_context(ContextInfo const & info) -> Context
    {
        return Context{ManagedPtr{new ImplContext(info)}};
    }

    Context::Context(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}

    auto Context::create_device(DeviceInfo const & device_info) -> Device
    {
        auto & impl = *as<ImplContext>();

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

    ImplContext::ImplContext(ContextInfo a_info)
        : info{std::move(a_info)}
    {
        std::vector<char const *> enabled_layers{};
        std::vector<char const *> extension_names{};

        auto instance_create_flags = VkInstanceCreateFlags{};

        this->info.enable_validation = false;
        {
            if (this->info.enable_validation)
            {
                enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
            }
            extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
            extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#if defined(WIN32)
            extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__linux__)
            extension_names.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined(__APPLE__)
            extension_names.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
            // Needed for MoltenVK.
            extension_names.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            extension_names.push_back("VK_EXT_metal_surface");
            instance_create_flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
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
                .flags = instance_create_flags,
                .pApplicationInfo = &app_info,
                .enabledLayerCount = static_cast<u32>(enabled_layers.size()),
                .ppEnabledLayerNames = enabled_layers.data(),
                .enabledExtensionCount = static_cast<u32>(extension_names.size()),
                .ppEnabledExtensionNames = extension_names.data(),
            };
            auto result = vkCreateInstance(&instance_ci, nullptr, &vk_instance);
            DAXA_DBG_ASSERT_TRUE_M(result == VK_SUCCESS && vk_instance != nullptr, "Failed to create a valid vk instance!");
        }

        if (this->info.enable_validation)
        {
            VkDebugUtilsMessengerCreateInfoEXT const dbg_utils_messenger_ci = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .pNext = {},
                .flags = {},
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
                .pfnUserCallback = debug_utils_messenger_callback,
                .pUserData = &this->info,
            };

            auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(vk_instance, "vkCreateDebugUtilsMessengerEXT"));
            DAXA_DBG_ASSERT_TRUE_M(func != nullptr, "failed to set up debug messenger!");
            func(vk_instance, &dbg_utils_messenger_ci, nullptr, &vk_debug_utils_messenger);
        }
    }

    ImplContext::~ImplContext() // NOLINT(bugprone-exception-escape)
    {
        if (this->info.enable_validation)
        {
            auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(vk_instance, "vkDestroyDebugUtilsMessengerEXT"));
            DAXA_DBG_ASSERT_TRUE_M(func != nullptr, "failed to destroy debug messenger!");
            func(vk_instance, vk_debug_utils_messenger, nullptr);
        }

        vkDestroyInstance(vk_instance, nullptr);
    }
} // namespace daxa
