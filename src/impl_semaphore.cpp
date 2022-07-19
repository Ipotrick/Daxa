#include "impl_semaphore.hpp"
#include "impl_device.hpp"

namespace daxa
{
    BinarySemaphore::BinarySemaphore(std::shared_ptr<void> a_impl) : Handle{std::move(a_impl)} {}

    BinarySemaphore::~BinarySemaphore()
    {
        if (this->impl.use_count() == 1)
        {
            std::shared_ptr<ImplBinarySemaphore> impl = std::static_pointer_cast<ImplBinarySemaphore>(this->impl);
#if defined(DAXA_ENABLE_THREADSAFETY)
            std::unique_lock lock{DAXA_LOCK_WEAK(impl->impl_device)->main_queue_zombies_mtx};
            u64 main_queue_cpu_timeline_value = DAXA_LOCK_WEAK(impl->impl_device)->main_queue_cpu_timeline.load(std::memory_order::relaxed);
#else 
            u64 main_queue_cpu_timeline_value = DAXA_LOCK_WEAK(impl->impl_device)->main_queue_cpu_timeline;
#endif
            DAXA_LOCK_WEAK(impl->impl_device)->main_queue_binary_semaphore_zombies.push_back({main_queue_cpu_timeline_value, impl});
        }
    }

    auto BinarySemaphore::info() const -> BinarySemaphoreInfo const &
    {
        auto& impl = *reinterpret_cast<ImplBinarySemaphore*>(this->impl.get());
        return impl.info;
    }

    ImplBinarySemaphore::ImplBinarySemaphore(std::weak_ptr<ImplDevice> a_impl_device)
        : impl_device{a_impl_device}
    {
        VkSemaphoreCreateInfo vk_semaphore_create_info{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
        };

        vkCreateSemaphore(DAXA_LOCK_WEAK(impl_device)->vk_device, &vk_semaphore_create_info, nullptr, &this->vk_semaphore);
    }

    ImplBinarySemaphore::~ImplBinarySemaphore()
    {
        vkDestroySemaphore(DAXA_LOCK_WEAK(impl_device)->vk_device, this->vk_semaphore, nullptr);
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
                .objectHandle = reinterpret_cast<uint64_t>(this->vk_semaphore),
                .pObjectName = this->info.debug_name.c_str(),
            };
            vkSetDebugUtilsObjectNameEXT(DAXA_LOCK_WEAK(impl_device)->vk_device, &name_info);
        }
    }

    void ImplBinarySemaphore::reset()
    {
    }

    TimelineSemaphore::TimelineSemaphore(std::shared_ptr<void> a_impl) : Handle{a_impl} {}

    auto TimelineSemaphore::value() const -> u64
    {
        auto& impl = *reinterpret_cast<ImplTimelineSemaphore*>(this->impl.get());

        u64 ret = {};
        vkGetSemaphoreCounterValue(DAXA_LOCK_WEAK(impl.impl_device)->vk_device, impl.vk_semaphore, &ret);
        return ret;
    }

    void TimelineSemaphore::set_value(u64 value)
    {
        auto& impl = *reinterpret_cast<ImplTimelineSemaphore*>(this->impl.get());

        VkSemaphoreSignalInfo vk_semaphore_signal_info {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .semaphore = impl.vk_semaphore,
            .value = value,
        };

        vkSignalSemaphore(DAXA_LOCK_WEAK(impl.impl_device)->vk_device, &vk_semaphore_signal_info);
    }

    auto TimelineSemaphore::wait_for_value(u64 value, u64 timeout_nanos) -> bool
    {
        auto& impl = *reinterpret_cast<ImplTimelineSemaphore*>(this->impl.get());

        VkSemaphoreWaitInfo vk_semaphore_wait_info{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
            .pNext = nullptr,
            .flags = {},
            .semaphoreCount = 1,
            .pSemaphores = &impl.vk_semaphore,
            .pValues = &value,
        };

        VkResult result = vkWaitSemaphores(DAXA_LOCK_WEAK(impl.impl_device)->vk_device, &vk_semaphore_wait_info, timeout_nanos);
        return result != VK_TIMEOUT;
    }

    auto TimelineSemaphore::info() const -> TimelineSemaphoreInfo const &
    {
        auto& impl = *reinterpret_cast<ImplTimelineSemaphore*>(this->impl.get());
        return impl.info;
    }

    TimelineSemaphore::~TimelineSemaphore()
    {
        if (this->impl.use_count() == 1)
        {
            std::shared_ptr<ImplBinarySemaphore> impl = std::static_pointer_cast<ImplBinarySemaphore>(this->impl);
#if defined(DAXA_ENABLE_THREADSAFETY)
            std::unique_lock lock{DAXA_LOCK_WEAK(impl->impl_device)->main_queue_zombies_mtx};
            u64 main_queue_cpu_timeline_value = DAXA_LOCK_WEAK(impl->impl_device)->main_queue_cpu_timeline.load(std::memory_order::relaxed);
#else 
            u64 main_queue_cpu_timeline_value = DAXA_LOCK_WEAK(impl->impl_device)->main_queue_cpu_timeline;
#endif
            DAXA_LOCK_WEAK(impl->impl_device)->main_queue_timeline_semaphore_zombies.push_back({main_queue_cpu_timeline_value, impl});
        }
    }
    
    ImplTimelineSemaphore::ImplTimelineSemaphore(std::weak_ptr<ImplDevice> a_impl_device, TimelineSemaphoreInfo const & a_info)
        :info{ a_info }
    {
        VkSemaphoreTypeCreateInfo timelineCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .pNext = nullptr,
            .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
            .initialValue = info.initial_value,
        };

        VkSemaphoreCreateInfo vk_semaphore_create_info{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = reinterpret_cast<void*>(&timelineCreateInfo),
            .flags = {},
        };

        vkCreateSemaphore(DAXA_LOCK_WEAK(impl_device)->vk_device, &vk_semaphore_create_info, nullptr, &this->vk_semaphore);

        if (DAXA_LOCK_WEAK(DAXA_LOCK_WEAK(this->impl_device)->impl_ctx)->enable_debug_names && this->info.debug_name.size() > 0)
        {
            VkDebugUtilsObjectNameInfoEXT name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_SEMAPHORE,
                .objectHandle = reinterpret_cast<uint64_t>(this->vk_semaphore),
                .pObjectName = this->info.debug_name.c_str(),
            };
            vkSetDebugUtilsObjectNameEXT(DAXA_LOCK_WEAK(impl_device)->vk_device, &name_info);
        }
    }

    ImplTimelineSemaphore::~ImplTimelineSemaphore()
    {
        vkDestroySemaphore(DAXA_LOCK_WEAK(impl_device)->vk_device, this->vk_semaphore, nullptr);
        this->vk_semaphore = {};
    }

} // namespace daxa
