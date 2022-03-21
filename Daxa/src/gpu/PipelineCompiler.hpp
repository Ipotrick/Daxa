#include "Pipeline.hpp"

namespace daxa {
    struct PipelineCompilerShadedData {
    public:
    private:
    };

    class PipelineCompiler {
    public:
        PipelineCompiler(std::shared_ptr<gpu::DeviceBackend> deviceBackend, std::shared_ptr<gpu::BindingSetLayoutCache> bindSetLayoutCache);
        void addShaderSourceRootPath(std::filesystem::path const& path);
        Result<gpu::PipelineHandle> createGraphicsPipeline(gpu::GraphicsPipelineBuilder const& builder);
        Result<gpu::PipelineHandle> createComputePipeline();
        Result<bool> recreatePipelineOnSourceChange(gpu::PipelineHandle const& pipeline);
    private:
        Result<std::filesystem::path> findFullPathOfFile(std::filesystem::path const& file);
		daxa::Result<gpu::PipelineHandle> build(gpu::GraphicsPipelineBuilder const& builder);

        std::shared_ptr<gpu::DeviceBackend> deviceBackend = {};
        std::shared_ptr<gpu::BindingSetLayoutCache> bindSetLayoutCache = {};
        std::vector<std::filesystem::path> rootPaths = { "." };
        std::shared_ptr<PipelineCompilerShadedData> sharedData = {};
        shaderc::Compiler compiler = {};
        shaderc::CompileOptions options = {};
    };

	class PipelineCompilerHandle : public daxa::gpu::SharedHandle<PipelineCompiler>{};
}