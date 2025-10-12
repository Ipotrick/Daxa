#include "daxa/core.hpp"
#if DAXA_BUILT_WITH_UTILS_FSR3

#include "../impl_core.hpp"

#include "impl_fsr3.hpp"

#include <ffx_api/vk/ffx_api_vk.h>
#include "ffx_api/ffx_api.h"
#include "ffx_api/ffx_upscale.h"

static auto DaxaImageInfoToVkImageCreateInfo(daxa_ImageInfo const & info) -> VkImageCreateInfo
{
    return VkImageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = static_cast<VkImageCreateFlags>(info.flags),
        .imageType = static_cast<VkImageType>(info.dimensions - 1),
        .format = static_cast<VkFormat>(info.format),
        .extent = std::bit_cast<VkExtent3D>(info.size),
        .mipLevels = info.mip_level_count,
        .arrayLayers = info.array_layer_count,
        .samples = static_cast<VkSampleCountFlagBits>(info.sample_count),
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = info.usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = 0,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
}

namespace daxa
{
    Fsr3Context::Fsr3Context(daxa::Instance & instance, daxa::Device & device)
    {
        this->object = new ImplFsr3Context(instance, device);
    }

    void Fsr3Context::resize(int outputSizeX, int outputSizeY, Mode mode)
    {
        auto & impl = *r_cast<ImplFsr3Context *>(this->object);
        impl.create_resizable_resources(outputSizeX, outputSizeY, mode);
    }

    void Fsr3Context::evaluate(daxa::TaskInterface const & ti, Fsr3Images const & images, Fsr3UpscaleInfo const & upscale_info)
    {
        auto & impl = *r_cast<ImplFsr3Context *>(this->object);
        impl.evaluate(ti, images, upscale_info);
    }

    auto Fsr3Context::get_jitter(u64 index) const -> daxa_f32vec2
    {
        auto const & impl = *r_cast<ImplFsr3Context *>(this->object);
        daxa_f32vec2 result{};
        
        auto jitter_query_desc = ffxQueryDescUpscaleGetJitterOffset{};
        jitter_query_desc.header.type = FFX_API_QUERY_DESC_TYPE_UPSCALE_GETJITTEROFFSET;
        jitter_query_desc.pOutX = &result.x;
        jitter_query_desc.pOutY = &result.y;
        jitter_query_desc.index = index % impl.phase_count;
        jitter_query_desc.phaseCount = impl.phase_count;
        auto context = impl.mFsrContext;
        auto queryRes = ffxQuery(&context, &jitter_query_desc.header);
        DAXA_DBG_ASSERT_TRUE_M(queryRes == FFX_API_RETURN_OK, "Failed to query for FSR jitter");

        return result;
    }

    ImplFsr3Context::ImplFsr3Context(daxa::Instance & instance, daxa::Device & device)
    {
        this->mDevice = device;
        auto backendDesc = ffxCreateBackendVKDesc{};
        backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_VK;
        backendDesc.header.pNext = nullptr;
        backendDesc.vkDevice = daxa_dvc_get_vk_device(device.get());
        backendDesc.vkPhysicalDevice = daxa_dvc_get_vk_physical_device(device.get());
        backendDesc.vkDeviceProcAddr = vkGetDeviceProcAddr;
        auto ctxDesc = ffxCreateContextDescUpscale{};
        ctxDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;
        ctxDesc.header.pNext = &backendDesc.header;
        ctxDesc.maxUpscaleSize = {3840, 2160};
        ctxDesc.maxRenderSize = {3840, 2160};
        ctxDesc.flags |= FFX_UPSCALE_ENABLE_DEPTH_INFINITE;
        ctxDesc.flags |= FFX_UPSCALE_ENABLE_DEPTH_INVERTED;
        ctxDesc.flags |= FFX_UPSCALE_ENABLE_HIGH_DYNAMIC_RANGE;
#if DEBUG_FSR
        ctxDesc.flags |= FFX_UPSCALE_ENABLE_DEBUG_CHECKING;
        ctxDesc.fpMessage = [](uint32_t type, wchar_t const * message)
        { std::wcout << "FFX message: " << message << std::endl; };
#endif
        auto allocCb = ffxAllocationCallbacks{};
        allocCb.alloc = [](void * pUserData, uint64_t size)
        { return malloc(size); };
        allocCb.dealloc = [](void * pUserData, void * ptr)
        { return free(ptr); };
        auto ctxResult = ffxCreateContext(&mFsrContext, &ctxDesc.header, &allocCb);
        DAXA_DBG_ASSERT_TRUE_M(ctxResult == FFX_API_RETURN_OK, "Failed to create FFX context");
        initialized = true;
    }

