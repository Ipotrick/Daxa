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
        std::vector<char const *> required_extensions{};
        required_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        if (info.enable_debug_utils)
        {
            required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        auto instance_create_flags = VkInstanceCreateFlags{};

#if defined(WIN32)
        required_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__linux__)
        required_extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined(__APPLE__)
        required_extensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
        // Needed for MoltenVK.
        required_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        required_extensions.push_back("VK_EXT_metal_surface");
        instance_create_flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#else
// no surface extension
#endif
        // Check existence of extensions:
        std::vector<VkExtensionProperties> instance_extensions = {};
        u32 instance_extension_count = {};
        vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr);
        instance_extensions.resize(instance_extension_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, instance_extensions.data());
        for (auto req_ext : required_extensions)
        {
            bool found = false;
            for (u32 i = 0; i < instance_extensions.size(); ++i)
            {
                if (std::strcmp(req_ext, instance_extensions[i].extensionName) == 0)
                {
                    found = true;
                    break;
                }
            }
            DAXA_DBG_ASSERT_TRUE_M(found, "Not all required instance extensions are available, extension missing: " + std::string(req_ext));
        }
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
            .enabledLayerCount = 0u,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<u32>(required_extensions.size()),
            .ppEnabledExtensionNames = required_extensions.data(),
        };
        vkCreateInstance(&instance_ci, nullptr, &vk_instance);
    }

    ImplInstance::~ImplInstance() // NOLINT(bugprone-exception-escape)
    {
        vkDestroyInstance(vk_instance, nullptr);
    }
} // namespace daxa
