#include "impl_split_barrier.hpp"

#include <utility>

#include "impl_device.hpp"

namespace daxa
{
    SplitBarrierState::SplitBarrierState(SplitBarrierState && other) noexcept
        : device{other.device}, create_info{other.create_info}, data{other.data}
    {
        other.create_info = {};
        other.data = {};
    }

    auto SplitBarrierState::operator=(SplitBarrierState && other) noexcept -> SplitBarrierState &
    {
        this->cleanup();
        this->device = std::move(other.device);
        this->create_info = other.create_info;
        this->data = other.data;
        other.create_info = {};
        other.data = {};
        return *this;
    }

    SplitBarrierState::~SplitBarrierState() // NOLINT(bugprone-exception-escape)
    {
        this->cleanup();
    }

    auto SplitBarrierState::info() const -> SplitBarrierInfo const &
    {
        return this->create_info;
    }

    void SplitBarrierState::cleanup()
    {
        if (this->data != 0u)
        {
            auto * device_impl = this->device.as<ImplDevice>();
            DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const lock{device_impl->main_queue_zombies_mtx});
            u64 const main_queue_cpu_timeline = DAXA_ATOMIC_FETCH(device_impl->main_queue_cpu_timeline);

            device_impl->main_queue_split_barrier_zombies.emplace_back(
                main_queue_cpu_timeline,
                SplitBarrierZombie{
                    .vk_event = reinterpret_cast<VkEvent>(this->data),
                });
            this->data = {};
        }
    }

    SplitBarrierState::SplitBarrierState(ManagedWeakPtr a_impl_device, SplitBarrierInfo a_info)
        : device{std::move(a_impl_device)}, create_info{std::move(a_info)}
    {
        auto * impl_device = this->device.as<ImplDevice>();
        VkEventCreateInfo const vk_event_create_info{
            .sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_EVENT_CREATE_DEVICE_ONLY_BIT,
        };
        VkEvent event = {};
        vkCreateEvent(impl_device->vk_device, &vk_event_create_info, nullptr, &event);
        this->data = reinterpret_cast<u64>(event);

        if (impl_device->impl_ctx.as<ImplContext>()->enable_debug_names && !this->create_info.debug_name.empty())
        {
            auto name = this->create_info.debug_name + std::string(" [Daxa Split Barrier]");
            VkDebugUtilsObjectNameInfoEXT const name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_EVENT,
                .objectHandle = this->data,
                .pObjectName = name.c_str(),
            };
            impl_device->vkSetDebugUtilsObjectNameEXT(impl_device->vk_device, &name_info);
        }
    }
} // namespace daxa
