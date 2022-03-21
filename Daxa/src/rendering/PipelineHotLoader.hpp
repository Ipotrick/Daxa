#pragma once

#include "../DaxaCore.hpp"

#include "../gpu/Device.hpp"

namespace daxa {
    class GraphicsPipelineHotLoader {
    public:
        GraphicsPipelineHotLoader() = default;

        template<size_t SIZE>
        GraphicsPipelineHotLoader(
            gpu::DeviceHandle device, 
            PipelineCompilerHandle pipelineCompiler,
            gpu::GraphicsPipelineBuilder builder, 
            std::array<daxa::gpu::ShaderModuleCreateInfo, SIZE> const& cis
        )
            : device{ std::move(device) }
            , pipelineCompiler{ std::move(pipelineCompiler) }
            , builder{ std::move(builder) }
        {
            shaderCIs.reserve(SIZE);
            shaderWriteTimes.reserve(SIZE);
            for (auto& ci : cis) {
                shaderCIs.push_back(ci);
                shaderWriteTimes.push_back(std::filesystem::last_write_time(ci.pathToSource));
            }
        }

        std::optional<gpu::PipelineHandle> getNewIfChanged() {
            size_t changedShaders = 0;
            for (size_t i = 0; i < shaderCIs.size(); i++) {
                auto& shaderCI = shaderCIs[i];
                auto& lastWriteTime = shaderWriteTimes[i];
                
                auto latestWriteTime = std::filesystem::last_write_time(shaderCI.pathToSource);
                if (lastWriteTime < latestWriteTime) {
                    lastWriteTime = latestWriteTime;
                    builder.addShaderStage(shaderCI); 
                    changedShaders += 1;
                }
            }
            if (changedShaders > 0) {
                auto ret = pipelineCompiler->createGraphicsPipeline(builder);
                if (ret.isErr()) {
                    std::cout << ret.message() << std::endl;
                } else {
                    return ret.value();
                }
            }
            return {};
        }
    private:
        gpu::DeviceHandle device;
        PipelineCompilerHandle pipelineCompiler;
        gpu::GraphicsPipelineBuilder builder;
        std::vector<gpu::ShaderModuleCreateInfo> shaderCIs;
        std::vector<std::filesystem::file_time_type> shaderWriteTimes;
    };
    
    class ComputePipelineHotLoader {
    public:
        ComputePipelineHotLoader() = default;

        ComputePipelineHotLoader(
            gpu::DeviceHandle device, 
            gpu::ComputePipelineCreateInfo const& pipelineCI,
            daxa::gpu::ShaderModuleCreateInfo const& shaderCI
        )
            : device{ std::move(device) }
            , pipelineCI{ pipelineCI }
            , shaderCI{ shaderCI }
        { }

        std::optional<gpu::PipelineHandle> getNewIfChanged() {
            auto latestWriteTime = std::filesystem::last_write_time(shaderCI.pathToSource);
            if (lastWriteTime < latestWriteTime) {
                lastWriteTime = latestWriteTime;
                auto fragmenstOpaqueShader = device->createShaderModule(shaderCI);
                if (fragmenstOpaqueShader.isErr()) {
                    std::cout << fragmenstOpaqueShader.message() << std::endl;
                }else {
                    pipelineCI.shaderModule = fragmenstOpaqueShader.value(); 
                    auto ret = device->createComputePipeline(pipelineCI);
                    if (ret.isErr()) {
                        std::cout << ret.message() << std::endl;
                    } else {
                        return ret.value();
                    }
                }
            }
            return {};
        }
    private:
        gpu::DeviceHandle device;
        gpu::ComputePipelineCreateInfo pipelineCI;
        gpu::ShaderModuleCreateInfo shaderCI;
        std::filesystem::file_time_type lastWriteTime;
    };
};