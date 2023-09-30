#include "cpp_wrapper.hpp"

#include <daxa/c/types.h>
#include <daxa/types.hpp>
#include <daxa/c/instance.h>
#include <daxa/instance.hpp>
#include <daxa/c/sync.h>
#include <daxa/sync.hpp>

#include <chrono>
#include <utility>
#include <fmt/format.h>
#include <bit>

namespace daxa
{
#if 0
    auto create_instance(InstanceInfo const & info) -> Instance
    {
        daxa_Instance instance;
        auto create_info = daxa_InstanceInfo{};
        if (info.enable_debug_utils)
        {
            create_info.flags |= DAXA_INSTANCE_FLAG_DEBUG_UTIL;
        }
        daxa_create_instance(&create_info, &instance);
        return Instance{ManagedPtr{new ImplInstance(instance, info)}};
    }
    Instance::Instance(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}
    auto Instance::create_device(DeviceInfo const & device_info) -> Device
    {
        auto & impl = *as<ImplInstance>();
        auto c_info = daxa_DeviceInfo{};
        auto c_device = daxa_Device{};
        auto res = daxa_instance_create_device(impl.instance, &c_info, &c_device);
        DAXA_DBG_ASSERT_TRUE_M(res == DAXA_RESULT_SUCCESS, "Failed to create device");
        return {};
    }

    ImplInstance::ImplInstance(daxa_Instance a_instance, InstanceInfo a_info)
        : instance{a_instance}, info{std::move(a_info)}
    {
    }

    ImplInstance::~ImplInstance() // NOLINT(bugprone-exception-escape)
    {
        daxa_destroy_instance(instance);
    }
#endif

    /// --- Begin Instance ---
    
    Instance::Instance(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}

