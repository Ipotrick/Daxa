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

#include "impl_instance.hpp"
#include "impl_device.hpp"

// --- Begin Helpers ---

auto daxa_result_to_string(daxa_Result result) -> std::string_view
{
    switch (result)
    {
    case DAXA_RESULT_SUCCESS: return "DAXA_RESULT_SUCCESS";
    case DAXA_RESULT_NOT_READY: return "DAXA_RESULT_NOT_READY";
    case DAXA_RESULT_TIMEOUT: return "DAXA_RESULT_TIMEOUT";
    case DAXA_RESULT_EVENT_SET: return "DAXA_RESULT_EVENT_SET";
    case DAXA_RESULT_EVENT_RESET: return "DAXA_RESULT_EVENT_RESET";
    case DAXA_RESULT_INCOMPLETE: return "DAXA_RESULT_INCOMPLETE";
    case DAXA_RESULT_ERROR_OUT_OF_HOST_MEMORY: return "DAXA_RESULT_ERROR_OUT_OF_HOST_MEMORY";
    case DAXA_RESULT_ERROR_OUT_OF_DEVICE_MEMORY: return "DAXA_RESULT_ERROR_OUT_OF_DEVICE_MEMORY";
    case DAXA_RESULT_ERROR_INITIALIZATION_FAILED: return "DAXA_RESULT_ERROR_INITIALIZATION_FAILED";
    case DAXA_RESULT_ERROR_DEVICE_LOST: return "DAXA_RESULT_ERROR_DEVICE_LOST";
    case DAXA_RESULT_ERROR_MEMORY_MAP_FAILED: return "DAXA_RESULT_ERROR_MEMORY_MAP_FAILED";
    case DAXA_RESULT_ERROR_LAYER_NOT_PRESENT: return "DAXA_RESULT_ERROR_LAYER_NOT_PRESENT";
    case DAXA_RESULT_ERROR_EXTENSION_NOT_PRESENT: return "DAXA_RESULT_ERROR_EXTENSION_NOT_PRESENT";
    case DAXA_RESULT_ERROR_FEATURE_NOT_PRESENT: return "DAXA_RESULT_ERROR_FEATURE_NOT_PRESENT";
    case DAXA_RESULT_ERROR_INCOMPATIBLE_DRIVER: return "DAXA_RESULT_ERROR_INCOMPATIBLE_DRIVER";
    case DAXA_RESULT_ERROR_TOO_MANY_OBJECTS: return "DAXA_RESULT_ERROR_TOO_MANY_OBJECTS";
    case DAXA_RESULT_ERROR_FORMAT_NOT_SUPPORTED: return "DAXA_RESULT_ERROR_FORMAT_NOT_SUPPORTED";
    case DAXA_RESULT_ERROR_FRAGMENTED_POOL: return "DAXA_RESULT_ERROR_FRAGMENTED_POOL";
    case DAXA_RESULT_ERROR_UNKNOWN: return "DAXA_RESULT_ERROR_UNKNOWN";
    case DAXA_RESULT_ERROR_OUT_OF_POOL_MEMORY: return "DAXA_RESULT_ERROR_OUT_OF_POOL_MEMORY";
    case DAXA_RESULT_ERROR_INVALID_EXTERNAL_HANDLE: return "DAXA_RESULT_ERROR_INVALID_EXTERNAL_HANDLE";
    case DAXA_RESULT_ERROR_FRAGMENTATION: return "DAXA_RESULT_ERROR_FRAGMENTATION";
    case DAXA_RESULT_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return "DAXA_RESULT_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
    case DAXA_RESULT_PIPELINE_COMPILE_REQUIRED: return "DAXA_RESULT_PIPELINE_COMPILE_REQUIRED";
    case DAXA_RESULT_ERROR_SURFACE_LOST_KHR: return "DAXA_RESULT_ERROR_SURFACE_LOST_KHR";
    case DAXA_RESULT_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "DAXA_RESULT_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case DAXA_RESULT_SUBOPTIMAL_KHR: return "DAXA_RESULT_SUBOPTIMAL_KHR";
    case DAXA_RESULT_ERROR_OUT_OF_DATE_KHR: return "DAXA_RESULT_ERROR_OUT_OF_DATE_KHR";
    case DAXA_RESULT_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "DAXA_RESULT_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case DAXA_RESULT_ERROR_VALIDATION_FAILED_EXT: return "DAXA_RESULT_ERROR_VALIDATION_FAILED_EXT";
    case DAXA_RESULT_ERROR_INVALID_SHADER_NV: return "DAXA_RESULT_ERROR_INVALID_SHADER_NV";
    case DAXA_RESULT_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR: return "DAXA_RESULT_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
    case DAXA_RESULT_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR: return "DAXA_RESULT_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
    case DAXA_RESULT_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR: return "DAXA_RESULT_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
    case DAXA_RESULT_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR: return "DAXA_RESULT_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
    case DAXA_RESULT_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR: return "DAXA_RESULT_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
    case DAXA_RESULT_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR: return "DAXA_RESULT_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
    case DAXA_RESULT_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "DAXA_RESULT_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
    case DAXA_RESULT_ERROR_NOT_PERMITTED_KHR: return "DAXA_RESULT_ERROR_NOT_PERMITTED_KHR";
    case DAXA_RESULT_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return "DAXA_RESULT_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
    case DAXA_RESULT_THREAD_IDLE_KHR: return "DAXA_RESULT_THREAD_IDLE_KHR";
    case DAXA_RESULT_THREAD_DONE_KHR: return "DAXA_RESULT_THREAD_DONE_KHR";
    case DAXA_RESULT_OPERATION_DEFERRED_KHR: return "DAXA_RESULT_OPERATION_DEFERRED_KHR";
    case DAXA_RESULT_OPERATION_NOT_DEFERRED_KHR: return "DAXA_RESULT_OPERATION_NOT_DEFERRED_KHR";
    case DAXA_RESULT_MISSING_EXTENSION: return "DAXA_RESULT_MISSING_EXTENSION";
    case DAXA_RESULT_INVALID_BUFFER_ID: return "DAXA_RESULT_INVALID_BUFFER_ID";
    case DAXA_RESULT_INVALID_IMAGE_ID: return "DAXA_RESULT_INVALID_IMAGE_ID";
    case DAXA_RESULT_INVALID_IMAGE_VIEW_ID: return "DAXA_RESULT_INVALID_IMAGE_VIEW_ID";
    case DAXA_RESULT_INVALID_SAMPLER_ID: return "DAXA_RESULT_INVALID_SAMPLER_ID";
    case DAXA_RESULT_BUFFER_DOUBLE_FREE: return "DAXA_RESULT_BUFFER_DOUBLE_FREE";
    case DAXA_RESULT_IMAGE_DOUBLE_FREE: return "DAXA_RESULT_IMAGE_DOUBLE_FREE";
    case DAXA_RESULT_IMAGE_VIEW_DOUBLE_FREE: return "DAXA_RESULT_IMAGE_VIEW_DOUBLE_FREE";
    case DAXA_RESULT_SAMPLER_DOUBLE_FREE: return "DAXA_RESULT_SAMPLER_DOUBLE_FREE";
    case DAXA_RESULT_INVALID_BUFFER_INFO: return "DAXA_RESULT_INVALID_BUFFER_INFO";
    case DAXA_RESULT_INVALID_IMAGE_INFO: return "DAXA_RESULT_INVALID_IMAGE_INFO";
    case DAXA_RESULT_INVALID_IMAGE_VIEW_INFO: return "DAXA_RESULT_INVALID_IMAGE_VIEW_INFO";
    case DAXA_RESULT_INVALID_SAMPLER_INFO: return "DAXA_RESULT_INVALID_SAMPLER_INFO";
    case DAXA_RESULT_COMMAND_LIST_COMPLETED: return "DAXA_RESULT_COMMAND_LIST_COMPLETED";
    case DAXA_RESULT_COMMAND_LIST_NOT_COMPLETED: return "DAXA_RESULT_COMMAND_LIST_NOT_COMPLETED";
    case DAXA_RESULT_INVALID_CLEAR_VALUE: return "DAXA_RESULT_INVALID_CLEAR_VALUE";
    case DAXA_RESULT_BUFFER_NOT_HOST_VISIBLE: return "DAXA_RESULT_BUFFER_NOT_HOST_VISIBLE";
    case DAXA_RESULT_BUFFER_NOT_DEVICE_VISIBLE: return "DAXA_RESULT_BUFFER_NOT_DEVICE_VISIBLE";
    case DAXA_RESULT_INCOMPLETE_COMMAND_LIST: return "DAXA_RESULT_INCOMPLETE_COMMAND_LIST";
    case DAXA_RESULT_DEVICE_DOES_NOT_SUPPORT_BUFFER_COUNT: return "DAXA_RESULT_DEVICE_DOES_NOT_SUPPORT_BUFFER_COUNT";
    case DAXA_RESULT_DEVICE_DOES_NOT_SUPPORT_IMAGE_COUNT: return "DAXA_RESULT_DEVICE_DOES_NOT_SUPPORT_IMAGE_COUNT";
    case DAXA_RESULT_DEVICE_DOES_NOT_SUPPORT_SAMPLER_COUNT: return "DAXA_RESULT_DEVICE_DOES_NOT_SUPPORT_SAMPLER_COUNT";
    case DAXA_RESULT_FAILED_TO_CREATE_NULL_BUFFER: return "DAXA_RESULT_FAILED_TO_CREATE_NULL_BUFFER";
    case DAXA_RESULT_FAILED_TO_CREATE_NULL_IMAGE: return "DAXA_RESULT_FAILED_TO_CREATE_NULL_IMAGE";
    case DAXA_RESULT_FAILED_TO_CREATE_NULL_IMAGE_VIEW: return "DAXA_RESULT_FAILED_TO_CREATE_NULL_IMAGE_VIEW";
    case DAXA_RESULT_FAILED_TO_CREATE_NULL_SAMPLER: return "DAXA_RESULT_FAILED_TO_CREATE_NULL_SAMPLER";
    case DAXA_RESULT_FAILED_TO_CREATE_BUFFER: return "DAXA_RESULT_FAILED_TO_CREATE_BUFFER";
    case DAXA_RESULT_FAILED_TO_CREATE_IMAGE: return "DAXA_RESULT_FAILED_TO_CREATE_IMAGE";
    case DAXA_RESULT_FAILED_TO_CREATE_IMAGE_VIEW: return "DAXA_RESULT_FAILED_TO_CREATE_IMAGE_VIEW";
    case DAXA_RESULT_FAILED_TO_CREATE_DEFAULT_IMAGE_VIEW: return "DAXA_RESULT_FAILED_TO_CREATE_DEFAULT_IMAGE_VIEW";
    case DAXA_RESULT_FAILED_TO_CREATE_SAMPLER: return "DAXA_RESULT_FAILED_TO_CREATE_SAMPLER";
    case DAXA_RESULT_FAILED_TO_CREATE_BDA_BUFFER: return "DAXA_RESULT_FAILED_TO_CREATE_BDA_BUFFER";
    case DAXA_RESULT_FAILED_TO_SUBMIT_DEVICE_INIT_COMMANDS: return "DAXA_RESULT_FAILED_TO_SUBMIT_DEVICE_INIT_COMMANDS";
    case DAXA_RESULT_INVALID_BUFFER_RANGE: return "DAXA_RESULT_INVALID_BUFFER_RANGE";
    case DAXA_RESULT_INVALID_BUFFER_OFFSET: return "DAXA_RESULT_INVALID_BUFFER_OFFSET";
    case DAXA_RESULT_INVALID_UNIFORM_BUFFER_SLOT: return "DAXA_RESULT_INVALID_UNIFORM_BUFFER_SLOT";
    case DAXA_RESULT_NO_SUITABLE_FORMAT_FOUND: return "DAXA_RESULT_NO_SUITABLE_FORMAT_FOUND";
    case DAXA_RESULT_RANGE_OUT_OF_BOUNDS: return "DAXA_RESULT_RANGE_OUT_OF_BOUNDS";
    case DAXA_RESULT_NO_SUITABLE_DEVICE_FOUND: return "DAXA_RESULT_NO_SUITABLE_DEVICE_FOUND";
    case DAXA_RESULT_MAX_ENUM: return "DAXA_RESULT_MAX_ENUM";
    default: return "UNIMPLEMENTED";
    }
    return "UNIMPLEMENTED";
};

