#include "Pipeline.hpp"

#include <set>
#include <chrono>

namespace daxa {
    struct PipelineCompilerShadedData {
        Result<std::filesystem::path> findFullPathOfFile(std::filesystem::path const& file);
        Result<std::string> tryLoadShaderSourceFromFile(std::filesystem::path const& path);

        std::vector<std::filesystem::path> rootPaths = { "./" };
        // stores all seen files in the shader that is currently compiled.
        std::vector<std::filesystem::path> currentShaderSeenFiles = {};
        // stored all include file names of all shaders and the shader source code path itself.
        std::set<std::pair<std::filesystem::path, std::chrono::file_clock::time_point>>* observedHotLoadFiles = {};
    };

    class PipelineCompiler {
    public:
        PipelineCompiler(std::shared_ptr<DeviceBackend> deviceBackend, std::shared_ptr<BindingSetLayoutCache> bindSetLayoutCache);
        void addShaderSourceRootPath(std::filesystem::path const& path);
        bool checkIfSourcesChanged(PipelineHandle& pipeline);
        Result<PipelineHandle> createGraphicsPipeline(GraphicsPipelineBuilder const& builder);
        Result<PipelineHandle> createComputePipeline(ComputePipelineCreateInfo const& ci);
        Result<PipelineHandle> recreatePipeline(PipelineHandle const& pipeline);
        void recreateIfChanged(PipelineHandle& pipeline);
    private:
        Result<ShaderModuleHandle> tryCreateShaderModule(ShaderModuleCreateInfo const& ci);
		Result<std::vector<u32>> tryGenSPIRVFromShaderc(
            std::string const& src, 
            VkShaderStageFlagBits shaderStage, 
            ShaderLang lang, 
            char const* sourceFileName,
		    std::vector<std::string> const& defines
        );
        Result<std::vector<u32>> tryGenSPIRVFromDxc(
            std::string const& src, 
            VkShaderStageFlagBits shaderStage, 
            char const* entryPoint, 
            char const* sourceFileName,
		    std::vector<std::string> const& defines
        );
        daxa::Result<PipelineHandle> build(GraphicsPipelineBuilder const& builder);

        std::shared_ptr<DeviceBackend> deviceBackend = {};
        std::shared_ptr<BindingSetLayoutCache> bindSetLayoutCache = {};
        std::shared_ptr<PipelineCompilerShadedData> sharedData = {};
        std::chrono::milliseconds recreationCooldown = {};
        std::unique_ptr<void, void(*)(void*)> backend;
    };

	class PipelineCompilerHandle : public daxa::SharedHandle<PipelineCompiler>{};
}