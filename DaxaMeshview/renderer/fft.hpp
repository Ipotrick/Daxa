#include "RenderContext.hpp"

class FFT{
public:
    void init(RenderContext& renderCTX, u32 width, u32 height) {
        auto cmd = renderCTX.queue->getCommandList({});
        recreateImages(renderCTX, cmd, width, height);
        cmd->finalize();
        renderCTX.queue->submit({ .commandLists = {cmd}});

        fftStripPipeline = renderCTX.pipelineCompiler->createComputePipeline({
            .shaderCI = {
                .pathToSource = "fftStripPass.comp",
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .debugName = "fft strip",
            },
            .debugName = "fft strip",
        }).value();
    }

    void recreateImages(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd, u32 width, u32 height) {
        std::array<daxa::gpu::ImageViewHandle*, 9> images = { &rowResultR, &rowResultG, &rowResultB, &colResultR, &colResultG, &colResultB, &freqImageR, &freqImageG, &freqImageB };
        std::array<char const*, 9> imageNames = { "rowResultR", "rowResultG", "rowResultB", "colResultR", "colResultG", "colResultB", "freqImageR", "freqImageG", "freqImageB" };
        for (int i = 0; i < 9; i++) {
            *images[i] = renderCTX.device->createImageView({
                .image = renderCTX.device->createImage({
                    .extent = { width, height, 1 },
                    .format = VK_FORMAT_R16G16_SFLOAT,
                    .usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
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
    }

    void update(RenderContext& renderCTX) {
		if (renderCTX.pipelineCompiler->checkIfSourcesChanged(fftStripPipeline)) {
            auto result = renderCTX.pipelineCompiler->recreatePipeline(fftStripPipeline);
            std::cout << result << std::endl;
            if (result.isOk()) {
                fftStripPipeline = std::move(result.value());
            }
        }
    }
private:
    daxa::gpu::ImageViewHandle rowResultR = {};
    daxa::gpu::ImageViewHandle rowResultG = {};
    daxa::gpu::ImageViewHandle rowResultB = {};
    daxa::gpu::ImageViewHandle colResultR = {};
    daxa::gpu::ImageViewHandle colResultG = {};
    daxa::gpu::ImageViewHandle colResultB = {};
    daxa::gpu::ImageViewHandle freqImageR = {};
    daxa::gpu::ImageViewHandle freqImageG = {};
    daxa::gpu::ImageViewHandle freqImageB = {};

    daxa::gpu::PipelineHandle fftStripPipeline = {};
};