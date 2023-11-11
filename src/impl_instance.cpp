#include "impl_core.hpp"
#include "impl_instance.hpp"
#include "impl_device.hpp"

#include <vector>

// --- Begin API Functions ---

auto daxa_create_instance(daxa_InstanceInfo const * info, daxa_Instance * out_instance) -> daxa_Result
{
    auto ret = daxa_ImplInstance{};
    ret.info = *reinterpret_cast<InstanceInfo const *>(info);
    ret.engine_name = {ret.info.engine_name.data(), ret.info.engine_name.size()};
    ret.info.engine_name = ret.engine_name;
    ret.app_name = {ret.info.app_name.data(), ret.info.app_name.size()};
    ret.info.app_name = ret.app_name;
    std::vector<char const *> required_extensions{};
    required_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    if ((ret.info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE)
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
    auto vk_result = vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr);
    if (vk_result != VK_SUCCESS)
    {
        return std::bit_cast<daxa_Result>(vk_result);
    }
    instance_extensions.resize(instance_extension_count);
    vk_result = vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, instance_extensions.data());
    if (vk_result != VK_SUCCESS)
    {
        return std::bit_cast<daxa_Result>(vk_result);
    }
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
            return DAXA_RESULT_ERROR_EXTENSION_NOT_PRESENT;
        }
    }
    const VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = ret.app_name.c_str(),
        .applicationVersion = 0,
        .pEngineName = ret.engine_name.c_str(),
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
    vk_result = vkCreateInstance(&instance_ci, nullptr, &ret.vk_instance);
    if (vk_result != VK_SUCCESS)
    {
        return std::bit_cast<daxa_Result>(vk_result);
    }
    ret.strong_count = 1;
    *out_instance = new daxa_ImplInstance{};
    **out_instance = std::move(ret);
    return DAXA_RESULT_SUCCESS;
}

auto daxa_instance_create_device(daxa_Instance self, daxa_DeviceInfo const * info, daxa_Device * out_device) -> daxa_Result
{
    uint32_t physical_device_n = 0;
    auto vk_result = vkEnumeratePhysicalDevices(self->vk_instance, &physical_device_n, nullptr);
    if (vk_result != VK_SUCCESS)
    {
        return std::bit_cast<daxa_Result>(vk_result);
    }
    std::vector<VkPhysicalDevice> physical_devices;
    physical_devices.resize(physical_device_n);
    vk_result = vkEnumeratePhysicalDevices(self->vk_instance, &physical_device_n, physical_devices.data());
    if (vk_result != VK_SUCCESS)
    {
        return std::bit_cast<daxa_Result>(vk_result);
    }

    auto device_score = [&](VkPhysicalDevice physical_device) -> int32_t
    {
        auto props = construct_daxa_physical_device_properties(physical_device);
        if (props.vulkan_api_version < VK_API_VERSION_1_3)
        {
            // NOTE: Found device with incompatible API version. Skipping this device...
            return 0;
        }
        return info->selector(&props);
    };

    auto device_comparator = [&](auto const & a, auto const & b) -> bool
    {
        return device_score(a) < device_score(b);
    };
    auto best_physical_device = std::max_element(physical_devices.begin(), physical_devices.end(), device_comparator);

    if (device_score(*best_physical_device) == -1)
    {
        return DAXA_RESULT_NO_SUITABLE_DEVICE_FOUND;
    }

    VkPhysicalDevice physical_device = *best_physical_device;

    *out_device = new daxa_ImplDevice{};

    auto const result = daxa_ImplDevice::create(self, *info, physical_device, *out_device);
    if (result != DAXA_RESULT_SUCCESS)
    {
        delete *out_device;
    }
    else
    {
        (**out_device).strong_count = 1;
        self->inc_weak_refcnt();
    }
    return result;
}

auto daxa_instance_inc_refcnt(daxa_Instance self) -> u64
{
    return self->inc_refcnt();
}

auto daxa_instance_dec_refcnt(daxa_Instance self) -> u64
{
    return self->dec_refcnt(
        &daxa_ImplInstance::zero_ref_callback,
        self);
}

auto daxa_instance_info(daxa_Instance self) -> daxa_InstanceInfo const *
{
    return reinterpret_cast<daxa_InstanceInfo const *>(&self->info);
}

auto daxa_instance_get_vk_instance(daxa_Instance self) -> VkInstance
{
    return self->vk_instance;
}

// --- End API Functions ---

// --- Begin Internals ---

void daxa_ImplInstance::zero_ref_callback(ImplHandle const * handle)
{
    _DAXA_TEST_PRINT("daxa_ImplInstance::zero_ref_callback\n");
    daxa_Instance self = rc_cast<daxa_Instance>(handle);
    vkDestroyInstance(self->vk_instance, nullptr);
    delete self;
}

// --- End Internals ---