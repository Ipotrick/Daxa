#include "impl_instance.hpp"
#include "impl_device.hpp"

#include <vector>

auto daxa_create_instance(daxa_InstanceInfo const * info, daxa_Instance * out_instance) -> daxa_Result
{
    auto * self = new daxa_ImplInstance;
    std::vector<char const *> required_extensions{};
    required_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    if ((info->flags & DAXA_INSTANCE_FLAG_DEBUG_UTIL) != 0)
    {
        required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

#if defined(WIN32)
    required_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__linux__)
    required_extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#else
// no surface extension
#endif
    // Check existence of extensions:
    std::vector<VkExtensionProperties> instance_extensions = {};
    uint32_t instance_extension_count = {};
    vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr);
    instance_extensions.resize(instance_extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, instance_extensions.data());
    for (auto req_ext : required_extensions)
    {
        bool found = false;
        for (uint32_t i = 0; i < instance_extensions.size(); ++i)
        {
            if (std::strcmp(req_ext, instance_extensions[i].extensionName) == 0)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            return DAXA_RESULT_UNKNOWN;
        }
        // DAXA_DBG_ASSERT_TRUE_M(found, "Not all required instance extensions are available, extension missing: " + std::string(req_ext));
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
        .flags = {},
        .pApplicationInfo = &app_info,
        .enabledLayerCount = 0u,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(required_extensions.size()),
        .ppEnabledExtensionNames = required_extensions.data(),
    };
    auto vk_res = vkCreateInstance(&instance_ci, nullptr, &self->vk_instance);
    if (vk_res != VK_SUCCESS)
    {
        return DAXA_RESULT_UNKNOWN;
    }
    *out_instance = self;

    return DAXA_RESULT_SUCCESS;
}

auto daxa_instance_create_device(daxa_Instance self, daxa_DeviceInfo const * info, daxa_Device * out_device) -> daxa_Result
{
    uint32_t physical_device_n = 0;
    vkEnumeratePhysicalDevices(self->vk_instance, &physical_device_n, nullptr);
    std::vector<VkPhysicalDevice> physical_devices;
    physical_devices.resize(physical_device_n);
    vkEnumeratePhysicalDevices(self->vk_instance, &physical_device_n, physical_devices.data());

    auto device_score = [&](VkPhysicalDevice physical_device) -> int32_t
    {
        VkPhysicalDeviceProperties vk_device_properties;
        vkGetPhysicalDeviceProperties(physical_device, &vk_device_properties);
        if (vk_device_properties.apiVersion < VK_API_VERSION_1_3)
        {
            // NOTE: Found device with incompatible API version. Skipping this device...
            return 0;
        }
        // TODO add device limits
        return info->selector(&vk_device_properties);
    };

    auto device_comparator = [&](auto const & a, auto const & b) -> bool
    {
        return device_score(a) < device_score(b);
    };
    auto best_physical_device = std::max_element(physical_devices.begin(), physical_devices.end(), device_comparator);

    DAXA_DBG_ASSERT_TRUE_M(device_score(*best_physical_device) != -1, "no suitable device found");

    // TODO: check for every possible device if it has the required features and if not dont even consider them.

    VkPhysicalDevice physical_device = *best_physical_device;

    // TODO: Needs C impl of device
    *out_device = nullptr;

    auto [result, device] = daxa_Device::create(self, info, physical_device);
    *out_device = device;
    return result;
}

void daxa_destroy_instance(daxa_Instance self)
{
    vkDestroyInstance(self->vk_instance, nullptr);
    delete self;
}

auto daxa_instance_info(daxa_Instance self) -> daxa_InstanceInfo const *
{
    return &self->info;
}

auto daxa_instance_get_vk_instance(daxa_Instance self) -> VkInstance
{
    return self->vk_instance;
}
