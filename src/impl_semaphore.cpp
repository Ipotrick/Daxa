#include "impl_semaphore.hpp"

namespace daxa
{
    BinarySemaphore::BinarySemaphore(std::shared_ptr<void> a_impl) : Handle{a_impl} {}

    BinarySemaphore::~BinarySemaphore()
    {
        auto & impl = *reinterpret_cast<ImplBinarySemaphore *>(this->impl.get());

        if (this->impl.use_count() == 1)
        {
            impl.reset();
            std::unique_lock lock{DAXA_LOCK_WEAK(impl.impl_device)->zombie_binary_semaphores_mtx};
            DAXA_LOCK_WEAK(impl.impl_device)->zombie_binary_semaphores.push_back(std::static_pointer_cast<ImplBinarySemaphore>(this->impl));
        }
    }

    ImplBinarySemaphore::ImplBinarySemaphore(std::weak_ptr<ImplDevice> a_impl_device)
        : impl_device{a_impl_device}
    {
        VkSemaphoreCreateInfo vk_semaphore_create_info{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
        };

        vkCreateSemaphore(DAXA_LOCK_WEAK(impl_device)->vk_device_handle, &vk_semaphore_create_info, nullptr, &this->vk_semaphore_handle);
    }

    ImplBinarySemaphore::~ImplBinarySemaphore()
    {
        vkDestroySemaphore(DAXA_LOCK_WEAK(impl_device)->vk_device_handle, this->vk_semaphore_handle, nullptr);
    }

    void ImplBinarySemaphore::initialize(BinarySemaphoreInfo const & info)
    {
        this->info = info;

        if (DAXA_LOCK_WEAK(DAXA_LOCK_WEAK(this->impl_device)->impl_ctx)->enable_debug_names && this->info.debug_name.size() > 0)
        {
            VkDebugUtilsObjectNameInfoEXT name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_SEMAPHORE,
                .objectHandle = reinterpret_cast<uint64_t>(this->vk_semaphore_handle),
                .pObjectName = this->info.debug_name.c_str(),
            };
            vkSetDebugUtilsObjectNameEXT(DAXA_LOCK_WEAK(impl_device)->vk_device_handle, &name_info);
        }
    }

    void ImplBinarySemaphore::reset()
    {
    }
} // namespace daxa
