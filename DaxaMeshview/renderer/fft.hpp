#pragma once

#include "RenderContext.hpp"

class FFT{
public:
    constexpr static inline VkFormat IMAGE_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT;
    constexpr static inline VkFormat IMAGE_FORMAT_FREQ = VK_FORMAT_R32G32_SFLOAT;

    constexpr static inline u32 WIDTH = 2048;
    constexpr static inline u32 HEIGHT = WIDTH;

    struct FFTGlobals {
        daxa::DescriptorIndex hdrImage;
        daxa::DescriptorIndex fftImage;
        daxa::DescriptorIndex horFreqImageRG;
        daxa::DescriptorIndex horFreqImageBA;
        daxa::DescriptorIndex fullFreqImageRG;
        daxa::DescriptorIndex fullFreqImageBA;
        daxa::DescriptorIndex kernelID;
        daxa::DescriptorIndex kernelImageID;
        u32 width = WIDTH;
        u32 height = HEIGHT;
        u32 padWidth = 512;
        u32 padHeight = 512;
    };

    struct ExtractPush{
        daxa::DescriptorIndex globalsID;
    };

    struct CombinePush {

    };

    struct HorPush{
        daxa::DescriptorIndex globalsID;
        u32 backwards;
    };

    struct VertApplyPush{
        daxa::DescriptorIndex globalsID;
    };

    struct GenKernelPush {
        daxa::DescriptorIndex apertureID;
        daxa::DescriptorIndex horFreqImageRG;
        daxa::DescriptorIndex horFreqImageBA;
        daxa::DescriptorIndex kernelImageID;
        daxa::DescriptorIndex kernelID;
    };

