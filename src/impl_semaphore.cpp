#include "impl_semaphore.hpp"

namespace daxa
{
    BinarySemaphore::BinarySemaphore(std::shared_ptr<void> a_impl) : Handle{a_impl} {}

    ImplBinarySemaphore::ImplBinarySemaphore(std::shared_ptr<ImplDevice> a_impl_device, BinarySemaphoreInfo const & info)
        : impl_device{a_impl_device}
    {
        VkSemaphoreCreateInfo vk_semaphore_create_info{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
        };

        vkCreateSemaphore(impl_device->vk_device_handle, &vk_semaphore_create_info, nullptr, &this->vk_semaphore_handle);

        if (this->info.debug_name.size() > 0)
        {
            VkDebugUtilsObjectNameInfoEXT name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_SEMAPHORE,
                .objectHandle = reinterpret_cast<uint64_t>(this->vk_semaphore_handle),
                .pObjectName = this->info.debug_name.c_str(),
            };
            vkSetDebugUtilsObjectNameEXT(impl_device->vk_device_handle, &name_info);
        }
    }

    ImplBinarySemaphore::~ImplBinarySemaphore()
    {
        vkDestroySemaphore(impl_device->vk_device_handle, this->vk_semaphore_handle, nullptr);
    }
} // namespace daxa