    ImplFsr3Context::~ImplFsr3Context() // NOLINT(bugprone-exception-escape)
    {
        destroy_resizable_resources();
    }

    void ImplFsr3Context::create_resizable_resources(int outputSizeX, int outputSizeY, Fsr3Context::Mode mode)
    {
        auto quality_query_desc = ffxQueryDescUpscaleGetRenderResolutionFromQualityMode{};
        switch (mode)
        {
        case Fsr3Context::MODE_ULTRA_PERF: quality_query_desc.qualityMode = FFX_UPSCALE_QUALITY_MODE_ULTRA_PERFORMANCE; break;
        case Fsr3Context::MODE_PERF: quality_query_desc.qualityMode = FFX_UPSCALE_QUALITY_MODE_PERFORMANCE; break;
        case Fsr3Context::MODE_BALANCED: quality_query_desc.qualityMode = FFX_UPSCALE_QUALITY_MODE_BALANCED; break;
        case Fsr3Context::MODE_QUALITY: quality_query_desc.qualityMode = FFX_UPSCALE_QUALITY_MODE_QUALITY; break;
        case Fsr3Context::MODE_NATIVE_AA: quality_query_desc.qualityMode = FFX_UPSCALE_QUALITY_MODE_NATIVEAA; break;
        default: DAXA_DBG_ASSERT_TRUE_M(false, "???");
        }
        quality_query_desc.header.type = FFX_API_QUERY_DESC_TYPE_UPSCALE_GETRENDERRESOLUTIONFROMQUALITYMODE;
        quality_query_desc.displayWidth = outputSizeX;
        quality_query_desc.displayHeight = outputSizeY;
        quality_query_desc.pOutRenderWidth = (uint32_t *)&mRenderWidth;
        quality_query_desc.pOutRenderHeight = (uint32_t *)&mRenderHeight;
        auto queryRes = ffxQuery(&mFsrContext, &quality_query_desc.header);
        DAXA_DBG_ASSERT_TRUE_M(queryRes == FFX_API_RETURN_OK, "Failed to query for FSR size");

        mOutputWidth = outputSizeX;
        mOutputHeight = outputSizeY;

        auto phase_count_query_desc = ffxQueryDescUpscaleGetJitterPhaseCount{};
        phase_count_query_desc.header.type = FFX_API_QUERY_DESC_TYPE_UPSCALE_GETJITTERPHASECOUNT;
        phase_count_query_desc.pOutPhaseCount = &this->phase_count;
        phase_count_query_desc.displayWidth = outputSizeX;
        phase_count_query_desc.renderWidth = mRenderWidth;
        queryRes = ffxQuery(&mFsrContext, &phase_count_query_desc.header);
        DAXA_DBG_ASSERT_TRUE_M(queryRes == FFX_API_RETURN_OK, "Failed to query for FSR jitter count");
    }

    void ImplFsr3Context::destroy_resizable_resources()
    {
        if (initialized)
        {
            auto allocCb = ffxAllocationCallbacks{};
            allocCb.alloc = [](void * pUserData, uint64_t size)
            { return malloc(size); };
            allocCb.dealloc = [](void * pUserData, void * ptr)
            { return free(ptr); };
            ffxDestroyContext(&mFsrContext, &allocCb);
        }
    }

