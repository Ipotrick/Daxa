#pragma once

#include "Daxa.hpp"

#include "RenderContext.hpp"
#include "../Components.hpp"

class FrameBufferDebugRenderer {
public:
    struct CameraData {
        glm::mat4 inverseView;
        glm::mat4 inverseTransposeVP;
        f32 near;
        f32 far;
    };
    struct UploadData {
        CameraData cameraData;
        i32 imageWidth;
        i32 imageHeight;
        f32 zMin = 0.1f;
        f32 zMax = 10.0f;
    };

    void init(RenderContext& renderCTX, u32 width, u32 height) {
        daxa::ShaderModuleCreateInfo shaderCI{
            .pathToSource = "./DaxaMeshview/renderer/fb_debug.comp", .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        };

		this->pipeline = renderCTX.pipelineCompiler->createComputePipeline({
            .shaderCI = shaderCI, 
            .debugName = "frame buffer debug pipeline",
        }).value(); 

		this->setAlloc = renderCTX.device->createBindingSetAllocator({
            .setLayout = pipeline->getSetLayout(0),
            .setPerPool = 4,
            .debugName = "frame buffer debug shader set allocator",
        });

        this->buffer = renderCTX.device->createBuffer({
            .size = sizeof(UploadData),
            //.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            //.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
            //.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .debugName = "frame buffer debug renderer global buffer"
        });

        auto cmdList = renderCTX.queue->getCommandList({});
        recreateImages(renderCTX, cmdList, width, height);
        cmdList.finalize();
        renderCTX.queue->submitBlocking({.commandLists = {cmdList}});
    }