    void uploadData(RenderContext& renderCTX, daxa::CommandListHandle& cmd) {
        globals.fftImage = fftImage->getDescriptorIndex();
        globals.hdrImage = renderCTX.hdrImage->getDescriptorIndex();
        globals.horFreqImageRG = horFreqImageRG->getDescriptorIndex();
        globals.horFreqImageBA = horFreqImageBA->getDescriptorIndex();
        globals.fullFreqImageRG = fullFreqImageRG->getDescriptorIndex();
        globals.fullFreqImageBA = fullFreqImageBA->getDescriptorIndex();
        globals.kernelID = kernel->getDescriptorIndex();
        globals.kernelImageID = kernelImage->getDescriptorIndex();
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
        std::string N_DEFINE_STRING = "N=2048";
        width = WIDTH;
        height = HEIGHT;
        fftImage = renderCTX.device->createImageView({
            .image = renderCTX.device->createImage({
                .imageType = VK_IMAGE_TYPE_2D,
                .format = IMAGE_FORMAT,
                .extent = { width, height, 1 },
                .usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                .debugName = "fftImage",
            }),
            .format = IMAGE_FORMAT,
            .defaultSampler = renderCTX.defaultSampler,
            .debugName = "fftImage",
        });
        horFreqImageRG = renderCTX.device->createImageView({
            .image = renderCTX.device->createImage({
                .imageType = VK_IMAGE_TYPE_2D,
                .format = IMAGE_FORMAT_FREQ,
                .extent = { width, height, 1 },
                .usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                .debugName = "horFreqImageRG",
            }),
            .format = IMAGE_FORMAT_FREQ,
            .defaultSampler = renderCTX.defaultSampler,
            .debugName = "horFreqImageRG",
        });
        horFreqImageBA = renderCTX.device->createImageView({
            .image = renderCTX.device->createImage({
                .imageType = VK_IMAGE_TYPE_2D,
                .format = IMAGE_FORMAT_FREQ,
                .extent = { width, height, 1 },
                .usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                .debugName = "horFreqImageBA",
            }),
            .format = IMAGE_FORMAT_FREQ,
            .defaultSampler = renderCTX.defaultSampler,
            .debugName = "horFreqImageBA",
        });
        fullFreqImageRG = renderCTX.device->createImageView({
            .image = renderCTX.device->createImage({
                .imageType = VK_IMAGE_TYPE_2D,
                .format = IMAGE_FORMAT_FREQ,
                .extent = { width, height, 1 },
                .usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                .debugName = "fullFreqImageRG",
            }),
            .format = IMAGE_FORMAT_FREQ,
            .defaultSampler = renderCTX.defaultSampler,
            .debugName = "fullFreqImageRG",
        });
        fullFreqImageBA = renderCTX.device->createImageView({
            .image = renderCTX.device->createImage({
                .imageType = VK_IMAGE_TYPE_2D,
                .format = IMAGE_FORMAT_FREQ,
                .extent = { width, height, 1 },
                .usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                .debugName = "fullFreqImageBA",
            }),
            .format = IMAGE_FORMAT_FREQ,
            .defaultSampler = renderCTX.defaultSampler,
            .debugName = "fullFreqImageBA",
        });
        kernel = renderCTX.device->createImageView({
            .image = renderCTX.device->createImage({
                .imageType = VK_IMAGE_TYPE_2D,
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .extent = { width, height, 1 },
                .usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                .debugName = "kernel",
            }),
            .format = VK_FORMAT_R32G32B32A32_SFLOAT,
            .defaultSampler = renderCTX.defaultSampler,
            .debugName = "kernel",
        });
        kernelImage = renderCTX.device->createImageView({
            .image = renderCTX.device->createImage({
                .imageType = VK_IMAGE_TYPE_2D,
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .extent = { width, height, 1 },
                .usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                .debugName = "kernelImage",
            }),
            .format = VK_FORMAT_R32G32B32A32_SFLOAT,
            .defaultSampler = renderCTX.defaultSampler,
            .debugName = "kernelImage",
        });
        aperatureImage = renderCTX.device->createImageView({
            .image = renderCTX.device->createImage({
                .imageType = VK_IMAGE_TYPE_2D,
                .format = VK_FORMAT_R8G8B8A8_SRGB,
                .extent = { width, height, 1 },
                .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                .debugName = "aperatureImage",
            }),
            .format = VK_FORMAT_R8G8B8A8_SRGB,
            .defaultSampler = renderCTX.defaultSampler,
            .debugName = "aperatureImage",
        });
        {
            char const HARDCODED_FILE_PATH[] = "C:\\Users\\Patrick\\Desktop\\aperture2048.png";

            int stb_width = 0;
            int stb_height = 0;
            int dummy = 0;
            void* data = stbi_load(HARDCODED_FILE_PATH, &stb_width, &stb_height, &dummy, 4);
            if (data) {
                cmd.queueImageBarrier({.image = aperatureImage, .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED, .layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL});
                cmd.singleCopyHostToImage({
                    .src = static_cast<u8*>(data),
                    .dst = aperatureImage->getImageHandle(),
                    .dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .region = {
                        .subRessource = {
                            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                            .mipLevel = 0,
                            .baseArrayLayer = 0,
                            .layerCount = 1,
                        },
                        .imageExtent = { WIDTH, HEIGHT, 1 },
                    },
                });
                cmd.queueImageBarrier({.image = aperatureImage, .layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
            } else {
                std::cout << "failed to load image: " << HARDCODED_FILE_PATH << std::endl;
            }
        }

        cmd.queueImageBarrier({
            .image = fftImage,
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
        cmd.insertQueuedBarriers();

        fftExtract = renderCTX.pipelineCompiler->createComputePipeline({
            .shaderCI = {
                .pathToSource = "fft/fftextract.hlsl",
                .shaderLang = daxa::ShaderLang::HLSL,
                .entryPoint = "Main",
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .defines = { N_DEFINE_STRING },
                .debugName = "fftExtract",
            },
            .overwriteSets = { daxa::BIND_ALL_SET_DESCRIPTION },
            .pushConstantSize = sizeof(ExtractPush),
            .debugName = "fftExtract",
        }).value();
        fftHorizontal = renderCTX.pipelineCompiler->createComputePipeline({
            .shaderCI = {
                .pathToSource = "fft/fftHorizontal.hlsl",
                .shaderLang = daxa::ShaderLang::HLSL,
                .entryPoint = "Main",
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .defines = { N_DEFINE_STRING },
                .debugName = "fftHorizontal",
            },
            .overwriteSets = { daxa::BIND_ALL_SET_DESCRIPTION },
            .pushConstantSize = sizeof(HorPush),
            .debugName = "fftHorizontal",
        }).value();
        fftVerticalApply = renderCTX.pipelineCompiler->createComputePipeline({
            .shaderCI = {
                .pathToSource = "fft/fftVerticalApply.hlsl",
                .shaderLang = daxa::ShaderLang::HLSL,
                .entryPoint = "Main",
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .defines = { N_DEFINE_STRING },
                .debugName = "fftVerticalApply",
            },
            .overwriteSets = { daxa::BIND_ALL_SET_DESCRIPTION },
            .pushConstantSize = sizeof(VertApplyPush),
            .debugName = "fftVerticalApply",
        }).value();
        genKernelHorizontal0 = renderCTX.pipelineCompiler->createComputePipeline({
            .shaderCI = {
                .pathToSource = "fft/fftGenKernel.hlsl",
                .shaderLang = daxa::ShaderLang::HLSL,
                .entryPoint = "MainHorizontal0",
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .defines = { N_DEFINE_STRING },
                .debugName = "fftGenKernel",
            },
            .overwriteSets = { daxa::BIND_ALL_SET_DESCRIPTION },
            .pushConstantSize = sizeof(GenKernelPush),
            .debugName = "genKernelHorizontal0",
        }).value();
        genKernelVertical0 = renderCTX.pipelineCompiler->createComputePipeline({
            .shaderCI = {
                .pathToSource = "fft/fftGenKernel.hlsl",
                .shaderLang = daxa::ShaderLang::HLSL,
                .entryPoint = "MainVertical0",
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .defines = { N_DEFINE_STRING },
                .debugName = "fftGenKernel",
            },
            .overwriteSets = { daxa::BIND_ALL_SET_DESCRIPTION },
            .pushConstantSize = sizeof(GenKernelPush),
            .debugName = "genKernelVertical0",
        }).value();
        genKernelHorizontal1 = renderCTX.pipelineCompiler->createComputePipeline({
            .shaderCI = {
                .pathToSource = "fft/fftGenKernel.hlsl",
                .shaderLang = daxa::ShaderLang::HLSL,
                .entryPoint = "MainHorizontal1",
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .defines = { N_DEFINE_STRING },
                .debugName = "fftGenKernel",
            },
            .overwriteSets = { daxa::BIND_ALL_SET_DESCRIPTION },
            .pushConstantSize = sizeof(GenKernelPush),
            .debugName = "genKernelHorizontal1",
        }).value();
        genKernelVertical1 = renderCTX.pipelineCompiler->createComputePipeline({
            .shaderCI = {
                .pathToSource = "fft/fftGenKernel.hlsl",
                .shaderLang = daxa::ShaderLang::HLSL,
                .entryPoint = "MainVertical1",
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .defines = { N_DEFINE_STRING },
                .debugName = "fftGenKernel",
            },
            .overwriteSets = { daxa::BIND_ALL_SET_DESCRIPTION },
            .pushConstantSize = sizeof(GenKernelPush),
            .debugName = "genKernelVertical1",
        }).value();

        globalsBuffer = renderCTX.device->createBuffer({
            .size = sizeof(FFTGlobals),
            .memoryType = daxa::MemoryType::GPU_ONLY,
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

    void horizontalPass(RenderContext& renderCTX, daxa::CommandListHandle& cmd, u32 backwards = 0) {
        renderCTX.pipelineCompiler->recreateIfChanged(fftHorizontal);

        cmd.bindPipeline(fftHorizontal);
        cmd.bindAll();
        cmd.pushConstant(VK_SHADER_STAGE_COMPUTE_BIT, HorPush{ .globalsID = globalsBuffer.getDescriptorIndex(), .backwards = backwards });
        cmd.dispatch(1, HEIGHT, 1);
        cmd.unbindPipeline();
    }

    void verticalApplyPass(RenderContext& renderCTX, daxa::CommandListHandle& cmd) {
        renderCTX.pipelineCompiler->recreateIfChanged(fftVerticalApply);

        cmd.bindPipeline(fftVerticalApply);
        cmd.bindAll();
        cmd.pushConstant(VK_SHADER_STAGE_COMPUTE_BIT, VertApplyPush{ .globalsID = globalsBuffer.getDescriptorIndex() });
        cmd.dispatch(WIDTH, 1, 1);
        cmd.unbindPipeline();
    }

    void genKernelPass(RenderContext& renderCTX, daxa::CommandListHandle& cmd) {
        bool one_changed = renderCTX.pipelineCompiler->checkIfSourcesChanged(genKernelHorizontal0) ||
            renderCTX.pipelineCompiler->checkIfSourcesChanged(genKernelVertical0) || 
            renderCTX.pipelineCompiler->checkIfSourcesChanged(genKernelHorizontal1) || 
            renderCTX.pipelineCompiler->checkIfSourcesChanged(genKernelVertical1) ||
            regenKernel; 

        auto reload = [&](daxa::PipelineHandle& pipeline) {
            auto result = renderCTX.pipelineCompiler->recreatePipeline(pipeline);
            std::cout << result << std::endl;
            if (result.isOk()) {
                pipeline = result.value();
            }
        };

        if (one_changed) {
            regenKernel = false;
            reload(genKernelHorizontal0);
            reload(genKernelVertical0);
            reload(genKernelHorizontal1);
            reload(genKernelVertical1);

            GenKernelPush push{
                .apertureID = aperatureImage->getDescriptorIndex(),
                .horFreqImageRG = horFreqImageRG->getDescriptorIndex(),
                .horFreqImageBA = horFreqImageBA->getDescriptorIndex(),
                .kernelImageID = kernelImage->getDescriptorIndex(),
                .kernelID = kernel->getDescriptorIndex()
            };

            cmd.bindPipeline(genKernelHorizontal0);
            cmd.bindAll();
            cmd.pushConstant(VK_SHADER_STAGE_COMPUTE_BIT, push);
            cmd.dispatch(1, HEIGHT, 1);
            cmd.unbindPipeline();
            cmd.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);
            cmd.bindPipeline(genKernelVertical0);
            cmd.bindAll();
            cmd.pushConstant(VK_SHADER_STAGE_COMPUTE_BIT, push);
            cmd.dispatch(WIDTH, 1, 1);
            cmd.unbindPipeline();

            cmd.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);

            cmd.bindPipeline(genKernelHorizontal1);
            cmd.bindAll();
            cmd.pushConstant(VK_SHADER_STAGE_COMPUTE_BIT, push);
            cmd.dispatch(1, HEIGHT, 1);
            cmd.unbindPipeline();
            cmd.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);
            cmd.bindPipeline(genKernelVertical1);
            cmd.bindAll();
            cmd.pushConstant(VK_SHADER_STAGE_COMPUTE_BIT, push);
            cmd.dispatch(WIDTH, 1, 1);
            cmd.unbindPipeline();
        }
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
            .image = horFreqImageRG,
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
        });
        cmd.queueImageBarrier({
            .image = horFreqImageBA,
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
        });
        cmd.queueImageBarrier({
            .image = fullFreqImageRG,
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
        });
        cmd.queueImageBarrier({
            .image = fullFreqImageBA,
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
        });
        cmd.queueImageBarrier({
            .image = kernel,
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
        });
        cmd.queueImageBarrier({
            .image = kernelImage,
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
        });
        cmd.queueImageBarrier({
            .image = renderCTX.hdrImage,
            .layoutBefore = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
        genKernelPass(renderCTX, cmd);
        cmd.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);
        extractPass(renderCTX, cmd);
        cmd.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);
        horizontalPass(renderCTX, cmd, 0);
        cmd.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);
        verticalApplyPass(renderCTX, cmd);
        cmd.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);
        horizontalPass(renderCTX, cmd, 1);
        
        cmd.queueImageBarrier({
            .image = renderCTX.hdrImage,
            .layoutBefore = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        });
        cmd.queueImageBarrier({
            .image = kernelImage,
            .layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
            .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
        cmd.queueImageBarrier({
            .image = kernel,
            .layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
            .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
        cmd.queueImageBarrier({
            .image = fullFreqImageBA,
            .layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
            .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
        cmd.queueImageBarrier({
            .image = fullFreqImageRG,
            .layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
            .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
        cmd.queueImageBarrier({
            .image = horFreqImageBA,
            .layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
            .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
        cmd.queueImageBarrier({
            .image = horFreqImageRG,
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
    daxa::ImageViewHandle horFreqImageRG = {};
    daxa::ImageViewHandle horFreqImageBA = {};
    daxa::ImageViewHandle fullFreqImageRG = {};
    daxa::ImageViewHandle fullFreqImageBA = {};

    daxa::ImageViewHandle aperatureImage = {};
    daxa::ImageViewHandle kernel = {};
    daxa::ImageViewHandle kernelImage = {};
    bool regenKernel = true;
    daxa::PipelineHandle genKernelVertical0 = {};
    daxa::PipelineHandle genKernelHorizontal0 = {};
    daxa::PipelineHandle genKernelVertical1 = {};
    daxa::PipelineHandle genKernelHorizontal1 = {};

    daxa::PipelineHandle fftExtract = {};
    daxa::PipelineHandle fftHorizontal = {};
    daxa::PipelineHandle fftVerticalApply = {};
    daxa::PipelineHandle fftCombine = {};
};