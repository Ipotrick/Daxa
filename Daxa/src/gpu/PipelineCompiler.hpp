#include "Pipeline.hpp"

#include <set>

namespace daxa {
    struct PipelineCompilerShadedData {
        Result<std::filesystem::path> findFullPathOfFile(std::filesystem::path const& file);
        std::vector<std::filesystem::path> rootPaths = { "./" };
        // stores all seen files in the shader that is currently compiled.
        std::vector<std::filesystem::path> currentShaderSeenFiles = {};
        // stored all include file names of all shaders and the shader source code path itself.
        std::set<std::pair<std::filesystem::path, std::chrono::file_clock::time_point>>* observedHotLoadFiles = {};
    };

    class PipelineCompiler {
    public:
        PipelineCompiler(std::shared_ptr<gpu::DeviceBackend> deviceBackend, std::shared_ptr<gpu::BindingSetLayoutCache> bindSetLayoutCache);
        void addShaderSourceRootPath(std::filesystem::path const& path);
        bool checkIfSourcesChanged(gpu::PipelineHandle& pipeline);
        Result<gpu::PipelineHandle> createGraphicsPipeline(gpu::GraphicsPipelineBuilder const& builder);
        Result<gpu::PipelineHandle> createComputePipeline(gpu::ComputePipelineCreateInfo const& ci);
        Result<gpu::PipelineHandle> recreatePipeline(gpu::PipelineHandle const& pipeline);
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