#include "RenderContext.hpp"

class FFT{
public:
    struct StripPush{
        u32 inImageId;
        u32 outImageId;
        u32 width;
        u32 height;
        // 0 => r
        // 1 => g
        // 2 => b
        u32 pass;   
        u32 logN;
    };

    void init(RenderContext& renderCTX, u32 width, u32 height) {
        auto cmd = renderCTX.queue->getCommandList({});
        recreateImages(renderCTX, cmd, width, height);
        cmd->finalize();
        renderCTX.queue->submit({ .commandLists = {cmd}});

        fft1024Horizontal = renderCTX.pipelineCompiler->createComputePipeline({
            .shaderCI = {
                .pathToSource = "fft1024horizonal.comp",
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .debugName = "fft1024horizonal",
            },
            .overwriteSets = {daxa::gpu::BIND_ALL_SET_DESCRIPTION},
            .debugName = "fft1024horizonal",
        }).value();
        fft1024Vertical = renderCTX.pipelineCompiler->createComputePipeline({
            .shaderCI = {
                .pathToSource = "fft1024vertical.comp",
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .debugName = "fft1024vertical",
            },
            .overwriteSets = {daxa::gpu::BIND_ALL_SET_DESCRIPTION},
            .debugName = "fft1024vertical",
        }).value();
        fftCombine = renderCTX.pipelineCompiler->createComputePipeline({
            .shaderCI = {
                .pathToSource = "fftCombine.comp",
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .debugName = "fftCombine",
            },
            .overwriteSets = {daxa::gpu::BIND_ALL_SET_DESCRIPTION},
            .debugName = "fftCombine",
        }).value();
        fftDebug = renderCTX.pipelineCompiler->createComputePipeline({
            .shaderCI = {
                .pathToSource = "fftDebug.comp",
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .debugName = "fftDebug",
            },
            .overwriteSets = {daxa::gpu::BIND_ALL_SET_DESCRIPTION},
            .debugName = "fftDebug",
        }).value();
    }

