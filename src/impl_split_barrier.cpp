#include "impl_split_barrier.hpp"

#include <utility>

namespace daxa
{
    auto to_string(MemoryBarrierInfo const & info) -> std::string
    {
        return fmt::format("access: ({}) -> ({})", to_string(info.src_access), to_string(info.dst_access));
    }

    auto to_string(ImageBarrierInfo const & info) -> std::string
    {
        return fmt::format("access: ({}) -> ({}), layout: ({}) -> ({}), slice: {}, id: {}", 
        to_string(info.src_access), 
        to_string(info.dst_access),
        to_string(info.src_layout),
        to_string(info.dst_layout),
        to_string(info.image_slice),
        to_string(info.image_id)
        );
    }

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
            DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const lock{this->device->main_queue_zombies_mtx});
            u64 const main_queue_cpu_timeline = DAXA_ATOMIC_FETCH(this->device->main_queue_cpu_timeline);

            this->device->main_queue_split_barrier_zombies.emplace_back(
                main_queue_cpu_timeline,
                SplitBarrierZombie{
                    .vk_event = reinterpret_cast<VkEvent>(this->data),
                });
            this->data = {};
        }
    }

    SplitBarrierState::SplitBarrierState(daxa_Device a_device, SplitBarrierInfo a_info)
        : device{a_device}, create_info{std::move(a_info)}
    {
        VkEventCreateInfo const vk_event_create_info{
            .sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_EVENT_CREATE_DEVICE_ONLY_BIT,
        };
        VkEvent event = {};
        vkCreateEvent(device->vk_device, &vk_event_create_info, nullptr, &event);
        this->data = reinterpret_cast<u64>(event);

        if ((device->instance->info.flags & DAXA_INSTANCE_FLAG_DEBUG_UTIL) != 0 && !this->create_info.name.empty())
        {
            auto name = this->create_info.name;
            VkDebugUtilsObjectNameInfoEXT const name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_EVENT,
                .objectHandle = this->data,
                .pObjectName = name.c_str(),
            };
            device->vkSetDebugUtilsObjectNameEXT(device->vk_device, &name_info);
        }
    }
} // namespace daxa
