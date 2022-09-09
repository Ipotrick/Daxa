#include "impl_semaphore.hpp"
#include "impl_device.hpp"

#include <iostream>

namespace daxa
{
    BinarySemaphore::BinarySemaphore(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}

    auto BinarySemaphore::info() const -> BinarySemaphoreInfo const &
    {
        auto & impl = *as<ImplBinarySemaphore>();
        return impl.info;
    }

    ImplBinarySemaphore::ImplBinarySemaphore(ManagedWeakPtr a_impl_device, BinarySemaphoreInfo const & info)
        : impl_device{a_impl_device}
    {
        // std::cout << "sem" << std::endl;

        VkSemaphoreCreateInfo vk_semaphore_create_info{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
        };

        vkCreateSemaphore(impl_device.as<ImplDevice>()->vk_device, &vk_semaphore_create_info, nullptr, &this->vk_semaphore);

        initialize(info);
    }

    ImplBinarySemaphore::~ImplBinarySemaphore()
    {
        auto device = this->impl_device.as<ImplDevice>();
        DAXA_ONLY_IF_THREADSAFETY(std::unique_lock lock{device->main_queue_zombies_mtx});
        u64 main_queue_cpu_timeline = DAXA_ATOMIC_FETCH(device->main_queue_cpu_timeline);

        device->main_queue_semaphore_zombies.push_back({
            main_queue_cpu_timeline,
            SemaphoreZombie{
                .vk_semaphore = vk_semaphore,
            },
        });
    }

    void ImplBinarySemaphore::initialize(BinarySemaphoreInfo const & info)
    {
        this->info = info;

        if (this->impl_device.as<ImplDevice>()->impl_ctx.as<ImplContext>()->enable_debug_names && this->info.debug_name.size() > 0)
        {
            auto binary_semaphore_name = this->info.debug_name + std::string(" [Daxa BinarySemaphore]");
            VkDebugUtilsObjectNameInfoEXT name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_SEMAPHORE,
                .objectHandle = reinterpret_cast<u64>(this->vk_semaphore),
                .pObjectName = binary_semaphore_name.c_str(),
            };
            vkSetDebugUtilsObjectNameEXT(impl_device.as<ImplDevice>()->vk_device, &name_info);
        }
    }

    auto ImplBinarySemaphore::managed_cleanup() -> bool
    {
        return true;
    }

    TimelineSemaphore::TimelineSemaphore(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}

    auto TimelineSemaphore::value() const -> u64
    {
        auto & impl = *as<ImplTimelineSemaphore>();

        u64 ret = {};
        vkGetSemaphoreCounterValue(impl.impl_device.as<ImplDevice>()->vk_device, impl.vk_semaphore, &ret);
        return ret;
    }

    void TimelineSemaphore::set_value(u64 value)
    {
        auto & impl = *as<ImplTimelineSemaphore>();

        VkSemaphoreSignalInfo vk_semaphore_signal_info{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .semaphore = impl.vk_semaphore,
            .value = value,
        };

        vkSignalSemaphore(impl.impl_device.as<ImplDevice>()->vk_device, &vk_semaphore_signal_info);
    }

    auto TimelineSemaphore::wait_for_value(u64 value, u64 timeout_nanos) -> bool
    {
        auto & impl = *as<ImplTimelineSemaphore>();

        VkSemaphoreWaitInfo vk_semaphore_wait_info{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
            .pNext = nullptr,
            .flags = {},
            .semaphoreCount = 1,
            .pSemaphores = &impl.vk_semaphore,
            .pValues = &value,
        };

        VkResult result = vkWaitSemaphores(impl.impl_device.as<ImplDevice>()->vk_device, &vk_semaphore_wait_info, timeout_nanos);
        return result != VK_TIMEOUT;
    }

    auto TimelineSemaphore::info() const -> TimelineSemaphoreInfo const &
    {
        auto & impl = *as<ImplTimelineSemaphore>();
        return impl.info;
    }

    ImplTimelineSemaphore::ImplTimelineSemaphore(ManagedWeakPtr a_impl_device, TimelineSemaphoreInfo const & a_info)
        : impl_device{a_impl_device}, info{a_info}
    {
        // std::cout << "tsem" << std::endl;

        VkSemaphoreTypeCreateInfo timeline_vk_semaphore{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .pNext = nullptr,
            .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
            .initialValue = info.initial_value,
        };

        VkSemaphoreCreateInfo vk_semaphore_create_info{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &timeline_vk_semaphore,
            .flags = {},
        };

        vkCreateSemaphore(impl_device.as<ImplDevice>()->vk_device, &vk_semaphore_create_info, nullptr, &this->vk_semaphore);

        if (this->impl_device.as<ImplDevice>()->impl_ctx.as<ImplContext>()->enable_debug_names && this->info.debug_name.size() > 0)
        {
            auto timeline_semaphore_name = this->info.debug_name + std::string(" [Daxa TimelineSemaphore]");
            VkDebugUtilsObjectNameInfoEXT name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_SEMAPHORE,
                .objectHandle = reinterpret_cast<uint64_t>(this->vk_semaphore),
                .pObjectName = timeline_semaphore_name.c_str(),
            };
            vkSetDebugUtilsObjectNameEXT(impl_device.as<ImplDevice>()->vk_device, &name_info);
        }
    }

    ImplTimelineSemaphore::~ImplTimelineSemaphore()
    {
        auto device = this->impl_device.as<ImplDevice>();
        DAXA_ONLY_IF_THREADSAFETY(std::unique_lock lock{device->main_queue_zombies_mtx});
        u64 main_queue_cpu_timeline = DAXA_ATOMIC_FETCH(device->main_queue_cpu_timeline);

        device->main_queue_semaphore_zombies.push_back({
            main_queue_cpu_timeline,
            SemaphoreZombie{
                .vk_semaphore = vk_semaphore,
            },
        });
    }

    auto ImplTimelineSemaphore::managed_cleanup() -> bool
    {
        return true;
    }

} // namespace daxa
