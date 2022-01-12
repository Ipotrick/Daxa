#pragma once

#include "Daxa.hpp"

#include "RenderContext.hpp"
#include "../Components.hpp"

class FrameBufferDebugRenderer {
public:
    struct UploadData {
        glm::mat4 inverseView;
        i32 imageWidth;
        i32 imageHeight;
        float near;
        float far;
    };

    void init(RenderContext& renderCTX, u32 width, u32 height) {
        auto shader = renderCTX.device->tryCreateShderModuleFromFile(
			"./DaxaMeshview/renderer/fb_debug.comp",
			VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT
		);
		if (shader.isErr()) {
			std::cout << "could not load vertex shader due to: " << shader.message() << std::endl;
		}

		this->pipeline = renderCTX.device->createComputePipeline(shader.value());

		this->setAlloc = renderCTX.device->createBindingSetAllocator(pipeline->getSetDescription(1), 4);

        this->buffer = renderCTX.device->createBuffer({
            .size = sizeof(UploadData),
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
        });

        auto cmdList = renderCTX.device->getEmptyCommandList();
        cmdList->begin();
        recreateImages(renderCTX, cmdList, width, height);
        cmdList->end();
        renderCTX.queue->submitBlocking({.commandLists = {cmdList}});
    }

    void recreateImages(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmdList, u32 width, u32 height) {
        auto ci = daxa::gpu::Image2dCreateInfo{
            .width = width,
            .height = height,
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .imageUsage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .sampler = renderCTX.defaultSampler,
        };
        debugScreenSpaceNormalImage = renderCTX.device->createImage2d(ci);
        debugWorldSpaceNormalImage = renderCTX.device->createImage2d(ci);
        debugLinearDepthImage = renderCTX.device->createImage2d(ci);

        cmdList->insertImageBarriers(std::array{
            daxa::gpu::ImageBarrier{
                .image = debugScreenSpaceNormalImage,
                .awaitedStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
                .waitingStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
                .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            },
            daxa::gpu::ImageBarrier{
                .image = debugWorldSpaceNormalImage,
                .awaitedStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
                .waitingStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
                .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            },
            daxa::gpu::ImageBarrier{
                .image = debugLinearDepthImage,
                .awaitedStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
                .waitingStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
                .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            }
        });
    }

    daxa::gpu::ImageHandle debugScreenSpaceNormalImage = {};
    daxa::gpu::ImageHandle debugWorldSpaceNormalImage = {};
    daxa::gpu::ImageHandle debugLinearDepthImage = {};
private:
    UploadData data;
    daxa::gpu::PipelineHandle pipeline = {};
    daxa::gpu::BindingSetAllocatorHandle setAlloc = {};
    daxa::gpu::BufferHandle buffer = {};
};