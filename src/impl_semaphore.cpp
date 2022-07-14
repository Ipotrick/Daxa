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
    }

    ImplBinarySemaphore::~ImplBinarySemaphore()
    {
        vkDestroySemaphore(impl_device->vk_device_handle, this->vk_semaphore_handle, nullptr);
    }
} // namespace daxa
