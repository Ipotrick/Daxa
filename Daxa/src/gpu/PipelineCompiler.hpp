#include "Pipeline.hpp"

namespace daxa {
    struct PipelineCompilerShadedData {
        Result<std::filesystem::path> findFullPathOfFile(std::filesystem::path const& file);
        std::vector<std::filesystem::path> rootPaths = { "." };
        std::vector<std::string> seenFiles = {};
    };

    class PipelineCompiler {
    public:
        PipelineCompiler(std::shared_ptr<gpu::DeviceBackend> deviceBackend, std::shared_ptr<gpu::BindingSetLayoutCache> bindSetLayoutCache);
        void addShaderSourceRootPath(std::filesystem::path const& path);
        Result<gpu::PipelineHandle> createGraphicsPipeline(gpu::GraphicsPipelineBuilder const& builder);
        Result<gpu::PipelineHandle> createComputePipeline(gpu::ComputePipelineCreateInfo const& ci);
        Result<bool> recreatePipelineOnSourceChange(gpu::PipelineHandle const& pipeline);
    private:
		daxa::Result<gpu::PipelineHandle> build(gpu::GraphicsPipelineBuilder const& builder);

        std::shared_ptr<gpu::DeviceBackend> deviceBackend = {};
        std::shared_ptr<gpu::BindingSetLayoutCache> bindSetLayoutCache = {};
        std::shared_ptr<PipelineCompilerShadedData> sharedData = {};
        shaderc::Compiler compiler = {};
        shaderc::CompileOptions options = {};
    };

	class PipelineCompilerHandle : public daxa::gpu::SharedHandle<PipelineCompiler>{};
}