    void ImplFsr3Context::evaluate(daxa::TaskInterface const & ti, Fsr3Images const & images, Fsr3UpscaleInfo const & upscale_info)
    {

#define HANDLE_RES(x)                   \
    {                                   \
        auto res = (x);                 \
        if (res != DAXA_RESULT_SUCCESS) \
        {                               \
            std::abort();               \
        }                               \
    }
        auto & recorder = ti.recorder;

        auto getFsrResource = [&ti](daxa::TaskImageView taskImageView, VkImageSubresourceRange subresourceRange, unsigned int width, unsigned int height, bool readWrite) -> FfxApiResource
        {
            auto const & taskUse = ti.get(taskImageView);
            auto daxaDevice = *reinterpret_cast<daxa_Device *>(&ti.device);
            VkImage vkImage;
            HANDLE_RES(daxa_dvc_get_vk_image(daxaDevice, std::bit_cast<daxa_ImageId>(taskUse.ids[0]), &vkImage));
            auto daxaImageInfo = daxa_ImageInfo{};
            daxa_dvc_info_image(daxaDevice, std::bit_cast<daxa_ImageId>(taskUse.ids[0]), &daxaImageInfo);
            auto desc = ffxApiGetImageResourceDescriptionVK(vkImage, DaxaImageInfoToVkImageCreateInfo(daxaImageInfo), 0);
            auto state = readWrite ? FFX_API_RESOURCE_STATE_UNORDERED_ACCESS : FFX_API_RESOURCE_STATE_PIXEL_COMPUTE_READ; // ?
            return ffxApiGetResourceVK(vkImage, desc, state);
        };

        auto srr = VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        auto resourceInput = getFsrResource(images.mRt, srr, mRenderWidth, mRenderHeight, false);
        auto resourceMv = getFsrResource(images.mMotion, srr, mRenderWidth, mRenderHeight, false);
        auto resourceDepth = getFsrResource(images.mDepth, srr, mRenderWidth, mRenderHeight, false);
        auto resourceOutput = getFsrResource(images.mOutput, srr, mOutputWidth, mOutputHeight, true);
        auto resourceExposure = getFsrResource(images.mExposure, srr, mRenderWidth, mRenderHeight, false);

        auto cmd = daxa_cmd_get_vk_command_buffer(*reinterpret_cast<daxa_CommandRecorder *>(&recorder));

        ffxDispatchDescUpscale dispatchDesc = {};
        dispatchDesc.header.type = FFX_API_DISPATCH_DESC_TYPE_UPSCALE;
        dispatchDesc.commandList = cmd;
        dispatchDesc.color = resourceInput;
        dispatchDesc.depth = resourceDepth;
        dispatchDesc.motionVectors = resourceMv;
        dispatchDesc.exposure = resourceExposure;
        dispatchDesc.output = resourceOutput;
        // TODO: Compute reactive mask properly
        // dispatch_desc.reactive = ...
        // dispatch_desc.transparencyAndComposition = ...

        dispatchDesc.jitterOffset = {-upscale_info.jitter.x, -upscale_info.jitter.y};
        dispatchDesc.motionVectorScale = {float(mRenderWidth), float(mRenderHeight)};
        dispatchDesc.renderSize = {uint32_t(mRenderWidth), uint32_t(mRenderHeight)};
        dispatchDesc.upscaleSize = {uint32_t(mOutputWidth), uint32_t(mOutputHeight)};

        dispatchDesc.preExposure = upscale_info.pre_exposure;
        dispatchDesc.cameraNear = upscale_info.near_plane;
        dispatchDesc.cameraFar = upscale_info.far_plane;
        dispatchDesc.cameraFovAngleVertical = upscale_info.vertical_fov;
        dispatchDesc.frameTimeDelta = upscale_info.dt * 1000.0f;

        if (upscale_info.debug)
            dispatchDesc.flags = FFX_UPSCALE_FLAG_DRAW_DEBUG_VIEW;

        auto result = ffxDispatch(&mFsrContext, &dispatchDesc.header);
        DAXA_DBG_ASSERT_TRUE_M(result == FFX_API_RETURN_OK, "Failed to dispatch FSR3");
    }

    void ImplFsr3Context::zero_ref_callback(ImplHandle const * handle)
    {
        auto self = r_cast<ImplFsr3Context const *>(handle);

        delete self;
    }

    auto Fsr3Context::inc_refcnt(ImplHandle const * object) -> u64
    {
        return object->inc_refcnt();
    }

    auto Fsr3Context::dec_refcnt(ImplHandle const * object) -> u64
    {
        return object->dec_refcnt(
            ImplFsr3Context::zero_ref_callback,
            nullptr);
    }
} // namespace daxa

#endif
