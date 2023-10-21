#include "impl_core.hpp"

#include "impl_instance.hpp"
#include "impl_device.hpp"

// --- Begin Helpers ---

auto is_depth_format(Format format) -> bool
{
    switch (format)
    {
    case Format::D16_UNORM: return true;
    case Format::X8_D24_UNORM_PACK32: return true;
    case Format::D32_SFLOAT: return true;
    case Format::S8_UINT: return true;
    case Format::D16_UNORM_S8_UINT: return true;
    case Format::D24_UNORM_S8_UINT: return true;
    case Format::D32_SFLOAT_S8_UINT: return true;
    default: return false;
    }
}

auto is_stencil_format(Format format) -> bool
{
    switch (format)
    {
    case Format::S8_UINT: return true;
    case Format::D16_UNORM_S8_UINT: return true;
    case Format::D24_UNORM_S8_UINT: return true;
    case Format::D32_SFLOAT_S8_UINT: return true;
    default: return false;
    }
}

auto infer_aspect_from_format(Format format) -> VkImageAspectFlags
{
    if (is_depth_format(format) || is_stencil_format(format))
    {
        return (is_depth_format(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0) | (is_stencil_format(format) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
    }
    return VK_IMAGE_ASPECT_COLOR_BIT;
}

auto make_subresource_range(ImageMipArraySlice const & slice, VkImageAspectFlags aspect) -> VkImageSubresourceRange
{
    return VkImageSubresourceRange{
        .aspectMask = aspect,
        .baseMipLevel = slice.base_mip_level,
        .levelCount = slice.level_count,
        .baseArrayLayer = slice.base_array_layer,
        .layerCount = slice.layer_count,
    };
}

auto make_subresource_layers(ImageArraySlice const & slice, VkImageAspectFlags aspect) -> VkImageSubresourceLayers
{
    return VkImageSubresourceLayers{
        .aspectMask = aspect,
        .mipLevel = slice.mip_level,
        .baseArrayLayer = slice.base_array_layer,
        .layerCount = slice.layer_count,
    };
}
auto create_surface(daxa_Instance instance, daxa_NativeWindowHandle handle, [[maybe_unused]] daxa_NativeWindowPlatform platform, VkSurfaceKHR * out_surface) -> daxa_Result
{
#if defined(_WIN32)
    VkWin32SurfaceCreateInfoKHR const surface_ci{
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .hinstance = GetModuleHandleA(nullptr),
        .hwnd = static_cast<HWND>(handle),
    };
    {
        auto func = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(vkGetInstanceProcAddr(instance->vk_instance, "vkCreateWin32SurfaceKHR"));
        VkResult vk_result = func(instance->vk_instance, &surface_ci, nullptr, out_surface);
        return std::bit_cast<daxa_Result>(vk_result);
    }
#elif defined(__linux__)
    switch (platform)
    {
#if DAXA_BUILT_WITH_WAYLAND
    case NativeWindowPlatform::WAYLAND_API:
    {
        // TODO(grundlett): figure out how to link Wayland
        VkWaylandSurfaceCreateInfoKHR surface_ci{
            .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .display = wl_display_connect(nullptr),
            .surface = static_cast<wl_surface *>(handle),
        };
        {
            auto func = reinterpret_cast<PFN_vkCreateWaylandSurfaceKHR>(vkGetInstanceProcAddr(instance->vk_instance, "vkCreateWaylandSurfaceKHR"));
            VkResult vk_result = func(instance->vk_instance, &surface_ci, nullptr, out_surface);
            return std::bit_cast<daxa_Result>(vk_result);
        }
    }
    break;
#endif
#if DAXA_BUILT_WITH_X11
    case NativeWindowPlatform::XLIB_API:
    default:
    {
        VkXlibSurfaceCreateInfoKHR surface_ci{
            .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .dpy = XOpenDisplay(nullptr),
            .window = reinterpret_cast<Window>(handle),
        };
        {
            auto func = reinterpret_cast<PFN_vkCreateXlibSurfaceKHR>(vkGetInstanceProcAddr(instance->vk_instance, "vkCreateXlibSurfaceKHR"));
            VkResult vk_result = func(instance->vk_instance, &surface_ci, nullptr, out_surface);
            return std::bit_cast<daxa_Result>(vk_result);
        }
    }
    break;
    }
#endif
#endif
}

auto mask_from_bit_count(u64 bits) -> u64
{
    return (1ull << bits) - 1;
}

// --- End Helpers ---

// --- Begin API Functions

auto daxa_default_view(daxa_ImageId id) -> daxa_ImageViewId
{
    return daxa_ImageViewId{.value = id.value};
}

auto daxa_index_of_buffer(daxa_BufferId id) -> u32
{
    return static_cast<u32>(id.value & mask_from_bit_count(DAXA_ID_INDEX_BITS));
}

auto daxa_index_of_image(daxa_ImageId id) -> u32
{
    return static_cast<u32>(id.value & mask_from_bit_count(DAXA_ID_INDEX_BITS));
}

auto daxa_index_of_image_view(daxa_ImageViewId id) -> u32
{
    return static_cast<u32>(id.value & mask_from_bit_count(DAXA_ID_INDEX_BITS));
}

auto daxa_index_of_sampler(daxa_SamplerId id) -> u32
{
    return static_cast<u32>(id.value & mask_from_bit_count(DAXA_ID_INDEX_BITS));
}

auto daxa_version_of_buffer(daxa_BufferId id) -> u64
{
    return (id.value >> DAXA_ID_VERSION_OFFSTET) & mask_from_bit_count(DAXA_ID_VERSION_BITS);
}

auto daxa_version_of_image(daxa_ImageId id) -> u64
{
    return (id.value >> DAXA_ID_VERSION_OFFSTET) & mask_from_bit_count(DAXA_ID_VERSION_BITS);
}

auto daxa_version_of_image_view(daxa_ImageViewId id) -> u64
{
    return (id.value >> DAXA_ID_VERSION_OFFSTET) & mask_from_bit_count(DAXA_ID_VERSION_BITS);
}

auto daxa_version_of_sampler(daxa_SamplerId id) -> u64
{
    return (id.value >> DAXA_ID_VERSION_OFFSTET) & mask_from_bit_count(DAXA_ID_VERSION_BITS);
}

// --- End API Functions

// --- Begin ImplHandle ---

auto ImplHandle::inc_refcnt() const -> u64
{
    auto & mut_strong_ref = *rc_cast<u64 *>(&this->strong_count);
    return std::atomic_ref{mut_strong_ref}.fetch_add(1, std::memory_order::relaxed);
}

auto ImplHandle::dec_refcnt(void (*zero_ref_callback)(ImplHandle const *), daxa_Instance instance) const -> u64
{
    auto & mut_strong_ref = *rc_cast<u64 *>(&this->strong_count);
    auto prev = std::atomic_ref{mut_strong_ref}.fetch_sub(1, std::memory_order::relaxed);
    if (prev == 1)
    {
        auto weak = this->get_weak_refcnt();
        if (weak == 0)
        {
            zero_ref_callback(this);
        }
        else if (instance != nullptr && (instance->info.flags & InstanceFlagBits::PARENT_MUST_OUTLIVE_CHILD) != InstanceFlagBits::NONE)
        {
            DAXA_DBG_ASSERT_TRUE_M(false, "not all children have been destroyed prior to destroying object");
        }
    }
    return prev;
}

auto ImplHandle::get_refcnt() const -> u64
{
    return std::atomic_ref{this->strong_count}.load(std::memory_order::relaxed);
}

auto ImplHandle::Minc_weak_refcnt([[maybe_unused]] char const * callsite) const -> u64
{
    _DAXA_TEST_PRINT("called \"inc_weak_refcnt\" in \"%s\"\n", callsite);
    auto & mut_weak_ref = *rc_cast<u64 *>(&this->weak_count);
    return std::atomic_ref{mut_weak_ref}.fetch_add(1, std::memory_order::relaxed);
}

auto ImplHandle::Mdec_weak_refcnt(void (*zero_ref_callback)(ImplHandle const *), daxa_Instance, [[maybe_unused]] char const * callsite) const -> u64
{
    _DAXA_TEST_PRINT("called \"dec_weak_refcnt\" in \"%s\"\n", callsite);
    auto & mut_weak_ref = *rc_cast<u64 *>(&this->weak_count);
    auto prev = std::atomic_ref{mut_weak_ref}.fetch_sub(1, std::memory_order::relaxed);
    if (prev == 1)
    {
        auto strong = this->get_refcnt();
        if (strong == 0)
        {
            zero_ref_callback(this);
        }
    }
    return prev;
}

auto ImplHandle::get_weak_refcnt() const -> u64
{
    return std::atomic_ref{this->weak_count}.load(std::memory_order::relaxed);
}

// --- End ImplHandle ---

// --- Begin daxa_ImplMemoryBlock ---

auto daxa_dvc_create_memory(daxa_Device self, daxa_MemoryBlockInfo const * info, daxa_MemoryBlock * out_memory_block) -> daxa_Result
{
    daxa_ImplMemoryBlock ret = {};
    ret.device = self;
    ret.info = std::bit_cast<daxa::MemoryBlockInfo>(*info);

    if (info->requirements.memoryTypeBits == 0)
    {
        DAXA_DBG_ASSERT_TRUE_M(false, "memory_type_bits must be non zero");
        return DAXA_RESULT_ERROR_UNKNOWN;
    }

    VmaAllocationCreateInfo create_info{
        .flags = info->flags,
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = {}, // TODO: idk what this is...
        .preferredFlags = {},
        .memoryTypeBits = {}, // TODO: idk what this is....
        .pool = {},
        .pUserData = {},
        .priority = 0.5f,
    };
    auto result = vmaAllocateMemory(self->vma_allocator, &info->requirements, &create_info, &ret.allocation, &ret.alloc_info);
    if (result != VK_SUCCESS)
    {
        return std::bit_cast<daxa_Result>(result);
    }

    ret.strong_count = 1;
    self->inc_weak_refcnt();
    *out_memory_block = new daxa_ImplMemoryBlock{};
    // TODO(general): memory block is missing a name.
    **out_memory_block = std::move(ret);
    return DAXA_RESULT_SUCCESS;
}

auto daxa_memory_block_info(daxa_MemoryBlock self) -> daxa_MemoryBlockInfo const *
{
    return r_cast<daxa_MemoryBlockInfo const *>(&self->info);
}

auto daxa_memory_block_inc_refcnt(daxa_MemoryBlock self) -> u64
{
    return self->inc_refcnt();
}

auto daxa_memory_block_dec_refcnt(daxa_MemoryBlock self) -> u64
{
    return self->dec_refcnt(
        &daxa_ImplMemoryBlock::zero_ref_callback,
        self->device->instance);
}

void daxa_ImplMemoryBlock::zero_ref_callback(ImplHandle const * handle)
{
    auto self = rc_cast<daxa_ImplMemoryBlock *>(handle);
    std::unique_lock const lock{self->device->main_queue_zombies_mtx};
    u64 const main_queue_cpu_timeline_value = self->device->main_queue_cpu_timeline.load(std::memory_order::relaxed);
    self->device->main_queue_memory_block_zombies.push_front({
        main_queue_cpu_timeline_value,
        MemoryBlockZombie{
            .allocation = self->allocation,
        },
    });
    self->device->dec_weak_refcnt(
        daxa_ImplDevice::zero_ref_callback,
        self->device->instance);
    delete self;
}

// --- End daxa_ImplMemoryBlock ---
