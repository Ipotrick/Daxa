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
        auto shader = renderCTX.device->createShderModule({
            .pathToSource = "./DaxaMeshview/renderer/fb_debug.comp",
            .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        });
		if (shader.isErr()) {
			std::cout << "could not load vertex shader due to: " << shader.message() << std::endl;
		}

		this->pipeline = renderCTX.device->createComputePipeline(shader.value(), "frame buffer debug pipeline");

		this->setAlloc = renderCTX.device->createBindingSetAllocator({
            .setDescription = pipeline->getSetDescription(0),
            .debugName = "frame buffer debug shader set allocator",
            .setPerPool = 4,
        });

        this->buffer = renderCTX.device->createBuffer({
            .size = sizeof(UploadData),
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
        });

        auto cmdList = renderCTX.device->getCommandList();
        recreateImages(renderCTX, cmdList, width, height);
        cmdList->finalize();
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
        ci.debugName = "debugScreenSpaceNormalImage";
        debugScreenSpaceNormalImage = renderCTX.device->createImage2d(ci);
        ci.debugName = "debugWorldSpaceNormalImage";
        debugWorldSpaceNormalImage = renderCTX.device->createImage2d(ci);
        ci.debugName = "debugLinearDepthImage";
        debugLinearDepthImage = renderCTX.device->createImage2d(ci);

        cmdList->insertImageBarriers(std::array{
            daxa::gpu::ImageBarrier{
                .image = debugScreenSpaceNormalImage,
                .awaitedStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
                .waitingStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
                .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            },
            daxa::gpu::ImageBarrier{
                .image = debugWorldSpaceNormalImage,
                .awaitedStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
                .waitingStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
                .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            },
            daxa::gpu::ImageBarrier{
                .image = debugLinearDepthImage,
                .awaitedStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
                .waitingStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
                .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            }
        });
    }
    
    void renderDebugViews(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmdList, UploadData uploadData) {
        cmdList->copyHostToBuffer({
            .src = (void*)&uploadData,
            .size = sizeof(decltype(uploadData)),
            .dst = buffer,
        });

        auto membar = std::array{ 
            daxa::gpu::MemoryBarrier{.awaitedStages = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT_KHR, .waitingStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR },
        };
        auto imgbar = std::array{
            daxa::gpu::ImageBarrier{
                .image = debugScreenSpaceNormalImage,
                .awaitedStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
                .waitingStages =  VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
                .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
            },
            daxa::gpu::ImageBarrier{
                .image = debugWorldSpaceNormalImage,
                .awaitedStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
                .waitingStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
                .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
            },
            daxa::gpu::ImageBarrier{
                .image = debugLinearDepthImage,
                .awaitedStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
                .waitingStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
                .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
            },
            daxa::gpu::ImageBarrier{
                .image = renderCTX.depthImage,
                .awaitedStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
                .waitingStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
                .layoutBefore = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            }
        };
        cmdList->insertBarriers(
            membar,
            imgbar
        );

        cmdList->bindPipeline(pipeline);

        auto set = setAlloc->getSet();
        set->bindBuffer(0, buffer);
        set->bindImage(1, renderCTX.depthImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        set->bindImage(2, renderCTX.normalsImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        set->bindImage(3, debugLinearDepthImage, VK_IMAGE_LAYOUT_GENERAL);
        set->bindImage(4, debugScreenSpaceNormalImage, VK_IMAGE_LAYOUT_GENERAL);
        set->bindImage(5, debugWorldSpaceNormalImage, VK_IMAGE_LAYOUT_GENERAL);

        cmdList->bindSet(0, set);

        cmdList->dispatch((uploadData.imageWidth + 1) / 8, (uploadData.imageHeight + 1) / 8);

        // TODO dispatch compute shader

        cmdList->insertImageBarriers(std::array{
            daxa::gpu::ImageBarrier{
                .image = debugScreenSpaceNormalImage,
                .awaitedStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
                .waitingStages =  VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
                .layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
                .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            },
            daxa::gpu::ImageBarrier{
                .image = debugWorldSpaceNormalImage,
                .awaitedStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
                .waitingStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
                .layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
                .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            },
            daxa::gpu::ImageBarrier{
                .image = debugLinearDepthImage,
                .awaitedStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
                .waitingStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
                .layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
                .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            },
            daxa::gpu::ImageBarrier{
                .image = renderCTX.depthImage,
                .awaitedStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
                .waitingStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
                .layoutAfter = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            },
        });
    }

    daxa::gpu::ImageHandle debugLinearDepthImage = {};
    daxa::gpu::ImageHandle debugScreenSpaceNormalImage = {};
    daxa::gpu::ImageHandle debugWorldSpaceNormalImage = {};
private:
    daxa::gpu::PipelineHandle pipeline = {};
    daxa::gpu::BindingSetAllocatorHandle setAlloc = {};
    daxa::gpu::BufferHandle buffer = {};
};