    void device_deletor(daxa::ManagedSharedState * v)
    {
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_dvc_destroy(reinterpret_cast<daxa_Device>(v)) == daxa_Result::DAXA_RESULT_SUCCESS,
            "failed to destroy device");
    }

    auto Instance::create_device(DeviceInfo const & device_info) -> Device
    {
        auto self = this->as<daxa_ImplInstance>();
        daxa_Device device = {};
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_instance_create_device(self, reinterpret_cast<daxa_DeviceInfo const *>(&device_info), &device) == VK_SUCCESS,
            "failed to create device");
        return Device(ManagedPtr{reinterpret_cast<ManagedSharedState *>(device), &device_deletor});
    }

    auto Instance::into() const -> InstanceInfo const &
    {
        auto self = this->as<daxa_ImplInstance>();
        return *reinterpret_cast<InstanceInfo const *>(daxa_instance_info(self));
    }

    void instance_deletor(daxa::ManagedSharedState * v)
    {
        // Can't fail.
        daxa_destroy_instance(reinterpret_cast<daxa_Instance>(v));
    }

    auto create_instance(InstanceInfo const & info) -> Instance
    {
        daxa_Instance instance;
        auto c_info = reinterpret_cast<daxa_InstanceInfo const *>(&info);
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_create_instance(c_info, &instance) == daxa_Result::DAXA_RESULT_SUCCESS,
            "failed to create instance");
        return Instance{ManagedPtr{reinterpret_cast<ManagedSharedState *>(instance), instance_deletor}};
    }

    /// --- End Instance ---

    /// --- Begin Device ---

    void memory_deletor(daxa::ManagedSharedState * v)
    {
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_memory_destroy(reinterpret_cast<daxa_MemoryBlock>(v)) == daxa_Result::DAXA_RESULT_SUCCESS,
            "failed to destroy memory");
    }

    auto Device::create_memory(MemoryBlockInfo const & info) -> MemoryBlock
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_info = reinterpret_cast<daxa_MemoryBlockInfo const *>(&info);
        daxa_MemoryBlock c_memory_block;
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_dvc_create_memory(self, c_info, &c_memory_block) == daxa_Result::DAXA_RESULT_SUCCESS,
            "failed to create memory");
        return MemoryBlock{ManagedPtr{reinterpret_cast<ManagedSharedState *>(c_memory_block), memory_deletor}};
    }

    auto Device::get_memory_requirements(BufferInfo const & info) -> MemoryRequirements
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_info = reinterpret_cast<daxa_BufferInfo const *>(&info);
        return std::bit_cast<MemoryRequirements>(daxa_dvc_buffer_memory_requirements(self, c_info));
    }

    auto Device::get_memory_requirements(ImageInfo const & info) -> MemoryRequirements
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_info = reinterpret_cast<daxa_ImageInfo const *>(&info);
        return std::bit_cast<MemoryRequirements>(daxa_dvc_image_memory_requirements(self, c_info));
    }


    auto Device::create_buffer(BufferInfo const & info) -> BufferId
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_info = reinterpret_cast<daxa_BufferInfo const *>(&info);
        daxa_BufferId c_buffer_id = {};
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_dvc_create_buffer(self, c_info, &c_buffer_id),
            "failed to create buffer",
        );
        return std::bit_cast<BufferId>(c_buffer_id);
    }

    auto Device::create_image(ImageInfo const & info) -> ImageId
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_info = reinterpret_cast<daxa_ImageInfo const *>(&info);
        daxa_ImageId c_image_id = {};
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_dvc_create_image(self, c_info, &c_image_id),
            "failed to create image",
        );
        return std::bit_cast<ImageId>(c_image_id);
    }

    auto Device::create_image_view(ImageViewInfo const & info) -> ImageViewId
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_info = reinterpret_cast<daxa_ImageViewInfo const *>(&info);
        daxa_ImageViewId c_image_view_id = {};
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_dvc_create_image_view(self, c_info, &c_image_view_id),
            "failed to create image_view",
        );
        return std::bit_cast<ImageViewId>(c_image_view_id);
    }

    auto Device::create_sampler(SamplerInfo const & info) -> SamplerId
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_info = reinterpret_cast<daxa_SamplerInfo const *>(&info);
        daxa_SamplerId c_sampler_id = {};
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_dvc_create_sampler(self, c_info, &c_sampler_id),
            "failed to create sampler",
        );
        return std::bit_cast<SamplerId>(c_sampler_id);
    }

    void Device::destroy_buffer(BufferId id)
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_id = std::bit_cast<daxa_BufferId>(id);
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_dvc_destroy_buffer(self, c_id),
            "failed to destroy buffer",
        );
    }

    void Device::destroy_image(ImageId id)
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_id = std::bit_cast<daxa_ImageId>(id);
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_dvc_destroy_image(self, c_id),
            "failed to destroy image",
        );
    }

    void Device::destroy_image_view(ImageViewId id)
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_id = std::bit_cast<daxa_ImageViewId>(id);
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_dvc_destroy_image_view(self, c_id),
            "failed to destroy image_view",
        );
    }

    void Device::destroy_sampler(SamplerId id)
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_id = std::bit_cast<daxa_SamplerId>(id);
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_dvc_destroy_sampler(self, c_id),
            "failed to destroy sampler",
        );
    }
        
    auto Device::info_buffer(BufferId id) const -> BufferInfo const &
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_id = std::bit_cast<daxa_BufferId>(id);
        daxa_BufferInfo* c_info = {}; 
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_dvc_info_buffer(self, c_id, &c_info),
            "failed to get info of buffer",
        );
        return *reinterpret_cast<BufferInfo const *>(c_info);
    }

    auto Device::info_image(ImageId id) const -> ImageInfo const &
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_id = std::bit_cast<daxa_ImageId>(id);
        daxa_ImageInfo* c_info = {}; 
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_dvc_info_image(self, c_id, &c_info),
            "failed to get info of image",
        );
        return *reinterpret_cast<ImageInfo const *>(c_info);
    }

    auto Device::info_image_view(ImageViewId id) const -> ImageViewInfo const &
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_id = std::bit_cast<daxa_ImageViewId>(id);
        daxa_ImageViewInfo* c_info = {}; 
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_dvc_info_image_view(self, c_id, &c_info),
            "failed to get info of image_view",
        );
        return *reinterpret_cast<ImageViewInfo const *>(c_info);
    }

    auto Device::info_sampler(SamplerId id) const -> SamplerInfo const &
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_id = std::bit_cast<daxa_SamplerId>(id);
        daxa_SamplerInfo* c_info = {}; 
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_dvc_info_sampler(self, c_id, &c_info),
            "failed to get info of sampler",
        );
        return *reinterpret_cast<SamplerInfo const *>(c_info);
    }


    auto Device::is_id_valid(ImageId id) const -> bool
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_id = std::bit_cast<daxa_BufferId>(id);
        return daxa_dvc_is_buffer_valid(self, c_id);
    }

    auto Device::is_id_valid(ImageViewId id) const -> bool
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_id = std::bit_cast<daxa_ImageId>(id);
        return daxa_dvc_is_image_valid(self, c_id);
    }

    auto Device::is_id_valid(BufferId id) const -> bool
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_id = std::bit_cast<daxa_ImageViewId>(id);
        return daxa_dvc_is_image_view_valid(self, c_id);
    }

    auto Device::is_id_valid(SamplerId id) const -> bool
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_id = std::bit_cast<daxa_SamplerId>(id);
        return daxa_dvc_is_sampler_valid(self, c_id);
    }


    auto Device::get_device_address(BufferId id) const -> BufferDeviceAddress
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_id = std::bit_cast<daxa_BufferId>(id);
        daxa_BufferDeviceAddress c_bda = {};
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_dvc_buffer_device_address(self, c_id, &c_bda),
            "failed to get buffer device address",
        );
        return std::bit_cast<daxa::BufferDeviceAddress>(c_bda);
    }
    
    auto Device::get_host_address(BufferId id) const -> void *
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_id = std::bit_cast<daxa_BufferId>(id);
        void* c_ptr = {};
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_dvc_buffer_host_address(self, c_id, &c_ptr),
            "failed to get buffer device address",
        );
        return c_ptr;
    }

    /// --- End Device ---

    /// --- Begin BinarySemaphore ---

    BinarySemaphore::BinarySemaphore(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}

    auto BinarySemaphore::info() const -> BinarySemaphoreInfo const &
    {
        auto self = this->as<daxa_ImplBinarySemaphore>();
        return *reinterpret_cast<BinarySemaphoreInfo const *>(
            daxa_binary_semaphore_info(self));
    }

    /// --- End BinarySemaphore ---

    /// --- Begin TimelineSemaphore ---

    TimelineSemaphore::TimelineSemaphore(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}

    auto TimelineSemaphore::info() const -> TimelineSemaphoreInfo const &
    {
        auto self = this->as<daxa_ImplTimelineSemaphore>();
        return *reinterpret_cast<TimelineSemaphoreInfo const *>(
            daxa_timeline_semaphore_info(self));
    }

    auto TimelineSemaphore::value() const -> u64
    {
        auto self = this->as<daxa_ImplTimelineSemaphore>();
        u64 ret = {};
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_timeline_semaphore_get_value(self, &ret) == VK_SUCCESS,
            "cant get timeline value");
        return ret;
    }

    void TimelineSemaphore::set_value(u64 value)
    {
        auto self = this->as<daxa_ImplTimelineSemaphore>();
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_timeline_semaphore_set_value(self, value) == VK_SUCCESS,
            "cant set timeline value");
    }

    auto TimelineSemaphore::wait_for_value(u64 value, u64 timeout_nanos) -> bool
    {
        auto self = this->as<daxa_ImplTimelineSemaphore>();
        auto result = daxa_timeline_semaphore_wait_for_value(self, value, timeout_nanos);
        DAXA_DBG_ASSERT_TRUE_M(result == VK_SUCCESS || result == VK_TIMEOUT, "could not wait on timeline");
        return result == VK_SUCCESS;
    }

    /// --- End TimelineSemaphore ---

    /// --- Begin Event

    Event::Event(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}

    auto Event::info() const -> EventInfo const &
    {
        auto self = this->as<daxa_ImplEvent>();
        return *reinterpret_cast<EventInfo const *>(daxa_event_info(self));
    }

    /// --- End Event

    /// --- Begin to_string ---

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
                           to_string(info.image_id));
    }

    auto to_string(AccessTypeFlags flags) -> std::string
    {
        if (flags == AccessTypeFlagBits::NONE)
        {
            return "NONE";
        }

        std::string ret = {};
        if ((flags & AccessTypeFlagBits::READ) != AccessTypeFlagBits::NONE)
        {
            // if (ret.size() != 0)
            // {
            //     ret += " | ";
            // }
            ret += "READ";
        }
        if ((flags & AccessTypeFlagBits::WRITE) != AccessTypeFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "WRITE";
        }
        return ret;
    }

    auto to_string(ImageLayout layout) -> std::string_view
    {
        switch (layout)
        {
        case ImageLayout::UNDEFINED: return "UNDEFINED";
        case ImageLayout::GENERAL: return "GENERAL";
        case ImageLayout::TRANSFER_SRC_OPTIMAL: return "TRANSFER_SRC_OPTIMAL";
        case ImageLayout::TRANSFER_DST_OPTIMAL: return "TRANSFER_DST_OPTIMAL";
        case ImageLayout::READ_ONLY_OPTIMAL: return "READ_ONLY_OPTIMAL";
        case ImageLayout::ATTACHMENT_OPTIMAL: return "ATTACHMENT_OPTIMAL";
        case ImageLayout::PRESENT_SRC: return "PRESENT_SRC";
        default: DAXA_DBG_ASSERT_TRUE_M(false, "invalid ImageLayout");
        }
        return "invalid ImageLayout";
    }

    auto to_string(ImageUsageFlags const & flags) -> std::string
    {
        if (flags == ImageUsageFlagBits::NONE)
        {
            return "NONE";
        }

        std::string ret = {};

        if ((flags & ImageUsageFlagBits::NONE) != ImageUsageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "NONE";
        }
        if ((flags & ImageUsageFlagBits::TRANSFER_SRC) != ImageUsageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "TRANSFER_SRC";
        }
        if ((flags & ImageUsageFlagBits::TRANSFER_DST) != ImageUsageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "TRANSFER_DST";
        }
        if ((flags & ImageUsageFlagBits::SHADER_SAMPLED) != ImageUsageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "SHADER_SAMPLED";
        }
        if ((flags & ImageUsageFlagBits::SHADER_STORAGE) != ImageUsageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "SHADER_STORAGE";
        }
        if ((flags & ImageUsageFlagBits::COLOR_ATTACHMENT) != ImageUsageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "COLOR_ATTACHMENT";
        }
        if ((flags & ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT) != ImageUsageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "DEPTH_STENCIL_ATTACHMENT";
        }
        if ((flags & ImageUsageFlagBits::TRANSIENT_ATTACHMENT) != ImageUsageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "TRANSIENT_ATTACHMENT";
        }
        if ((flags & ImageUsageFlagBits::FRAGMENT_DENSITY_MAP) != ImageUsageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "FRAGMENT_DENSITY_MAP";
        }
        if ((flags & ImageUsageFlagBits::FRAGMENT_SHADING_RATE_ATTACHMENT) != ImageUsageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "FRAGMENT_SHADING_RATE_ATTACHMENT";
        }
        return ret;
    }

    auto to_string(Format format) -> std::string_view
    {
        switch (format)
        {
        case Format::UNDEFINED: return "UNDEFINED";
        case Format::R4G4_UNORM_PACK8: return "R4G4_UNORM_PACK8";
        case Format::R4G4B4A4_UNORM_PACK16: return "R4G4B4A4_UNORM_PACK16";
        case Format::B4G4R4A4_UNORM_PACK16: return "B4G4R4A4_UNORM_PACK16";
        case Format::R5G6B5_UNORM_PACK16: return "R5G6B5_UNORM_PACK16";
        case Format::B5G6R5_UNORM_PACK16: return "B5G6R5_UNORM_PACK16";
        case Format::R5G5B5A1_UNORM_PACK16: return "R5G5B5A1_UNORM_PACK16";
        case Format::B5G5R5A1_UNORM_PACK16: return "B5G5R5A1_UNORM_PACK16";
        case Format::A1R5G5B5_UNORM_PACK16: return "A1R5G5B5_UNORM_PACK16";
        case Format::R8_UNORM: return "R8_UNORM";
        case Format::R8_SNORM: return "R8_SNORM";
        case Format::R8_USCALED: return "R8_USCALED";
        case Format::R8_SSCALED: return "R8_SSCALED";
        case Format::R8_UINT: return "R8_UINT";
        case Format::R8_SINT: return "R8_SINT";
        case Format::R8_SRGB: return "R8_SRGB";
        case Format::R8G8_UNORM: return "R8G8_UNORM";
        case Format::R8G8_SNORM: return "R8G8_SNORM";
        case Format::R8G8_USCALED: return "R8G8_USCALED";
        case Format::R8G8_SSCALED: return "R8G8_SSCALED";
        case Format::R8G8_UINT: return "R8G8_UINT";
        case Format::R8G8_SINT: return "R8G8_SINT";
        case Format::R8G8_SRGB: return "R8G8_SRGB";
        case Format::R8G8B8_UNORM: return "R8G8B8_UNORM";
        case Format::R8G8B8_SNORM: return "R8G8B8_SNORM";
        case Format::R8G8B8_USCALED: return "R8G8B8_USCALED";
        case Format::R8G8B8_SSCALED: return "R8G8B8_SSCALED";
        case Format::R8G8B8_UINT: return "R8G8B8_UINT";
        case Format::R8G8B8_SINT: return "R8G8B8_SINT";
        case Format::R8G8B8_SRGB: return "R8G8B8_SRGB";
        case Format::B8G8R8_UNORM: return "B8G8R8_UNORM";
        case Format::B8G8R8_SNORM: return "B8G8R8_SNORM";
        case Format::B8G8R8_USCALED: return "B8G8R8_USCALED";
        case Format::B8G8R8_SSCALED: return "B8G8R8_SSCALED";
        case Format::B8G8R8_UINT: return "B8G8R8_UINT";
        case Format::B8G8R8_SINT: return "B8G8R8_SINT";
        case Format::B8G8R8_SRGB: return "B8G8R8_SRGB";
        case Format::R8G8B8A8_UNORM: return "R8G8B8A8_UNORM";
        case Format::R8G8B8A8_SNORM: return "R8G8B8A8_SNORM";
        case Format::R8G8B8A8_USCALED: return "R8G8B8A8_USCALED";
        case Format::R8G8B8A8_SSCALED: return "R8G8B8A8_SSCALED";
        case Format::R8G8B8A8_UINT: return "R8G8B8A8_UINT";
        case Format::R8G8B8A8_SINT: return "R8G8B8A8_SINT";
        case Format::R8G8B8A8_SRGB: return "R8G8B8A8_SRGB";
        case Format::B8G8R8A8_UNORM: return "B8G8R8A8_UNORM";
        case Format::B8G8R8A8_SNORM: return "B8G8R8A8_SNORM";
        case Format::B8G8R8A8_USCALED: return "B8G8R8A8_USCALED";
        case Format::B8G8R8A8_SSCALED: return "B8G8R8A8_SSCALED";
        case Format::B8G8R8A8_UINT: return "B8G8R8A8_UINT";
        case Format::B8G8R8A8_SINT: return "B8G8R8A8_SINT";
        case Format::B8G8R8A8_SRGB: return "B8G8R8A8_SRGB";
        case Format::A8B8G8R8_UNORM_PACK32: return "A8B8G8R8_UNORM_PACK32";
        case Format::A8B8G8R8_SNORM_PACK32: return "A8B8G8R8_SNORM_PACK32";
        case Format::A8B8G8R8_USCALED_PACK32: return "A8B8G8R8_USCALED_PACK32";
        case Format::A8B8G8R8_SSCALED_PACK32: return "A8B8G8R8_SSCALED_PACK32";
        case Format::A8B8G8R8_UINT_PACK32: return "A8B8G8R8_UINT_PACK32";
        case Format::A8B8G8R8_SINT_PACK32: return "A8B8G8R8_SINT_PACK32";
        case Format::A8B8G8R8_SRGB_PACK32: return "A8B8G8R8_SRGB_PACK32";
        case Format::A2R10G10B10_UNORM_PACK32: return "A2R10G10B10_UNORM_PACK32";
        case Format::A2R10G10B10_SNORM_PACK32: return "A2R10G10B10_SNORM_PACK32";
        case Format::A2R10G10B10_USCALED_PACK32: return "A2R10G10B10_USCALED_PACK32";
        case Format::A2R10G10B10_SSCALED_PACK32: return "A2R10G10B10_SSCALED_PACK32";
        case Format::A2R10G10B10_UINT_PACK32: return "A2R10G10B10_UINT_PACK32";
        case Format::A2R10G10B10_SINT_PACK32: return "A2R10G10B10_SINT_PACK32";
        case Format::A2B10G10R10_UNORM_PACK32: return "A2B10G10R10_UNORM_PACK32";
        case Format::A2B10G10R10_SNORM_PACK32: return "A2B10G10R10_SNORM_PACK32";
        case Format::A2B10G10R10_USCALED_PACK32: return "A2B10G10R10_USCALED_PACK32";
        case Format::A2B10G10R10_SSCALED_PACK32: return "A2B10G10R10_SSCALED_PACK32";
        case Format::A2B10G10R10_UINT_PACK32: return "A2B10G10R10_UINT_PACK32";
        case Format::A2B10G10R10_SINT_PACK32: return "A2B10G10R10_SINT_PACK32";
        case Format::R16_UNORM: return "R16_UNORM";
        case Format::R16_SNORM: return "R16_SNORM";
        case Format::R16_USCALED: return "R16_USCALED";
        case Format::R16_SSCALED: return "R16_SSCALED";
        case Format::R16_UINT: return "R16_UINT";
        case Format::R16_SINT: return "R16_SINT";
        case Format::R16_SFLOAT: return "R16_SFLOAT";
        case Format::R16G16_UNORM: return "R16G16_UNORM";
        case Format::R16G16_SNORM: return "R16G16_SNORM";
        case Format::R16G16_USCALED: return "R16G16_USCALED";
        case Format::R16G16_SSCALED: return "R16G16_SSCALED";
        case Format::R16G16_UINT: return "R16G16_UINT";
        case Format::R16G16_SINT: return "R16G16_SINT";
        case Format::R16G16_SFLOAT: return "R16G16_SFLOAT";
        case Format::R16G16B16_UNORM: return "R16G16B16_UNORM";
        case Format::R16G16B16_SNORM: return "R16G16B16_SNORM";
        case Format::R16G16B16_USCALED: return "R16G16B16_USCALED";
        case Format::R16G16B16_SSCALED: return "R16G16B16_SSCALED";
        case Format::R16G16B16_UINT: return "R16G16B16_UINT";
        case Format::R16G16B16_SINT: return "R16G16B16_SINT";
        case Format::R16G16B16_SFLOAT: return "R16G16B16_SFLOAT";
        case Format::R16G16B16A16_UNORM: return "R16G16B16A16_UNORM";
        case Format::R16G16B16A16_SNORM: return "R16G16B16A16_SNORM";
        case Format::R16G16B16A16_USCALED: return "R16G16B16A16_USCALED";
        case Format::R16G16B16A16_SSCALED: return "R16G16B16A16_SSCALED";
        case Format::R16G16B16A16_UINT: return "R16G16B16A16_UINT";
        case Format::R16G16B16A16_SINT: return "R16G16B16A16_SINT";
        case Format::R16G16B16A16_SFLOAT: return "R16G16B16A16_SFLOAT";
        case Format::R32_UINT: return "R32_UINT";
        case Format::R32_SINT: return "R32_SINT";
        case Format::R32_SFLOAT: return "R32_SFLOAT";
        case Format::R32G32_UINT: return "R32G32_UINT";
        case Format::R32G32_SINT: return "R32G32_SINT";
        case Format::R32G32_SFLOAT: return "R32G32_SFLOAT";
        case Format::R32G32B32_UINT: return "R32G32B32_UINT";
        case Format::R32G32B32_SINT: return "R32G32B32_SINT";
        case Format::R32G32B32_SFLOAT: return "R32G32B32_SFLOAT";
        case Format::R32G32B32A32_UINT: return "R32G32B32A32_UINT";
        case Format::R32G32B32A32_SINT: return "R32G32B32A32_SINT";
        case Format::R32G32B32A32_SFLOAT: return "R32G32B32A32_SFLOAT";
        case Format::R64_UINT: return "R64_UINT";
        case Format::R64_SINT: return "R64_SINT";
        case Format::R64_SFLOAT: return "R64_SFLOAT";
        case Format::R64G64_UINT: return "R64G64_UINT";
        case Format::R64G64_SINT: return "R64G64_SINT";
        case Format::R64G64_SFLOAT: return "R64G64_SFLOAT";
        case Format::R64G64B64_UINT: return "R64G64B64_UINT";
        case Format::R64G64B64_SINT: return "R64G64B64_SINT";
        case Format::R64G64B64_SFLOAT: return "R64G64B64_SFLOAT";
        case Format::R64G64B64A64_UINT: return "R64G64B64A64_UINT";
        case Format::R64G64B64A64_SINT: return "R64G64B64A64_SINT";
        case Format::R64G64B64A64_SFLOAT: return "R64G64B64A64_SFLOAT";
        case Format::B10G11R11_UFLOAT_PACK32: return "B10G11R11_UFLOAT_PACK32";
        case Format::E5B9G9R9_UFLOAT_PACK32: return "E5B9G9R9_UFLOAT_PACK32";
        case Format::D16_UNORM: return "D16_UNORM";
        case Format::X8_D24_UNORM_PACK32: return "X8_D24_UNORM_PACK32";
        case Format::D32_SFLOAT: return "D32_SFLOAT";
        case Format::S8_UINT: return "S8_UINT";
        case Format::D16_UNORM_S8_UINT: return "D16_UNORM_S8_UINT";
        case Format::D24_UNORM_S8_UINT: return "D24_UNORM_S8_UINT";
        case Format::D32_SFLOAT_S8_UINT: return "D32_SFLOAT_S8_UINT";
        case Format::BC1_RGB_UNORM_BLOCK: return "BC1_RGB_UNORM_BLOCK";
        case Format::BC1_RGB_SRGB_BLOCK: return "BC1_RGB_SRGB_BLOCK";
        case Format::BC1_RGBA_UNORM_BLOCK: return "BC1_RGBA_UNORM_BLOCK";
        case Format::BC1_RGBA_SRGB_BLOCK: return "BC1_RGBA_SRGB_BLOCK";
        case Format::BC2_UNORM_BLOCK: return "BC2_UNORM_BLOCK";
        case Format::BC2_SRGB_BLOCK: return "BC2_SRGB_BLOCK";
        case Format::BC3_UNORM_BLOCK: return "BC3_UNORM_BLOCK";
        case Format::BC3_SRGB_BLOCK: return "BC3_SRGB_BLOCK";
        case Format::BC4_UNORM_BLOCK: return "BC4_UNORM_BLOCK";
        case Format::BC4_SNORM_BLOCK: return "BC4_SNORM_BLOCK";
        case Format::BC5_UNORM_BLOCK: return "BC5_UNORM_BLOCK";
        case Format::BC5_SNORM_BLOCK: return "BC5_SNORM_BLOCK";
        case Format::BC6H_UFLOAT_BLOCK: return "BC6H_UFLOAT_BLOCK";
        case Format::BC6H_SFLOAT_BLOCK: return "BC6H_SFLOAT_BLOCK";
        case Format::BC7_UNORM_BLOCK: return "BC7_UNORM_BLOCK";
        case Format::BC7_SRGB_BLOCK: return "BC7_SRGB_BLOCK";
        case Format::ETC2_R8G8B8_UNORM_BLOCK: return "ETC2_R8G8B8_UNORM_BLOCK";
        case Format::ETC2_R8G8B8_SRGB_BLOCK: return "ETC2_R8G8B8_SRGB_BLOCK";
        case Format::ETC2_R8G8B8A1_UNORM_BLOCK: return "ETC2_R8G8B8A1_UNORM_BLOCK";
        case Format::ETC2_R8G8B8A1_SRGB_BLOCK: return "ETC2_R8G8B8A1_SRGB_BLOCK";
        case Format::ETC2_R8G8B8A8_UNORM_BLOCK: return "ETC2_R8G8B8A8_UNORM_BLOCK";
        case Format::ETC2_R8G8B8A8_SRGB_BLOCK: return "ETC2_R8G8B8A8_SRGB_BLOCK";
        case Format::EAC_R11_UNORM_BLOCK: return "EAC_R11_UNORM_BLOCK";
        case Format::EAC_R11_SNORM_BLOCK: return "EAC_R11_SNORM_BLOCK";
        case Format::EAC_R11G11_UNORM_BLOCK: return "EAC_R11G11_UNORM_BLOCK";
        case Format::EAC_R11G11_SNORM_BLOCK: return "EAC_R11G11_SNORM_BLOCK";
        case Format::ASTC_4x4_UNORM_BLOCK: return "ASTC_4x4_UNORM_BLOCK";
        case Format::ASTC_4x4_SRGB_BLOCK: return "ASTC_4x4_SRGB_BLOCK";
        case Format::ASTC_5x4_UNORM_BLOCK: return "ASTC_5x4_UNORM_BLOCK";
        case Format::ASTC_5x4_SRGB_BLOCK: return "ASTC_5x4_SRGB_BLOCK";
        case Format::ASTC_5x5_UNORM_BLOCK: return "ASTC_5x5_UNORM_BLOCK";
        case Format::ASTC_5x5_SRGB_BLOCK: return "ASTC_5x5_SRGB_BLOCK";
        case Format::ASTC_6x5_UNORM_BLOCK: return "ASTC_6x5_UNORM_BLOCK";
        case Format::ASTC_6x5_SRGB_BLOCK: return "ASTC_6x5_SRGB_BLOCK";
        case Format::ASTC_6x6_UNORM_BLOCK: return "ASTC_6x6_UNORM_BLOCK";
        case Format::ASTC_6x6_SRGB_BLOCK: return "ASTC_6x6_SRGB_BLOCK";
        case Format::ASTC_8x5_UNORM_BLOCK: return "ASTC_8x5_UNORM_BLOCK";
        case Format::ASTC_8x5_SRGB_BLOCK: return "ASTC_8x5_SRGB_BLOCK";
        case Format::ASTC_8x6_UNORM_BLOCK: return "ASTC_8x6_UNORM_BLOCK";
        case Format::ASTC_8x6_SRGB_BLOCK: return "ASTC_8x6_SRGB_BLOCK";
        case Format::ASTC_8x8_UNORM_BLOCK: return "ASTC_8x8_UNORM_BLOCK";
        case Format::ASTC_8x8_SRGB_BLOCK: return "ASTC_8x8_SRGB_BLOCK";
        case Format::ASTC_10x5_UNORM_BLOCK: return "ASTC_10x5_UNORM_BLOCK";
        case Format::ASTC_10x5_SRGB_BLOCK: return "ASTC_10x5_SRGB_BLOCK";
        case Format::ASTC_10x6_UNORM_BLOCK: return "ASTC_10x6_UNORM_BLOCK";
        case Format::ASTC_10x6_SRGB_BLOCK: return "ASTC_10x6_SRGB_BLOCK";
        case Format::ASTC_10x8_UNORM_BLOCK: return "ASTC_10x8_UNORM_BLOCK";
        case Format::ASTC_10x8_SRGB_BLOCK: return "ASTC_10x8_SRGB_BLOCK";
        case Format::ASTC_10x10_UNORM_BLOCK: return "ASTC_10x10_UNORM_BLOCK";
        case Format::ASTC_10x10_SRGB_BLOCK: return "ASTC_10x10_SRGB_BLOCK";
        case Format::ASTC_12x10_UNORM_BLOCK: return "ASTC_12x10_UNORM_BLOCK";
        case Format::ASTC_12x10_SRGB_BLOCK: return "ASTC_12x10_SRGB_BLOCK";
        case Format::ASTC_12x12_UNORM_BLOCK: return "ASTC_12x12_UNORM_BLOCK";
        case Format::ASTC_12x12_SRGB_BLOCK: return "ASTC_12x12_SRGB_BLOCK";
        case Format::G8B8G8R8_422_UNORM: return "G8B8G8R8_422_UNORM";
        case Format::B8G8R8G8_422_UNORM: return "B8G8R8G8_422_UNORM";
        case Format::G8_B8_R8_3PLANE_420_UNORM: return "G8_B8_R8_3PLANE_420_UNORM";
        case Format::G8_B8R8_2PLANE_420_UNORM: return "G8_B8R8_2PLANE_420_UNORM";
        case Format::G8_B8_R8_3PLANE_422_UNORM: return "G8_B8_R8_3PLANE_422_UNORM";
        case Format::G8_B8R8_2PLANE_422_UNORM: return "G8_B8R8_2PLANE_422_UNORM";
        case Format::G8_B8_R8_3PLANE_444_UNORM: return "G8_B8_R8_3PLANE_444_UNORM";
        case Format::R10X6_UNORM_PACK16: return "R10X6_UNORM_PACK16";
        case Format::R10X6G10X6_UNORM_2PACK16: return "R10X6G10X6_UNORM_2PACK16";
        case Format::R10X6G10X6B10X6A10X6_UNORM_4PACK16: return "R10X6G10X6B10X6A10X6_UNORM_4PACK16";
        case Format::G10X6B10X6G10X6R10X6_422_UNORM_4PACK16: return "G10X6B10X6G10X6R10X6_422_UNORM_4PACK16";
        case Format::B10X6G10X6R10X6G10X6_422_UNORM_4PACK16: return "B10X6G10X6R10X6G10X6_422_UNORM_4PACK16";
        case Format::G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16: return "G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16";
        case Format::G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16: return "G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16";
        case Format::G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16: return "G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16";
        case Format::G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16: return "G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16";
        case Format::G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16: return "G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16";
        case Format::R12X4_UNORM_PACK16: return "R12X4_UNORM_PACK16";
        case Format::R12X4G12X4_UNORM_2PACK16: return "R12X4G12X4_UNORM_2PACK16";
        case Format::R12X4G12X4B12X4A12X4_UNORM_4PACK16: return "R12X4G12X4B12X4A12X4_UNORM_4PACK16";
        case Format::G12X4B12X4G12X4R12X4_422_UNORM_4PACK16: return "G12X4B12X4G12X4R12X4_422_UNORM_4PACK16";
        case Format::B12X4G12X4R12X4G12X4_422_UNORM_4PACK16: return "B12X4G12X4R12X4G12X4_422_UNORM_4PACK16";
        case Format::G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16: return "G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16";
        case Format::G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16: return "G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16";
        case Format::G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16: return "G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16";
        case Format::G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16: return "G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16";
        case Format::G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16: return "G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16";
        case Format::G16B16G16R16_422_UNORM: return "G16B16G16R16_422_UNORM";
        case Format::B16G16R16G16_422_UNORM: return "B16G16R16G16_422_UNORM";
        case Format::G16_B16_R16_3PLANE_420_UNORM: return "G16_B16_R16_3PLANE_420_UNORM";
        case Format::G16_B16R16_2PLANE_420_UNORM: return "G16_B16R16_2PLANE_420_UNORM";
        case Format::G16_B16_R16_3PLANE_422_UNORM: return "G16_B16_R16_3PLANE_422_UNORM";
        case Format::G16_B16R16_2PLANE_422_UNORM: return "G16_B16R16_2PLANE_422_UNORM";
        case Format::G16_B16_R16_3PLANE_444_UNORM: return "G16_B16_R16_3PLANE_444_UNORM";
        case Format::G8_B8R8_2PLANE_444_UNORM: return "G8_B8R8_2PLANE_444_UNORM";
        case Format::G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16: return "G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16";
        case Format::G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16: return "G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16";
        case Format::G16_B16R16_2PLANE_444_UNORM: return "G16_B16R16_2PLANE_444_UNORM";
        case Format::A4R4G4B4_UNORM_PACK16: return "A4R4G4B4_UNORM_PACK16";
        case Format::A4B4G4R4_UNORM_PACK16: return "A4B4G4R4_UNORM_PACK16";
        case Format::ASTC_4x4_SFLOAT_BLOCK: return "ASTC_4x4_SFLOAT_BLOCK";
        case Format::ASTC_5x4_SFLOAT_BLOCK: return "ASTC_5x4_SFLOAT_BLOCK";
        case Format::ASTC_5x5_SFLOAT_BLOCK: return "ASTC_5x5_SFLOAT_BLOCK";
        case Format::ASTC_6x5_SFLOAT_BLOCK: return "ASTC_6x5_SFLOAT_BLOCK";
        case Format::ASTC_6x6_SFLOAT_BLOCK: return "ASTC_6x6_SFLOAT_BLOCK";
        case Format::ASTC_8x5_SFLOAT_BLOCK: return "ASTC_8x5_SFLOAT_BLOCK";
        case Format::ASTC_8x6_SFLOAT_BLOCK: return "ASTC_8x6_SFLOAT_BLOCK";
        case Format::ASTC_8x8_SFLOAT_BLOCK: return "ASTC_8x8_SFLOAT_BLOCK";
        case Format::ASTC_10x5_SFLOAT_BLOCK: return "ASTC_10x5_SFLOAT_BLOCK";
        case Format::ASTC_10x6_SFLOAT_BLOCK: return "ASTC_10x6_SFLOAT_BLOCK";
        case Format::ASTC_10x8_SFLOAT_BLOCK: return "ASTC_10x8_SFLOAT_BLOCK";
        case Format::ASTC_10x10_SFLOAT_BLOCK: return "ASTC_10x10_SFLOAT_BLOCK";
        case Format::ASTC_12x10_SFLOAT_BLOCK: return "ASTC_12x10_SFLOAT_BLOCK";
        case Format::ASTC_12x12_SFLOAT_BLOCK: return "ASTC_12x12_SFLOAT_BLOCK";
        case Format::PVRTC1_2BPP_UNORM_BLOCK_IMG: return "PVRTC1_2BPP_UNORM_BLOCK_IMG";
        case Format::PVRTC1_4BPP_UNORM_BLOCK_IMG: return "PVRTC1_4BPP_UNORM_BLOCK_IMG";
        case Format::PVRTC2_2BPP_UNORM_BLOCK_IMG: return "PVRTC2_2BPP_UNORM_BLOCK_IMG";
        case Format::PVRTC2_4BPP_UNORM_BLOCK_IMG: return "PVRTC2_4BPP_UNORM_BLOCK_IMG";
        case Format::PVRTC1_2BPP_SRGB_BLOCK_IMG: return "PVRTC1_2BPP_SRGB_BLOCK_IMG";
        case Format::PVRTC1_4BPP_SRGB_BLOCK_IMG: return "PVRTC1_4BPP_SRGB_BLOCK_IMG";
        case Format::PVRTC2_2BPP_SRGB_BLOCK_IMG: return "PVRTC2_2BPP_SRGB_BLOCK_IMG";
        case Format::PVRTC2_4BPP_SRGB_BLOCK_IMG: return "PVRTC2_4BPP_SRGB_BLOCK_IMG";
        default: return "unknown";
        }
    }

    auto to_string(ImageMipArraySlice slice) -> std::string
    {
        return fmt::format("mips: {}-{}, layers: {}-{}",
                           slice.base_mip_level,
                           slice.base_mip_level + slice.level_count - 1,
                           slice.base_array_layer,
                           slice.base_array_layer + slice.layer_count - 1);
    }

    auto to_string(ImageArraySlice slice) -> std::string
    {
        return fmt::format("mip: {}, layers: {}-{}",
                           slice.mip_level,
                           slice.base_array_layer,
                           slice.base_array_layer + slice.layer_count - 1);
    }

    auto to_string(ImageSlice slice) -> std::string
    {
        return fmt::format(" mip: {}, layer: {}",
                           slice.mip_level,
                           slice.array_layer);
    }

    auto to_string(PipelineStageFlags flags) -> std::string
    {
        if (flags == PipelineStageFlagBits::NONE)
        {
            return "NONE";
        }

        std::string ret = {};

        if ((flags & PipelineStageFlagBits::TOP_OF_PIPE) != PipelineStageFlagBits::NONE)
        {
            ret += "TOP_OF_PIPE";
        }
        if ((flags & PipelineStageFlagBits::DRAW_INDIRECT) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "DRAW_INDIRECT";
        }
        if ((flags & PipelineStageFlagBits::VERTEX_SHADER) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "VERTEX_SHADER";
        }
        if ((flags & PipelineStageFlagBits::TASK_SHADER) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "TASK_SHADER";
        }
        if ((flags & PipelineStageFlagBits::MESH_SHADER) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "MESH_SHADER";
        }
        if ((flags & PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "TESSELLATION_CONTROL_SHADER";
        }
        if ((flags & PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "TESSELLATION_EVALUATION_SHADER";
        }
        if ((flags & PipelineStageFlagBits::GEOMETRY_SHADER) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "GEOMETRY_SHADER";
        }
        if ((flags & PipelineStageFlagBits::FRAGMENT_SHADER) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "FRAGMENT_SHADER";
        }
        if ((flags & PipelineStageFlagBits::EARLY_FRAGMENT_TESTS) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "EARLY_FRAGMENT_TESTS";
        }
        if ((flags & PipelineStageFlagBits::LATE_FRAGMENT_TESTS) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "LATE_FRAGMENT_TESTS";
        }
        if ((flags & PipelineStageFlagBits::COLOR_ATTACHMENT_OUTPUT) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "COLOR_ATTACHMENT_OUTPUT";
        }
        if ((flags & PipelineStageFlagBits::COMPUTE_SHADER) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "COMPUTE_SHADER";
        }
        if ((flags & PipelineStageFlagBits::TRANSFER) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "TRANSFER";
        }
        if ((flags & PipelineStageFlagBits::BOTTOM_OF_PIPE) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "BOTTOM_OF_PIPE";
        }
        if ((flags & PipelineStageFlagBits::HOST) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "HOST";
        }
        if ((flags & PipelineStageFlagBits::ALL_GRAPHICS) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "ALL_GRAPHICS";
        }
        if ((flags & PipelineStageFlagBits::ALL_COMMANDS) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "ALL_COMMANDS";
        }
        if ((flags & PipelineStageFlagBits::COPY) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "COPY";
        }
        if ((flags & PipelineStageFlagBits::RESOLVE) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "RESOLVE";
        }
        if ((flags & PipelineStageFlagBits::BLIT) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "BLIT";
        }
        if ((flags & PipelineStageFlagBits::CLEAR) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "CLEAR";
        }
        if ((flags & PipelineStageFlagBits::INDEX_INPUT) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "INDEX_INPUT";
        }
        if ((flags & PipelineStageFlagBits::PRE_RASTERIZATION_SHADERS) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "PRE_RASTERIZATION_SHADERS";
        }
        return ret;
    }

    auto to_string(Access access) -> std::string
    {
        return fmt::format("stages: {}, type: {}", to_string(access.stages), to_string(access.type));
    }

    /// --- End to_string ---
} // namespace daxa
