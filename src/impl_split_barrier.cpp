#include "impl_split_barrier.hpp"

#include "impl_device.hpp"

namespace daxa
{
    SplitBarrier::SplitBarrier(SplitBarrier&& other)
        : device{ std::move(other.device) }
        , info{ other.info }
        , data{ other.data }
    {
        other.info = {};
        other.data = {};
    }

    auto SplitBarrier::operator=(SplitBarrier&& other) -> SplitBarrier&
    {
        this->cleanup();
        this->device = std::move(other.device);
        this->info = other.info;
        this->data = other.data;
        other.info = {};
        other.data = {};
        return *this;
    }

    SplitBarrier::~SplitBarrier()
    {
        this->cleanup();
    }

    auto SplitBarrier::info() const -> SplitBarrierInfo const &;
    {
        return this->info;
    }

    void SplitBarrier::cleanup()
    {
        if (this->data)
        {
            ImplDevice* device = this->device.as<ImplDevice>();
            DAXA_ONLY_IF_THREADSAFETY(std::unique_lock lock{device->main_queue_zombies_mtx});
            u64 main_queue_cpu_timeline = DAXA_ATOMIC_FETCH(device->main_queue_cpu_timeline);

            device->main_queue_event_zombies.push_back({
                main_queue_cpu_timeline,
                GPUEventZombie{
                    .vk_event = reinterpret_cast<VkEvent>(this->data),
                },
            });
            this->data = {};
        }
    }

    SplitBarrier::SplitBarrier(Device device, SplitBarrierInfo const& info)
        : device{ std::move(device) }
        , info{ info }
    {
        ImplDevice* impl_device = this->device.as<ImplDevice>();
        VkEventCreateInfo vk_event_create_info{
            .sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_EVENT_CREATE_DEVICE_ONLY_BIT,
        };
        VkEvent event = {};
        vkCreateEvent(impl_device->vk_device, &vk_event_create_info, nullptr, &event);
        this->data = reinterpret_cast<u64>(event);
    }
}