    void recreateImages(RenderContext& renderCTX, daxa::CommandListHandle& cmdList, u32 width, u32 height) {
        debugScreenSpaceNormalImage = renderCTX.device->createImageView(daxa::ImageViewCreateInfo{
            .image = renderCTX.device->createImage({
                .format = VK_FORMAT_R8G8B8A8_UNORM,
                .extent = { width, height, 1 },
                .usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            }),
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .defaultSampler = renderCTX.defaultSampler,
            .debugName = "debugScreenSpaceNormalImage",
        });
        debugWorldSpaceNormalImage = renderCTX.device->createImageView(daxa::ImageViewCreateInfo{
            .image = renderCTX.device->createImage({
                .format = VK_FORMAT_R8G8B8A8_UNORM,
                .extent = { width, height, 1 },
                .usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            }),
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .defaultSampler = renderCTX.defaultSampler,
            .debugName = "debugWorldSpaceNormalImage",
        });
        debugLinearDepthImage = renderCTX.device->createImageView(daxa::ImageViewCreateInfo{
            .image = renderCTX.device->createImage({
                .format = VK_FORMAT_R8G8B8A8_UNORM,
                .extent = { width, height, 1 },
                .usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            }),
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .defaultSampler = renderCTX.defaultSampler,
            .debugName = "debugLinearDepthImage",
        });

        daxa::MemoryBarrier postMemBar = {
            .dstStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
            .dstAccess = VK_ACCESS_2_MEMORY_READ_BIT_KHR | VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
        };

        cmdList.queueImageBarrier({
            .barrier = postMemBar,
            .image = debugScreenSpaceNormalImage,
            .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
        cmdList.queueImageBarrier({
                .barrier = postMemBar,
                .image = debugWorldSpaceNormalImage,
                .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
        cmdList.queueImageBarrier({
                .barrier = postMemBar,
                .image = debugLinearDepthImage,
                .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
    }
    
    void renderDebugViews(RenderContext& renderCTX, daxa::CommandListHandle& cmdList, CameraData cameraData) {
        uploadData.cameraData = cameraData;
        uploadData.imageWidth = static_cast<i32>(debugLinearDepthImage->getImageHandle()->getVkExtent3D().width);
        uploadData.imageHeight = static_cast<i32>(debugLinearDepthImage->getImageHandle()->getVkExtent3D().height);
        uploadData.zMin = std::min(std::max(cameraData.near, uploadData.zMin), cameraData.far);
        uploadData.zMax = std::min(std::max(cameraData.near, uploadData.zMax), cameraData.far);
        cmdList.singleCopyHostToBuffer({
            .src = reinterpret_cast<u8*>(&uploadData),
            .dst = buffer,
            .region = {.size = sizeof(decltype(uploadData)) }
        });

        cmdList.queueMemoryBarrier({
            .srcStages = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT_KHR, 
            .srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR,
            .dstStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
            .dstAccess = VK_ACCESS_2_SHADER_READ_BIT_KHR,
        });

        daxa::MemoryBarrier preMemBarr{
            .srcStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
            .srcAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
            .dstStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
            .dstAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT_KHR | VK_ACCESS_2_SHADER_STORAGE_READ_BIT_KHR | VK_ACCESS_2_SHADER_SAMPLED_READ_BIT_KHR,
        };

        cmdList.queueImageBarrier({
            .barrier = preMemBarr,
            .image = debugScreenSpaceNormalImage,
            .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
        });
        cmdList.queueImageBarrier({
                .barrier = preMemBarr,
                .image = debugWorldSpaceNormalImage,
                .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
        });
        cmdList.queueImageBarrier({
                .barrier = preMemBarr,
                .image = debugLinearDepthImage,
                .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
        });
        cmdList.queueImageBarrier({
                .barrier = preMemBarr,
                .image = renderCTX.depthImage,
                .layoutBefore = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });

        cmdList.bindPipeline(pipeline);

        auto set = setAlloc->getSet();
        set->bindBuffer(0, buffer);
        set->bindImage(1, renderCTX.depthImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        set->bindImage(2, renderCTX.normalsImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        set->bindImage(3, debugLinearDepthImage, VK_IMAGE_LAYOUT_GENERAL);
        set->bindImage(4, debugScreenSpaceNormalImage, VK_IMAGE_LAYOUT_GENERAL);
        set->bindImage(5, debugWorldSpaceNormalImage, VK_IMAGE_LAYOUT_GENERAL);
        cmdList.bindSet(0, set);

        cmdList.dispatch(static_cast<u32>(uploadData.imageWidth + 1) / 8, static_cast<u32>(uploadData.imageHeight + 1) / 8);
        cmdList.unbindPipeline();

        // TODO dispatch compute shader

        daxa::MemoryBarrier postMemBarr {
            .srcStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
            .srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT_KHR,
            .dstStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
            .dstAccess = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT_KHR,
        };

        cmdList.queueImageBarrier({
            .barrier = postMemBarr,
            .image = debugScreenSpaceNormalImage,
            .layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
            .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
        cmdList.queueImageBarrier({
            .barrier = postMemBarr,
            .image = debugWorldSpaceNormalImage,
            .layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
            .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
        cmdList.queueImageBarrier({
            .barrier = postMemBarr,
            .image = debugLinearDepthImage,
            .layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
            .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
        cmdList.queueImageBarrier({
            .barrier = postMemBarr,
            .image = renderCTX.depthImage,
            .layoutAfter = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        });
    }

	void doGui(daxa::ImGuiRenderer& imguiRenderer) {
		ImGui::Begin("frame buffer inspector");
		ImGui::Text("screenspace normals");
        f32 frameBufferWidth = static_cast<f32>(debugLinearDepthImage->getImageHandle()->getVkExtent3D().width);
        f32 frameBufferHeight = static_cast<f32>(debugLinearDepthImage->getImageHandle()->getVkExtent3D().height);
		f32 w = ImGui::GetWindowWidth() - 10;
		f32 aspect = frameBufferHeight / frameBufferWidth;
		f32 h = w * aspect;
		auto id = imguiRenderer.getImGuiTextureId(debugScreenSpaceNormalImage);
		ImGui::Image(reinterpret_cast<void*>(id), ImVec2(w, h));
		ImGui::Text("worldspace normals");
		id = imguiRenderer.getImGuiTextureId(debugWorldSpaceNormalImage);
		ImGui::Image(reinterpret_cast<void*>(id), ImVec2(w, h));
		ImGui::Text("depth");
        ImGui::SliderFloat("z min", &uploadData.zMin, uploadData.cameraData.near, uploadData.cameraData.far);
        ImGui::SliderFloat("z max", &uploadData.zMax, uploadData.cameraData.near, uploadData.cameraData.far);
		id = imguiRenderer.getImGuiTextureId(debugLinearDepthImage);
		ImGui::Image(reinterpret_cast<void*>(id), ImVec2(w, h));
		ImGui::End();
	}

    daxa::ImageViewHandle debugLinearDepthImage = {};
    daxa::ImageViewHandle debugScreenSpaceNormalImage = {};
    daxa::ImageViewHandle debugWorldSpaceNormalImage = {};
private:
    UploadData uploadData;
    daxa::PipelineHandle pipeline = {};
    daxa::BindingSetAllocatorHandle setAlloc = {};
    daxa::BufferHandle buffer = {};
};