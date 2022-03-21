#include "Pipeline.hpp"

namespace daxa {
    class PipelineCompiler {
    public:
        void addShaderSourceRootPath(std::filesystem::path const& path);
        Result<gpu::PipelineHandle> createGraphicsPipeline();
        Result<gpu::PipelineHandle> createComputePipeline();
        Result<bool> recreatePipelineOnSourceChange(gpu::PipelineHandle const& pipeline);
    private:
        Result<std::filesystem::path> findFullPathOfFile(std::filesystem::path const& file);

        std::vector<std::filesystem::path> rootPaths = { "." };
    };
}