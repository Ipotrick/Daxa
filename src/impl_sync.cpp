#include "impl_sync.hpp"
#include "impl_device.hpp"

daxa_BinarySemaphoreInfo const *
daxa_binary_semaphore_info(daxa_BinarySemaphore self)
{
    return &self->info;
}

VkSemaphore
daxa_binary_semaphore_get_vk_semaphore(daxa_BinarySemaphore self)
{
    return self->vk_semaphore;
}

void daxa_binary_semaphore_destroy(daxa_BinarySemaphore self)
{
    {
        DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const lock{self->device->main_queue_zombies_mtx});
        u64 const main_queue_cpu_timeline = DAXA_ATOMIC_FETCH(self->device->main_queue_cpu_timeline);
        self->device->main_queue_semaphore_zombies.emplace_back(
            main_queue_cpu_timeline,
            SemaphoreZombie{
                .vk_semaphore = self->vk_semaphore,
            });
    }
    delete self;
}

daxa_TimelineSemaphoreInfo const *
daxa_timeline_semaphore_info(daxa_TimelineSemaphore self)
{
    return &self->info;
}

daxa_Result
daxa_timeline_semaphore_get_value(daxa_TimelineSemaphore self, uint64_t * out_value)
{
    return static_cast<daxa_Result>(vkGetSemaphoreCounterValue(self->device->vk_device, self->vk_semaphore, out_value));
}

daxa_Result
daxa_timeline_semaphore_set_value(daxa_TimelineSemaphore self, uint64_t value)
{
    VkSemaphoreSignalInfo const vk_semaphore_signal_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .semaphore = self->vk_semaphore,
        .value = value,
    };

    return static_cast<daxa_Result>(vkSignalSemaphore(self->device->vk_device, &vk_semaphore_signal_info));
}

daxa_Result
daxa_timeline_semaphore_wait_for_value(daxa_TimelineSemaphore self, uint64_t value, uint64_t timeout)
{
    VkSemaphoreWaitInfo const vk_semaphore_wait_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        .pNext = nullptr,
        .flags = {},
        .semaphoreCount = 1,
        .pSemaphores = &self->vk_semaphore,
        .pValues = &value,
    };

    return static_cast<daxa_Result>(vkWaitSemaphores(self->device->vk_device, &vk_semaphore_wait_info, timeout));
}

VkSemaphore
daxa_timeline_semaphore_get_vk_semaphore(daxa_TimelineSemaphore self)
{
    return self->vk_semaphore;
}

void daxa_timeline_semaphore_destroy(daxa_TimelineSemaphore self)
{
    {
        DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const lock{self->device->main_queue_zombies_mtx});
        u64 const main_queue_cpu_timeline = DAXA_ATOMIC_FETCH(self->device->main_queue_cpu_timeline);

        self->device->main_queue_semaphore_zombies.emplace_back(
            main_queue_cpu_timeline,
            SemaphoreZombie{
                .vk_semaphore = self->vk_semaphore,
            });
    }
    delete self;
}

daxa_EventInfo const *
daxa_event_info(daxa_Event self)
{
    return &self->info;
}

void daxa_event_destroy(daxa_Event self)
{
    {
        DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const lock{self->device->main_queue_zombies_mtx});
        u64 const main_queue_cpu_timeline = DAXA_ATOMIC_FETCH(self->device->main_queue_cpu_timeline);

        self->device->main_queue_split_barrier_zombies.emplace_back(
            main_queue_cpu_timeline,
            EventZombie{
                .vk_event = self->vk_event,
            });
    }
    delete self;
}

auto daxa_ImplBinarySemaphore::create(daxa_Device device, daxa_BinarySemaphoreInfo a_info) -> daxa_BinarySemaphore
{
    daxa_ImplBinarySemaphore & self = *(new daxa_ImplBinarySemaphore);
    self.device = device;
    self.info = a_info;
    self.info_name = std::string(self.info.name.data, self.info.name.size);
    self.info.name.data = self.info_name.data();
    VkSemaphoreCreateInfo const vk_semaphore_create_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = {},
    };
    vkCreateSemaphore(device->vk_device, &vk_semaphore_create_info, nullptr, &self.vk_semaphore);
    if ((device->instance->info.flags & DAXA_INSTANCE_FLAG_DEBUG_UTIL) != 0 && !self.info_name.empty())
    {
        auto binary_semaphore_name = std::string{self.info.name.data, self.info.name.size};
        VkDebugUtilsObjectNameInfoEXT const name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_SEMAPHORE,
            .objectHandle = reinterpret_cast<u64>(self.vk_semaphore),
            .pObjectName = binary_semaphore_name.c_str(),
        };
        device->vkSetDebugUtilsObjectNameEXT(device->vk_device, &name_info);
    }
    return &self;
}

auto daxa_ImplTimelineSemaphore::create(daxa_Device device, daxa_TimelineSemaphoreInfo a_info) -> daxa_TimelineSemaphore
{
    daxa_ImplTimelineSemaphore & self = *(new daxa_ImplTimelineSemaphore);
    self.device = device;
    self.info = a_info;
    self.info_name = std::string(self.info.name.data, self.info.name.size);
    self.info.name.data = self.info_name.data();
    VkSemaphoreTypeCreateInfo timeline_vk_semaphore{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .pNext = nullptr,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = self.info.initial_value,
    };

    VkSemaphoreCreateInfo const vk_semaphore_create_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &timeline_vk_semaphore,
        .flags = {},
    };

    vkCreateSemaphore(self.device->vk_device, &vk_semaphore_create_info, nullptr, &self.vk_semaphore);

    if ((self.device->instance->info.flags & DAXA_INSTANCE_FLAG_DEBUG_UTIL) != 0 && !self.info_name.empty())
    {
        auto timeline_semaphore_name = self.info_name;
        VkDebugUtilsObjectNameInfoEXT const name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_SEMAPHORE,
            .objectHandle = reinterpret_cast<uint64_t>(self.vk_semaphore),
            .pObjectName = timeline_semaphore_name.c_str(),
        };
        self.device->vkSetDebugUtilsObjectNameEXT(self.device->vk_device, &name_info);
    }
    return &self;
}

auto daxa_ImplEvent::create(daxa_Device device, daxa_EventInfo a_info) -> daxa_Event
{
    daxa_ImplEvent & self = *(new daxa_ImplEvent);
    self.device = device;
    self.info = a_info;
    self.info_name = std::string(self.info.name.data, self.info.name.size);
    self.info.name.data = self.info_name.data();
    VkEventCreateInfo const vk_event_create_info{
        .sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_EVENT_CREATE_DEVICE_ONLY_BIT,
    };
    VkEvent event = {};
    vkCreateEvent(self.device->vk_device, &vk_event_create_info, nullptr, &event);
    self.vk_event = event;

    if ((device->instance->info.flags & DAXA_INSTANCE_FLAG_DEBUG_UTIL) != 0 && !self.info_name.empty())
    {
        auto name = self.info_name;
        VkDebugUtilsObjectNameInfoEXT const name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_EVENT,
            .objectHandle = std::bit_cast<uint64_t>(self.vk_event),
            .pObjectName = name.c_str(),
        };
        self.device->vkSetDebugUtilsObjectNameEXT(self.device->vk_device, &name_info);
    }
    return &self;
}