void check_result(daxa_Result result, std::string const & message)
{
    if (result != DAXA_RESULT_SUCCESS)
    {
        throw std::runtime_error(fmt::format(
            "[[DAXA ASSERT FAILURE]]: error code: {}, {}.\n",
            daxa_result_to_string(result),
            message));
    }
}

// --- End Helpers ---

namespace daxa
{
    /// --- Begin Instance ---

    auto create_instance(InstanceInfo const & info) -> Instance
    {
        Instance instance = {};
        check_result(daxa_create_instance(
                         reinterpret_cast<daxa_InstanceInfo const *>(&info),
                         reinterpret_cast<daxa_Instance *>(&instance)),
                     "failed to create instance");
        return instance;
    }

    auto Instance::create_device(DeviceInfo const & device_info) -> Device
    {
        Device device = {};
        check_result(daxa_instance_create_device(
                         reinterpret_cast<daxa_Instance>(this),
                         reinterpret_cast<daxa_DeviceInfo const *>(&device_info),
                         reinterpret_cast<daxa_Device *>(&device)),
                     "failed to create device");
        return device;
    }

    auto Instance::info() const -> InstanceInfo const &
    {
        return *reinterpret_cast<InstanceInfo const *>(
            daxa_instance_info(const_cast<daxa_Instance>(reinterpret_cast<daxa_ImplInstance const *>(this))) 
        );
    }

