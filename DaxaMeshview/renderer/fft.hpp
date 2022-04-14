#pragma once

#include "RenderContext.hpp"

class FFT{
public:
    constexpr static inline VkFormat IMAGE_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT;
    constexpr static inline VkFormat IMAGE_FORMAT_FREQ = VK_FORMAT_R32G32_SFLOAT;

    struct FFTGlobals {
        daxa::DescriptorIndex hdrImage;
        daxa::DescriptorIndex fftImage;
        daxa::DescriptorIndex horFreqImageR;
        daxa::DescriptorIndex horFreqImageG;
        daxa::DescriptorIndex horFreqImageB;
        u32 width = 1024;
        u32 height = 1024;
        u32 padWidth = 128;
        u32 padHeight = 128;
    };

    struct ExtractPush{
        daxa::DescriptorIndex globalsID;
    };

    struct CombinePush {

    };

    struct HorPush{
        daxa::DescriptorIndex globalsID;
    };

    struct VertApplyPush{

    };

    void uploadData(RenderContext& renderCTX, daxa::CommandListHandle& cmd) {
        globals.fftImage = fftImage->getDescriptorIndex();
        globals.hdrImage = renderCTX.hdrImage->getDescriptorIndex();
        globals.horFreqImageR = horFreqImageR->getDescriptorIndex();
        globals.horFreqImageG = horFreqImageG->getDescriptorIndex();
        globals.horFreqImageB = horFreqImageB->getDescriptorIndex();
        cmd.singleCopyHostToBuffer({
            .src = reinterpret_cast<u8*>(&globals),
            .dst = globalsBuffer,
            .region = { .size = sizeof(decltype(globals)) }
        });
        cmd.queueMemoryBarrier({
            .srcStages = daxa::STAGE_TRANSFER,
            .dstStages = daxa::STAGE_COMPUTE_SHADER,
        });
    }