    void recreateImages(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd, u32 width, u32 height) {
        std::array<daxa::gpu::ImageViewHandle*, 9> images = { &verResultR, &verResultG, &verResultB, &horResultR, &horResultG, &horResultB, &freqImageR, &freqImageG, &freqImageB };
        std::array<char const*, 9> imageNames = { "verResultR", "verResultG", "verResultB", "horResultR", "horResultG", "horResultB", "freqImageR", "freqImageG", "freqImageB" };
        for (int i = 0; i < 9; i++) {
            *images[i] = renderCTX.device->createImageView({
                .image = renderCTX.device->createImage({
                    .format = VK_FORMAT_R16G16_SFLOAT,
                    .extent = { width, height, 1 },
                    .usage = VK_IMAGE_USAGE_STORAGE_BIT,
                    .debugName = imageNames[i],
                }),
                .format = VK_FORMAT_R16G16_SFLOAT,
                .defaultSampler = renderCTX.defaultSampler,
                .debugName = imageNames[i]
            });
            cmd->queueImageBarrier({
                .image = *images[i],
                .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
                .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
            });
        }
        fftDebugImage = renderCTX.device->createImageView({
            .image = renderCTX.device->createImage({
                .format = VK_FORMAT_R8G8B8A8_UNORM,
                .extent = { width, height, 1},
                .usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                .debugName = "fftDebugImage",
            }),
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .defaultSampler = renderCTX.defaultSampler,
            .debugName = "fftDebugImage",
        });
        cmd->queueImageBarrier({
            .image = fftDebugImage,
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
    }

    void horizontalPass(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd) {
		if (renderCTX.pipelineCompiler->checkIfSourcesChanged(fft1024Horizontal)) {
            auto result = renderCTX.pipelineCompiler->recreatePipeline(fft1024Horizontal);
            std::cout << result << std::endl;
            if (result.isOk()) {
                fft1024Horizontal = std::move(result.value());
            }
        }
        u32 width = renderCTX.swapchainImage.getImageViewHandle()->getImageHandle()->getVkExtent3D().width;
        u32 height = renderCTX.swapchainImage.getImageViewHandle()->getImageHandle()->getVkExtent3D().height;
        u32 log2Width = std::log2(width);
        u32 log2Height = std::log2(height);
        // horizontal pass
        cmd->bindPipeline(fft1024Horizontal);
        cmd->bindAll();
        StripPush spush{
            renderCTX.swapchainImage.getImageViewHandle()->getDescriptorIndex(),
            horResultR->getDescriptorIndex(),
            width,
            height,
            0,
            log2Width,
        };
        cmd->pushConstant(VK_SHADER_STAGE_COMPUTE_BIT, spush);
        cmd->dispatch(1, height, 1);
        spush = {
            renderCTX.swapchainImage.getImageViewHandle()->getDescriptorIndex(),
            horResultG->getDescriptorIndex(),
            width,
            height,
            1,
            log2Width,
        };
        cmd->pushConstant(VK_SHADER_STAGE_COMPUTE_BIT, spush);
        cmd->dispatch(1, height, 1);
        spush = {
            renderCTX.swapchainImage.getImageViewHandle()->getDescriptorIndex(),
            horResultB->getDescriptorIndex(),
            width,
            height,
            2,
            log2Width,
        };
        cmd->pushConstant(VK_SHADER_STAGE_COMPUTE_BIT, spush);
        cmd->dispatch(1, height, 1);
        cmd->unbindPipeline();
    }

    void verticalPass(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd) {
		if (renderCTX.pipelineCompiler->checkIfSourcesChanged(fft1024Vertical)) {
            auto result = renderCTX.pipelineCompiler->recreatePipeline(fft1024Vertical);
            std::cout << result << std::endl;
            if (result.isOk()) {
                fft1024Vertical = std::move(result.value());
            }
        }
        u32 width = renderCTX.swapchainImage.getImageViewHandle()->getImageHandle()->getVkExtent3D().width;
        u32 height = renderCTX.swapchainImage.getImageViewHandle()->getImageHandle()->getVkExtent3D().height;
        u32 log2Width = std::log2(width);
        u32 log2Height = std::log2(height);
        // horizontal pass
        cmd->bindPipeline(fft1024Vertical);
        cmd->bindAll();
        StripPush spush{
            renderCTX.swapchainImage.getImageViewHandle()->getDescriptorIndex(),
            verResultR->getDescriptorIndex(),
            width,
            height,
            0,
            log2Height,
        };
        cmd->pushConstant(VK_SHADER_STAGE_COMPUTE_BIT, spush);
        cmd->dispatch(width, 1, 1);
        spush = {
            renderCTX.swapchainImage.getImageViewHandle()->getDescriptorIndex(),
            verResultG->getDescriptorIndex(),
            width,
            height,
            1,
            log2Height,
        };
        cmd->pushConstant(VK_SHADER_STAGE_COMPUTE_BIT, spush);
        cmd->dispatch(width, 1, 1);
        spush = {
            renderCTX.swapchainImage.getImageViewHandle()->getDescriptorIndex(),
            verResultB->getDescriptorIndex(),
            width,
            height,
            2,
            log2Height,
        };
        cmd->pushConstant(VK_SHADER_STAGE_COMPUTE_BIT, spush);
        cmd->dispatch(width, 1, 1);
        cmd->unbindPipeline();
    }

    void combinePass(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd) {
		if (renderCTX.pipelineCompiler->checkIfSourcesChanged(fftCombine)) {
            auto result = renderCTX.pipelineCompiler->recreatePipeline(fftCombine);
            std::cout << result << std::endl;
            if (result.isOk()) {
                fftCombine = std::move(result.value());
            }
        }
        u32 width = renderCTX.swapchainImage.getImageViewHandle()->getImageHandle()->getVkExtent3D().width;
        u32 height = renderCTX.swapchainImage.getImageViewHandle()->getImageHandle()->getVkExtent3D().height;
        cmd->queueMemoryBarrier(daxa::gpu::FULL_MEMORY_BARRIER);
        cmd->bindPipeline(fftCombine);
        cmd->bindAll();
        struct Push{
            u32 rowImageId;
            u32 colImageId;
            u32 outImageId;
            u32 width;
            u32 height;
        };
        Push push{
            horResultR->getDescriptorIndex(),
            verResultR->getDescriptorIndex(),
            freqImageR->getDescriptorIndex(),
            width,
            height
        };
        cmd->pushConstant(VK_SHADER_STAGE_COMPUTE_BIT, push);
        cmd->dispatch(width / 8 + 1, height / 8 + 1, 1);
        push = {
            horResultG->getDescriptorIndex(),
            verResultG->getDescriptorIndex(),
            freqImageG->getDescriptorIndex(),
            width,
            height
        };
        cmd->pushConstant(VK_SHADER_STAGE_COMPUTE_BIT, push);
        cmd->dispatch(width / 8 + 1, height / 8 + 1, 1);
        push = {
            horResultB->getDescriptorIndex(),
            verResultB->getDescriptorIndex(),
            freqImageB->getDescriptorIndex(),
            width,
            height
        };
        cmd->pushConstant(VK_SHADER_STAGE_COMPUTE_BIT, push);
        cmd->dispatch(width / 8 + 1, height / 8 + 1, 1);
        cmd->unbindPipeline();
    }

    void debugPass(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd) {
		if (renderCTX.pipelineCompiler->checkIfSourcesChanged(fftDebug)) {
            auto result = renderCTX.pipelineCompiler->recreatePipeline(fftDebug);
            std::cout << result << std::endl;
            if (result.isOk()) {
                fftDebug = std::move(result.value());
            }
        }
        u32 width = renderCTX.swapchainImage.getImageViewHandle()->getImageHandle()->getVkExtent3D().width;
        u32 height = renderCTX.swapchainImage.getImageViewHandle()->getImageHandle()->getVkExtent3D().height;
        cmd->queueImageBarrier({
            .image = fftDebugImage,
            .layoutBefore = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
        });
        cmd->bindPipeline(fftDebug);
        cmd->bindAll();
        struct Push{
            u32 rImageId;
            u32 gImageId;
            u32 bImageId;
            u32 outImageId;
            u32 width;
            u32 height;
        };
        Push push{
            freqImageR->getDescriptorIndex(),
            freqImageG->getDescriptorIndex(),
            freqImageB->getDescriptorIndex(),
            fftDebugImage->getDescriptorIndex(),
            width,
            height
        };
        cmd->pushConstant(VK_SHADER_STAGE_COMPUTE_BIT, push);
        cmd->dispatch(width/8 + 1, height/8 + 1, 1);
        cmd->unbindPipeline();
        cmd->queueImageBarrier({
            .image = fftDebugImage,
            .layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
            .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
    }

    void update(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd) {
        cmd->queueImageBarrier({
            .image = renderCTX.swapchainImage.getImageViewHandle(),
            .layoutBefore = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
        horizontalPass(renderCTX, cmd);
        verticalPass(renderCTX, cmd);
        cmd->queueImageBarrier({
            .image = renderCTX.swapchainImage.getImageViewHandle(),
            .layoutBefore = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        });
        combinePass(renderCTX, cmd);
        debugPass(renderCTX, cmd);
    }
    
    daxa::gpu::ImageViewHandle fftDebugImage = {};
private:
    daxa::gpu::ImageViewHandle horResultR = {};
    daxa::gpu::ImageViewHandle horResultG = {};
    daxa::gpu::ImageViewHandle horResultB = {};
    daxa::gpu::ImageViewHandle verResultR = {};
    daxa::gpu::ImageViewHandle verResultG = {};
    daxa::gpu::ImageViewHandle verResultB = {};
    daxa::gpu::ImageViewHandle freqImageR = {};
    daxa::gpu::ImageViewHandle freqImageG = {};
    daxa::gpu::ImageViewHandle freqImageB = {};

    daxa::gpu::PipelineHandle fft1024Horizontal = {};
    daxa::gpu::PipelineHandle fft1024Vertical = {};
    daxa::gpu::PipelineHandle fftCombine = {};
    daxa::gpu::PipelineHandle fftDebug = {};
};