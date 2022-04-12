#include "RenderContext.hpp"

class FFT{
public:
    constexpr static inline VkFormat IMAGE_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT;

    struct FFTGlobals {

    };

    struct CombinePush {

    };

    struct HorPush{

    };

    struct VertApplyPush{

    };

    void uploadData(RenderContext& renderCTX, daxa::CommandListHandle& cmd) {
        daxa::ImageViewHandle fftImage = {};
        daxa::ImageViewHandle horFreqImageR = {};
        daxa::ImageViewHandle horFreqImageG = {};
        daxa::ImageViewHandle horFreqImageB = {};
    }

    void init(RenderContext& renderCTX, daxa::CommandListHandle& cmd, u32 width, u32 height) {
        fftImage = renderCTX.device->createImageView({
            .image = renderCTX.device->createImage({
                .format = IMAGE_FORMAT,
                .extent = { width, height, 1 },
                .usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                .imageType = VK_IMAGE_TYPE_2D,
                .debugName = "fftImage",
            }),
            .format = IMAGE_FORMAT,
            .defaultSampler = renderCTX.defaultSampler,
            .debugName = "fftImage",
        });
        horFreqImageR = {};
        horFreqImageG = {};
        horFreqImageB = {};

        cmd.queueImageBarrier({
            .image = fftImage,
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
        cmd.insertQueuedBarriers();

        fftExtract = renderCTX.pipelineCompiler->createComputePipeline({
            .shaderCI = {
                .entryPoint = "Main",
                .pathToSource = "fft/fftextract.hlsl",
                .shaderLang = daxa::ShaderLang::HLSL,
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .debugName = "fftExtract",
            },
            .debugName = "fftExtract",
        }).value();
    }

    void recreateImages(RenderContext& renderCTX, daxa::CommandListHandle& cmd, u32 width, u32 height) {
    }

    void extractPass(RenderContext& renderCTX, daxa::CommandListHandle& cmd) {
        renderCTX.pipelineCompiler->recreateIfChanged(fftExtract);
    }

    void horizontalPass(RenderContext& renderCTX, daxa::CommandListHandle& cmd) {

    }

    void verticalApplyPass(RenderContext& renderCTX, daxa::CommandListHandle& cmd) {
    }

    void combinePass(RenderContext& renderCTX, daxa::CommandListHandle& cmd) {
    }

    void update(RenderContext& renderCTX, daxa::CommandListHandle& cmd) {
        cmd.queueImageBarrier({
            .image = fftImage,
            .layoutBefore = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
        });
        cmd.insertQueuedBarriers();

        extractPass(renderCTX, cmd);
        
        cmd.queueImageBarrier({
            .image = fftImage,
            .layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
            .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
    }
    
    u32 width = 512;
    u32 height = 512;

    daxa::ImageViewHandle fftImage = {};
    daxa::ImageViewHandle horFreqImageR = {};
    daxa::ImageViewHandle horFreqImageG = {};
    daxa::ImageViewHandle horFreqImageB = {};

    daxa::PipelineHandle fftExtract = {};
    daxa::PipelineHandle fft512Horizontal = {};
    daxa::PipelineHandle fft512VerticalApply = {};
    daxa::PipelineHandle fftCombine = {};
};