    void init(RenderContext& renderCTX, daxa::CommandListHandle& cmd, u32 width, u32 height) {
        width = 1024;
        height = 1024;
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
        horFreqImageR = renderCTX.device->createImageView({
            .image = renderCTX.device->createImage({
                .format = IMAGE_FORMAT_FREQ,
                .extent = { width, height, 1 },
                .usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                .imageType = VK_IMAGE_TYPE_2D,
                .debugName = "horFreqImageR",
            }),
            .format = IMAGE_FORMAT_FREQ,
            .defaultSampler = renderCTX.defaultSampler,
            .debugName = "horFreqImageR",
        });
        horFreqImageG = renderCTX.device->createImageView({
            .image = renderCTX.device->createImage({
                .format = IMAGE_FORMAT_FREQ,
                .extent = { width, height, 1 },
                .usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                .imageType = VK_IMAGE_TYPE_2D,
                .debugName = "horFreqImageG",
            }),
            .format = IMAGE_FORMAT_FREQ,
            .defaultSampler = renderCTX.defaultSampler,
            .debugName = "horFreqImageG",
        });
        horFreqImageB = renderCTX.device->createImageView({
            .image = renderCTX.device->createImage({
                .format = IMAGE_FORMAT_FREQ,
                .extent = { width, height, 1 },
                .usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                .imageType = VK_IMAGE_TYPE_2D,
                .debugName = "horFreqImageB",
            }),
            .format = IMAGE_FORMAT_FREQ,
            .defaultSampler = renderCTX.defaultSampler,
            .debugName = "horFreqImageB",
        });

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
            .overwriteSets = { daxa::BIND_ALL_SET_DESCRIPTION },
            .pushConstantSize = sizeof(ExtractPush),
            .debugName = "fftExtract",
        }).value();
        fft1024Horizontal = renderCTX.pipelineCompiler->createComputePipeline({
            .shaderCI = {
                .entryPoint = "Main",
                .pathToSource = "fft/fft1024Horizontal.hlsl",
                .shaderLang = daxa::ShaderLang::HLSL,
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .debugName = "fft1024Horizontal",
            },
            .overwriteSets = { daxa::BIND_ALL_SET_DESCRIPTION },
            .pushConstantSize = sizeof(HorPush),
            .debugName = "fft1024Horizontal",
        }).value();
        fft1024VerticalApply = renderCTX.pipelineCompiler->createComputePipeline({
            .shaderCI = {
                .entryPoint = "Main",
                .pathToSource = "fft/fft1024VerticalApply.hlsl",
                .shaderLang = daxa::ShaderLang::HLSL,
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .debugName = "fft1024VerticalApply",
            },
            .overwriteSets = { daxa::BIND_ALL_SET_DESCRIPTION },
            .pushConstantSize = sizeof(VertApplyPush),
            .debugName = "fft1024VerticalApply",
        }).value();

        globalsBuffer = renderCTX.device->createBuffer({
            .memoryType = daxa::MemoryType::GPU_ONLY,
            .size = sizeof(FFTGlobals),
            .debugName = "globalsBuffer",
        });
    }

    void recreateImages(RenderContext& renderCTX, daxa::CommandListHandle& cmd, u32 width, u32 height) {

    }

    void extractPass(RenderContext& renderCTX, daxa::CommandListHandle& cmd) {
        renderCTX.pipelineCompiler->recreateIfChanged(fftExtract);

        cmd.bindPipeline(fftExtract);
        cmd.bindAll();
        cmd.pushConstant(VK_SHADER_STAGE_COMPUTE_BIT, ExtractPush{ .globalsID = globalsBuffer.getDescriptorIndex() });
        cmd.dispatch((globals.width + 7) / 8, (globals.height + 7) / 8, 1);
        cmd.unbindPipeline();
    }

    void horizontalPass(RenderContext& renderCTX, daxa::CommandListHandle& cmd) {
        renderCTX.pipelineCompiler->recreateIfChanged(fft1024Horizontal);

        cmd.bindPipeline(fft1024Horizontal);
        cmd.bindAll();
        cmd.pushConstant(VK_SHADER_STAGE_COMPUTE_BIT, HorPush{ .globalsID = globalsBuffer.getDescriptorIndex() });
        cmd.dispatch(1, 1024, 1);
        cmd.unbindPipeline();
    }

    void verticalApplyPass(RenderContext& renderCTX, daxa::CommandListHandle& cmd) {

    }

    void combinePass(RenderContext& renderCTX, daxa::CommandListHandle& cmd) {

    }

    void update(RenderContext& renderCTX, daxa::CommandListHandle& cmd) {
        cmd.queueImageBarrier({
            .image = fftImage,
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
        });
        cmd.queueImageBarrier({
            .image = horFreqImageR,
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
        });
        cmd.queueImageBarrier({
            .image = horFreqImageG,
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
        });
        cmd.queueImageBarrier({
            .image = horFreqImageB,
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
        });
        cmd.queueImageBarrier({
            .image = renderCTX.hdrImage,
            .layoutBefore = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
        cmd.insertQueuedBarriers();

        extractPass(renderCTX, cmd);
        cmd.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);
        horizontalPass(renderCTX, cmd);
        
        cmd.queueImageBarrier({
            .image = renderCTX.hdrImage,
            .layoutBefore = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        });
        cmd.queueImageBarrier({
            .image = horFreqImageB,
            .layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
            .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
        cmd.queueImageBarrier({
            .image = horFreqImageG,
            .layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
            .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
        cmd.queueImageBarrier({
            .image = horFreqImageR,
            .layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
            .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
        cmd.queueImageBarrier({
            .image = fftImage,
            .layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
            .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
    }
    
    FFTGlobals globals = {};
    daxa::BufferHandle globalsBuffer = {};

    daxa::ImageViewHandle fftImage = {};
    daxa::ImageViewHandle horFreqImageR = {};
    daxa::ImageViewHandle horFreqImageG = {};
    daxa::ImageViewHandle horFreqImageB = {};

    daxa::PipelineHandle fftExtract = {};
    daxa::PipelineHandle fft1024Horizontal = {};
    daxa::PipelineHandle fft1024VerticalApply = {};
    daxa::PipelineHandle fftCombine = {};
};