    /// --- End Instance ---

    /// --- Begin Device ---

    auto Device::create_memory(MemoryBlockInfo const & info) -> MemoryBlock
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_info = reinterpret_cast<daxa_MemoryBlockInfo const *>(&info);
        daxa_MemoryBlock c_memory_block;
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_dvc_create_memory(self, c_info, &c_memory_block) == daxa_Result::DAXA_RESULT_SUCCESS,
            "failed to create memory");
        return MemoryBlock{ManagedPtr{reinterpret_cast<daxa_ImplHandle *>(c_memory_block)}};
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

#define _DAXA_DECL_GPU_RES_FN(Name, name)                                       \
    auto Device::create_##name(Name##Info const & info)->Name##Id               \
    {                                                                           \
        auto self = this->as<daxa_ImplDevice>();                                \
        auto c_info = reinterpret_cast<daxa_##Name##Info const *>(&info);       \
        daxa_##Name##Id c_id = {};                                              \
        DAXA_DBG_ASSERT_TRUE_M(                                                 \
            daxa_dvc_create_##name(self, c_info, &c_id),                        \
            "failed to create " #name);                                         \
        return std::bit_cast<Name##Id>(c_id);                                   \
    }                                                                           \
    void Device::destroy_##name(Name##Id id)                                    \
    {                                                                           \
        auto self = this->as<daxa_ImplDevice>();                                \
        auto c_id = std::bit_cast<daxa_##Name##Id>(id);                         \
        DAXA_DBG_ASSERT_TRUE_M(                                                 \
            daxa_dvc_dec_refcnt_##name(self, c_id),                             \
            "failed to destroy " #name);                                        \
    }                                                                           \
    auto Device::is_id_valid(Name##Id id) const->bool                           \
    {                                                                           \
        auto self = this->as<daxa_ImplDevice>();                                \
        auto c_id = std::bit_cast<daxa_##Name##Id>(id);                         \
        return daxa_dvc_is_##name##_valid(const_cast<daxa_Device>(self), c_id); \
    }                                                                           \
    auto Device::info_##name(Name##Id id) const->Name##Info const &             \
    {                                                                           \
        auto self = this->as<daxa_ImplDevice>();                                \
        auto c_id = std::bit_cast<daxa_##Name##Id>(id);                         \
        daxa_##Name##Info const * c_info = {};                                  \
        DAXA_DBG_ASSERT_TRUE_M(                                                 \
            daxa_dvc_info_##name(const_cast<daxa_Device>(self), c_id, &c_info), \
            "failed to get info of " #name);                                    \
        return *reinterpret_cast<Name##Info const *>(c_info);                   \
    }

    _DAXA_DECL_GPU_RES_FN(Buffer, buffer)
    _DAXA_DECL_GPU_RES_FN(Image, image)
    _DAXA_DECL_GPU_RES_FN(ImageView, image_view)
    _DAXA_DECL_GPU_RES_FN(Sampler, sampler)

    auto Device::get_device_address(BufferId id) const -> BufferDeviceAddress
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_id = std::bit_cast<daxa_BufferId>(id);
        daxa_BufferDeviceAddress c_bda = {};
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_dvc_buffer_device_address(const_cast<daxa_Device>(self), c_id, &c_bda),
            "failed to get buffer device address");
        return std::bit_cast<daxa::BufferDeviceAddress>(c_bda);
    }

    auto Device::get_host_address(BufferId id) const -> void *
    {
        auto self = this->as<daxa_ImplDevice>();
        auto c_id = std::bit_cast<daxa_BufferId>(id);
        void * c_ptr = {};
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_dvc_buffer_host_address(const_cast<daxa_Device>(self), c_id, &c_ptr),
            "failed to get buffer device address");
        return c_ptr;
    }

#define _DAXA_DECL_DVC_CREATE_FN(Name, name)                                     \
    auto Device::create_##name(Name##Info const & info)->Name                    \
    {                                                                            \
        auto self = this->as<daxa_ImplDevice>();                                 \
        auto c_info = reinterpret_cast<daxa_##Name##Info const *>(&info);        \
        daxa_##Name c_obj = {};                                                  \
        DAXA_DBG_ASSERT_TRUE_M(                                                  \
            daxa_dvc_create_##name(self, c_info, &c_obj) == DAXA_RESULT_SUCCESS, \
            "failed to create " #name);                                          \
        return Name{ManagedPtr{reinterpret_cast<daxa_ImplHandle *>(c_obj)}};     \
    }

    // TODO(capi) where do the deleters go now??
    _DAXA_DECL_DVC_CREATE_FN(RasterPipeline, raster_pipeline)
    _DAXA_DECL_DVC_CREATE_FN(ComputePipeline, compute_pipeline)
    _DAXA_DECL_DVC_CREATE_FN(Swapchain, swapchain)
    _DAXA_DECL_DVC_CREATE_FN(BinarySemaphore, binary_semaphore)
    _DAXA_DECL_DVC_CREATE_FN(TimelineSemaphore, timeline_semaphore)
    _DAXA_DECL_DVC_CREATE_FN(Event, event)
    _DAXA_DECL_DVC_CREATE_FN(TimelineQueryPool, timeline_query_pool)

    auto Device::info() const -> DeviceInfo const &
    {
        auto self = this->as<daxa_ImplDevice>();
        return *reinterpret_cast<DeviceInfo const *>(daxa_dvc_info(const_cast<daxa_Device>(self)));
    }

    auto Device::properties() const -> DeviceProperties const &
    {
        auto self = this->as<daxa_ImplDevice>();
        return *reinterpret_cast<DeviceProperties const *>(&self->vk_physical_device_properties2.properties);
    }

    auto Device::mesh_shader_properties() const -> MeshShaderDeviceProperties const &
    {
        auto self = this->as<daxa_ImplDevice>();
        return *reinterpret_cast<MeshShaderDeviceProperties const *>(&self->mesh_shader_properties);
    }

    void Device::wait_idle()
    {
        auto self = this->as<daxa_ImplDevice>();
        daxa_dvc_wait_idle(self);
    }

    void Device::submit_commands(CommandSubmitInfo const & submit_info)
    {
        auto self = this->as<daxa_ImplDevice>();
        daxa_CommandSubmitInfo c_submit_info = {
            .wait_stages = static_cast<VkPipelineStageFlags>(submit_info.wait_stages.data),
            .command_lists = reinterpret_cast<daxa_CommandList const *>(submit_info.command_lists.data()),
            .command_list_count = submit_info.command_lists.size(),
            .wait_binary_semaphores = reinterpret_cast<daxa_BinarySemaphore const *>(submit_info.wait_binary_semaphores.data()),
            .wait_binary_semaphore_count = submit_info.wait_binary_semaphores.size(),
            .signal_binary_semaphores = reinterpret_cast<daxa_BinarySemaphore const *>(submit_info.signal_binary_semaphores.data()),
            .signal_binary_semaphore_count = submit_info.signal_binary_semaphores.size(),
            .wait_timeline_semaphores = reinterpret_cast<daxa_TimelinePair const *>(submit_info.wait_timeline_semaphores.data()),
            .wait_timeline_semaphore_count = submit_info.wait_timeline_semaphores.size(),
            .signal_timeline_semaphores = reinterpret_cast<daxa_TimelinePair const *>(submit_info.signal_timeline_semaphores.data()),
            .signal_timeline_semaphore_count = submit_info.signal_timeline_semaphores.size(),
        };
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_dvc_submit(self, &c_submit_info) == DAXA_RESULT_SUCCESS,
            "failed to submit commands");
    }

    void Device::present_frame(PresentInfo const & info)
    {
        auto self = this->as<daxa_ImplDevice>();
        daxa_PresentInfo c_present_info = {
            .wait_binary_semaphores = reinterpret_cast<daxa_BinarySemaphore const *>(info.wait_binary_semaphores.data()),
            .wait_binary_semaphore_count = info.wait_binary_semaphores.size(),
            .swapchain = *reinterpret_cast<daxa_Swapchain const *>(&info.swapchain),
        };
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_dvc_present(self, &c_present_info) == DAXA_RESULT_SUCCESS,
            "failed to present frame");
    }

    void Device::collect_garbage()
    {
        auto self = this->as<daxa_ImplDevice>();
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_dvc_collect_garbage(self) == DAXA_RESULT_SUCCESS,
            "failed to collect garbage");
    }

    /// --- End Device ---

    /// --- Begin BinarySemaphore ---

    BinarySemaphore::BinarySemaphore(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}

    auto BinarySemaphore::info() const -> BinarySemaphoreInfo const &
    {
        auto self = this->as<daxa_ImplBinarySemaphore>();
        return *reinterpret_cast<BinarySemaphoreInfo const *>(
            daxa_binary_semaphore_info(const_cast<daxa_BinarySemaphore>(self)));
    }

    /// --- End BinarySemaphore ---

    /// --- Begin TimelineSemaphore ---

    TimelineSemaphore::TimelineSemaphore(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}

    auto TimelineSemaphore::info() const -> TimelineSemaphoreInfo const &
    {
        auto self = this->as<daxa_ImplTimelineSemaphore>();
        return *reinterpret_cast<TimelineSemaphoreInfo const *>(
            daxa_timeline_semaphore_info(const_cast<daxa_TimelineSemaphore>(self)));
    }

    auto TimelineSemaphore::value() const -> u64
    {
        auto self = this->as<daxa_ImplTimelineSemaphore>();
        u64 ret = {};
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_timeline_semaphore_get_value(const_cast<daxa_TimelineSemaphore>(self), &ret) == DAXA_RESULT_SUCCESS,
            "cant get timeline value");
        return ret;
    }

    void TimelineSemaphore::set_value(u64 value)
    {
        auto self = this->as<daxa_ImplTimelineSemaphore>();
        DAXA_DBG_ASSERT_TRUE_M(
            daxa_timeline_semaphore_set_value(self, value) == DAXA_RESULT_SUCCESS,
            "cant set timeline value");
    }

    auto TimelineSemaphore::wait_for_value(u64 value, u64 timeout_nanos) -> bool
    {
        auto self = this->as<daxa_ImplTimelineSemaphore>();
        auto result = daxa_timeline_semaphore_wait_for_value(self, value, timeout_nanos);
        DAXA_DBG_ASSERT_TRUE_M(result == DAXA_RESULT_SUCCESS || result == DAXA_RESULT_TIMEOUT, "could not wait on timeline");
        return result == DAXA_RESULT_SUCCESS;
    }

    /// --- End TimelineSemaphore ---

    /// --- Begin Event

    Event::Event(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}

    auto Event::info() const -> EventInfo const &
    {
        auto self = this->as<daxa_ImplEvent>();
        return *reinterpret_cast<EventInfo const *>(daxa_event_info(const_cast<daxa_Event>(self)));
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
