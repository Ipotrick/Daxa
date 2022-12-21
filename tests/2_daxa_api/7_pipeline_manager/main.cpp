#include <daxa/daxa.hpp>
using namespace daxa::types;

#include <daxa/utils/pipeline_manager.hpp>

#include <iostream>

#define APPNAME "Daxa API Sample Pipeline Compiler"
#define APPNAME_PREFIX(x) ("[" APPNAME "] " x)

auto main() -> int
{
    daxa::Context daxa_ctx = daxa::create_context({
        .enable_validation = false,
    });
    daxa::Device const device = daxa_ctx.create_device({
        .debug_name = APPNAME_PREFIX("device"),
    });
    daxa::PipelineManager pipeline_manager = daxa::PipelineManager({
        .device = device,
        .shader_compile_options = {
            .root_paths = {
                DAXA_SAMPLE_PATH "/shaders",
                "tests/0_common/shaders",
                "include",
            },
            .language = daxa::ShaderLanguage::GLSL,
        },
        .debug_name = APPNAME_PREFIX("pipeline_manager"),
    });

    auto compilation_result = pipeline_manager.add_compute_pipeline({
        .shader_info = {.source = daxa::ShaderFile{"main.glsl"}},
        .debug_name = APPNAME_PREFIX("compute_pipeline"),
    });

    if (compilation_result.is_err())
    {
        std::cerr << "Failed to compile the compute_pipeline!\n";
        std::cerr << compilation_result.message() << std::endl;
        return -1;
    }

    std::cout << "Success!" << std::endl;

    std::shared_ptr<daxa::ComputePipeline> const compute_pipeline = compilation_result.value();
}
