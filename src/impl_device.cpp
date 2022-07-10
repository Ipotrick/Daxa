#include "impl_core.hpp"
#include "impl_device.hpp"

namespace daxa
{

    Device::Device(std::shared_ptr<void> impl) : Handle(impl) {}

    auto Device::info() const -> DeviceInfo const &
    {
        auto & impl = *reinterpret_cast<ImplDevice *>(this->impl.get());
        return impl.info;
    }

    void Device::wait_idle()
    {
        auto & impl = *reinterpret_cast<ImplDevice *>(this->impl.get());

        vkQueueWaitIdle(impl.vk_main_queue_handle);
        vkDeviceWaitIdle(impl.vk_device_handle);
    }

    void Device::present_frame()
    {
        auto & impl = *reinterpret_cast<ImplDevice *>(this->impl.get());

        VkPresentInfoKHR present_info{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = nullptr,
            .swapchainCount = 0,
            .pSwapchains = nullptr,
            // .pImageIndices = ,
            // .pResults = ,
        };

        vkQueuePresentKHR(impl.vk_main_queue_handle, &present_info);
    }

    ImplDevice::ImplDevice(std::shared_ptr<ImplContext> impl_ctx, VkPhysicalDevice physical_device)
    {
        VkPhysicalDeviceProperties vk_device_properties;
        vkGetPhysicalDeviceProperties(physical_device, &vk_device_properties);
        this->info = *reinterpret_cast<DeviceInfo *>(&vk_device_properties);

        // SELECT QUEUE
        u32 vk_queue_family_index = std::numeric_limits<u32>::max();
        u32 queue_family_props_count = 0;
        std::vector<VkQueueFamilyProperties> queue_props;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_props_count, nullptr);
        queue_props.resize(queue_family_props_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_props_count, queue_props.data());
        std::vector<VkBool32> supports_present;
        supports_present.resize(queue_family_props_count);

        // Maybe check if device supports present for this surface?
        // this requires that the surface was already created.. which
        // is really CRINGE
        // for (uint32_t i = 0; i < queue_family_props_count; i++)
        //     vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &supports_present[i]);

        for (uint32_t i = 0; i < queue_family_props_count; i++)
        {
            if ((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0
                // && supports_present[i] == VK_TRUE
            )
            {
                vk_queue_family_index = i;
                break;
            }
        }
        DAXA_DBG_ASSERT_TRUE_M(vk_queue_family_index != std::numeric_limits<uint32_t>::max(), "found no suitable queue family");

        f32 queue_priorities[1] = {0.0};
        VkDeviceQueueCreateInfo queue_ci{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = vk_queue_family_index,
            .queueCount = 1,
            .pQueuePriorities = queue_priorities,
        };

        std::vector<char const *> extension_names;
        std::vector<char const *> enabled_layers;

        extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        VkDeviceCreateInfo device_ci = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .queueCreateInfoCount = static_cast<u32>(1),
            .pQueueCreateInfos = &queue_ci,
            .enabledLayerCount = static_cast<u32>(enabled_layers.size()),
            .ppEnabledLayerNames = enabled_layers.data(),
            .enabledExtensionCount = static_cast<u32>(extension_names.size()),
            .ppEnabledExtensionNames = extension_names.data(),
            .pEnabledFeatures = nullptr,
        };
        vkCreateDevice(physical_device, &device_ci, nullptr, &this->vk_device_handle);
        volkLoadDevice(this->vk_device_handle);
    }

    ImplDevice::~ImplDevice()
    {
        vkDestroyDevice(this->vk_device_handle, nullptr);
    }
} // namespace daxa