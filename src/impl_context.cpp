#include "impl_core.hpp"
#include "impl_context.hpp"
#include "impl_device.hpp"

namespace daxa
{
    VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_messenger_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
        VkDebugUtilsMessageTypeFlagsEXT msg_type,
        const VkDebugUtilsMessengerCallbackDataEXT * p_callback_data,
        void * p_user_data)
    {
        ContextInfo const & info = *reinterpret_cast<ContextInfo const *>(p_user_data);
        std::string_view msg = p_callback_data->pMessage;
        info.validation_callback(static_cast<MsgSeverity>(msg_severity), static_cast<MsgType>(msg_type), msg);
        return VK_TRUE;
    }

    auto create_context(ContextInfo const & info) -> Context
    {
        return Context{std::make_shared<ImplContext>(info)};
    }

    ImplContext::ImplContext(ContextInfo const & info_param)
        : info{info_param}
    {
        volkInitialize();

        const VkApplicationInfo app_info = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "Daxa Vulkan App",
            .applicationVersion = 0,
            .pEngineName = "daxa",
            .engineVersion = 1,
            .apiVersion = VK_API_VERSION_1_3,
        };

        std::vector<const char *> enabled_layers, extension_names;

        if (info.enable_validation)
        {
            enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
        }
        // enabled_layers.push_back("VK_LAYER_LUNARG_monitor");
        extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

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

        // volkLoadInstance(vk_instance_handle);
        volkLoadInstance(vk_instance_handle);

        VkDebugUtilsMessengerCreateInfoEXT createInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
            .pfnUserCallback = debug_utils_messenger_callback,
            .pUserData = const_cast<ContextInfo *>(&this->info),
        };

        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vk_instance_handle, "vkCreateDebugUtilsMessengerEXT");
        DAXA_DBG_ASSERT_TRUE_M(func != nullptr, "failed to set up debug messenger!");
        func(vk_instance_handle, &createInfo, nullptr, &vk_debug_utils_messenger);
    }

    ImplContext::~ImplContext()
    {
        if (info.enable_validation)
        {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vk_instance_handle, "vkDestroyDebugUtilsMessengerEXT");
            DAXA_DBG_ASSERT_TRUE_M(func != nullptr, "failed to destroy debug messenger!");
            func(vk_instance_handle, vk_debug_utils_messenger, nullptr);
        }

        vkDestroyInstance(vk_instance_handle, nullptr);
    }

    Context::Context(std::shared_ptr<void> impl) : Handle(impl) {}

    auto Context::create_device(DeviceInfo const & device_info) -> Device
    {
        ImplContext & impl = *reinterpret_cast<ImplContext *>(this->impl.get());

        u32 physical_device_n = 0;
        vkEnumeratePhysicalDevices(impl.vk_instance_handle, &physical_device_n, nullptr);
        std::vector<VkPhysicalDevice> physical_devices;
        physical_devices.resize(physical_device_n);
        vkEnumeratePhysicalDevices(impl.vk_instance_handle, &physical_device_n, physical_devices.data());

        auto device_score = [&](VkPhysicalDevice physical_device) -> i32
        {
            VkPhysicalDeviceProperties vk_device_properties;
            vkGetPhysicalDeviceProperties(physical_device, &vk_device_properties);
            return device_info.selector(*reinterpret_cast<DeviceVulkanInfo *>(&vk_device_properties));
        };

        auto device_comparator = [&](auto const & a, auto const & b) -> bool
        {
            return device_score(a) < device_score(b);
        };
        auto best_physical_device = std::max_element(physical_devices.begin(), physical_devices.end(), device_comparator);

        DAXA_DBG_ASSERT_TRUE_M(device_score(*best_physical_device) != -1, "no suitable device found");

        // TODO: check for every possible device if it has the required features and if not dont even consider them.

        auto physical_device = *best_physical_device;

        VkPhysicalDeviceProperties vk_device_properties;
        vkGetPhysicalDeviceProperties(physical_device, &vk_device_properties);
        auto device_vulkan_info = *reinterpret_cast<DeviceVulkanInfo *>(&vk_device_properties);

        // std::cout << "Selected device: " << vk_device_properties.deviceName << std::endl;

        return Device{std::make_shared<ImplDevice>(device_info, device_vulkan_info, std::static_pointer_cast<ImplContext>(this->impl), physical_device)};
    }

    auto Context::create_default_device() -> Device
    {
        auto default_selector = [](DeviceVulkanInfo const & device_info) -> i32
        {
            i32 score = 0;
            switch (device_info.device_type)
            {
            case daxa::DeviceType::DISCRETE_GPU: score += 10000; break;
            case daxa::DeviceType::VIRTUAL_GPU: score += 1000; break;
            case daxa::DeviceType::INTEGRATED_GPU: score += 100; break;
            default: break;
            }
            score += device_info.limits.max_memory_allocation_count;
            score += device_info.limits.max_descriptor_set_storage_buffers / 10;
            score += device_info.limits.max_image_array_layers;
            return score;
        };
        return create_device({
            .selector = default_selector,
            .debug_name = "Daxa Default Device",
        });
    }
} // namespace daxa
