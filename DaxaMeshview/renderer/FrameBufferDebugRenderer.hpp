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
        auto shader = renderCTX.device->createShaderModule({
            .pathToSource = "./DaxaMeshview/renderer/fb_debug.comp",
            .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        });
		if (shader.isErr()) {
			std::cout << "could not load vertex shader due to: " << shader.message() << std::endl;
		}

		this->pipeline = renderCTX.device->createComputePipeline({
            .shaderModule = shader.value(), 
            .debugName = "frame buffer debug pipeline",
        }).value(); 

		this->setAlloc = renderCTX.device->createBindingSetAllocator({
            .setDescription = pipeline->getSetDescription(0),
            .debugName = "frame buffer debug shader set allocator",
            .setPerPool = 4,
        });

        this->buffer = renderCTX.device->createBuffer({
            .size = sizeof(UploadData),
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
            .memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .debugName = "frame buffer debug renderer global buffer"
        });

        auto cmdList = renderCTX.device->getCommandList();
        recreateImages(renderCTX, cmdList, width, height);
        cmdList->finalize();
        renderCTX.queue->submitBlocking({.commandLists = {cmdList}});

        hotLoader = daxa::ComputePipelineHotLoader{
            renderCTX.device,
            {
                .shaderModule = shader.value(), 
                .debugName = "frame buffer debug pipeline",
            },
            {
                .pathToSource = "./DaxaMeshview/renderer/fb_debug.comp",
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
            }
        };
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

        daxa::gpu::MemoryBarrier postMemBar = {
            .dstStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
            .dstAccess = VK_ACCESS_2_MEMORY_READ_BIT_KHR | VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
        };

        cmdList->insertImageBarriers(std::array{
            daxa::gpu::ImageBarrier{
                .barrier = postMemBar,
                .image = debugScreenSpaceNormalImage,
                .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            },
            daxa::gpu::ImageBarrier{
                .barrier = postMemBar,
                .image = debugWorldSpaceNormalImage,
                .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            },
            daxa::gpu::ImageBarrier{
                .barrier = postMemBar,
                .image = debugLinearDepthImage,
                .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            }
        });
    }
    
    void renderDebugViews(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmdList, CameraData cameraData) {
        if (auto newPipeline = hotLoader.getNewIfChanged(); newPipeline.has_value()) {
            printf("hot laoded\n");
            this->pipeline = newPipeline.value();
        }

        uploadData.cameraData = cameraData;
        uploadData.imageWidth = debugLinearDepthImage->getVkExtent().width;
        uploadData.imageHeight = debugLinearDepthImage->getVkExtent().height;
        uploadData.zMin = std::min(std::max(cameraData.near, uploadData.zMin), cameraData.far);
        uploadData.zMax = std::min(std::max(cameraData.near, uploadData.zMax), cameraData.far);
        cmdList->copyHostToBuffer({
            .src = (void*)&uploadData,
            .dst = buffer,
            .size = sizeof(decltype(uploadData)),
        });

        auto copyMemBarr = std::array{ 
            daxa::gpu::MemoryBarrier{
                .srcStages = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT_KHR, 
                .srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR,
                .dstStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
                .dstAccess = VK_ACCESS_2_SHADER_READ_BIT_KHR,
            },
        };

        daxa::gpu::MemoryBarrier preMemBarr{
            .srcStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
            .srcAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
            .dstStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
            .dstAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT_KHR | VK_ACCESS_2_SHADER_STORAGE_READ_BIT_KHR | VK_ACCESS_2_SHADER_SAMPLED_READ_BIT_KHR,
        };

        auto imgbar = std::array{
            daxa::gpu::ImageBarrier{
                .barrier = preMemBarr,
                .image = debugScreenSpaceNormalImage,
                .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
            },
            daxa::gpu::ImageBarrier{
                .barrier = preMemBarr,
                .image = debugWorldSpaceNormalImage,
                .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
            },
            daxa::gpu::ImageBarrier{
                .barrier = preMemBarr,
                .image = debugLinearDepthImage,
                .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
            },
            daxa::gpu::ImageBarrier{
                .barrier = preMemBarr,
                .image = renderCTX.depthImage,
                .layoutBefore = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            },
        };
        cmdList->insertBarriers(
            copyMemBarr,
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
        cmdList->unbindPipeline();

        // TODO dispatch compute shader

        daxa::gpu::MemoryBarrier postMemBarr {
            .srcStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
            .srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT_KHR,
            .dstStages =  VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
            .dstAccess = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT_KHR,
        };

        cmdList->insertImageBarriers(std::array{
            daxa::gpu::ImageBarrier{
                .barrier = postMemBarr,
                .image = debugScreenSpaceNormalImage,
                .layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
                .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            },
            daxa::gpu::ImageBarrier{
                .barrier = postMemBarr,
                .image = debugWorldSpaceNormalImage,
                .layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
                .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            },
            daxa::gpu::ImageBarrier{
                .barrier = postMemBarr,
                .image = debugLinearDepthImage,
                .layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
                .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            },
            daxa::gpu::ImageBarrier{
                .barrier = postMemBarr,
                .image = renderCTX.depthImage,
                .layoutAfter = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            },
        });
    }

	void doGui(daxa::ImGuiRenderer& imguiRenderer) {
		ImGui::Begin("frame buffer inspector");
		ImGui::Text("screenspace normals");
        f32 frameBufferWidth = debugLinearDepthImage->getVkExtent().width;
        f32 frameBufferHeight = debugLinearDepthImage->getVkExtent().height;
		f32 w = ImGui::GetWindowWidth() - 10;
		f32 aspect = (f32)frameBufferHeight / (f32)frameBufferWidth;
		f32 h = w * aspect;
		auto id = imguiRenderer.getImGuiTextureId(debugScreenSpaceNormalImage);
		ImGui::Image((void*)id, ImVec2(w, h));
		ImGui::Text("worldspace normals");
		id = imguiRenderer.getImGuiTextureId(debugWorldSpaceNormalImage);
		ImGui::Image((void*)id, ImVec2(w, h));
		ImGui::Text("depth");
        ImGui::SliderFloat("z min", &uploadData.zMin, uploadData.cameraData.near, uploadData.cameraData.far);
        ImGui::SliderFloat("z max", &uploadData.zMax, uploadData.cameraData.near, uploadData.cameraData.far);
		id = imguiRenderer.getImGuiTextureId(debugLinearDepthImage);
		ImGui::Image((void*)id, ImVec2(w, h));
		ImGui::End();
	}

    daxa::gpu::ImageHandle debugLinearDepthImage = {};
    daxa::gpu::ImageHandle debugScreenSpaceNormalImage = {};
    daxa::gpu::ImageHandle debugWorldSpaceNormalImage = {};
private:
    UploadData uploadData;
    daxa::ComputePipelineHotLoader hotLoader;
    daxa::gpu::PipelineHandle pipeline = {};
    daxa::gpu::BindingSetAllocatorHandle setAlloc = {};
    daxa::gpu::BufferHandle buffer = {};
};