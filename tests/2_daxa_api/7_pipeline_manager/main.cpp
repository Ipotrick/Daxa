#include <daxa/daxa.hpp>
using namespace daxa::types;

#include <daxa/utils/pipeline_manager.hpp>

#include <iostream>

#define APPNAME "Daxa API Sample Pipeline Compiler"
#define APPNAME_PREFIX(x) ("[" APPNAME "] " x)

namespace tests
{
    auto simplest(daxa::Device & device) -> i32
    {
        daxa::PipelineManager pipeline_manager = daxa::PipelineManager({
            .device = device,
            .shader_compile_options = {
                .root_paths = {
                    DAXA_SHADER_INCLUDE_DIR,
                    DAXA_SAMPLE_PATH "/shaders",
                    "tests/0_common/shaders",
                },
                .language = daxa::ShaderLanguage::GLSL,
            },
            .name = APPNAME_PREFIX("pipeline_manager"),
        });

        auto compilation_result = pipeline_manager.add_compute_pipeline({
            .shader_info = {.source = daxa::ShaderFile{"main.glsl"}},
            .name = APPNAME_PREFIX("compute_pipeline"),
        });

        if (compilation_result.is_err())
        {
            std::cerr << "Failed to compile the compute_pipeline!\n";
            std::cerr << compilation_result.message() << std::endl;
            return -1;
        }

        std::shared_ptr<daxa::ComputePipeline> const compute_pipeline = compilation_result.value();

        return 0;
    }

    auto virtual_includes(daxa::Device & device) -> i32
    {
        daxa::PipelineManager pipeline_manager = daxa::PipelineManager({
            .device = device,
            .shader_compile_options = {
                .language = daxa::ShaderLanguage::GLSL,
            },
            .name = APPNAME_PREFIX("pipeline_manager"),
        });

        pipeline_manager.add_virtual_include_file({
            .name = "my_include",
            .contents = R"glsl(
                #pragma once
                #define MY_INCLUDE_DEFINE
            )glsl",
        });

        auto compilation_result = pipeline_manager.add_compute_pipeline({
            .shader_info = {.source = daxa::ShaderCode{R"glsl(
                #include <my_include>

                #ifndef MY_INCLUDE_DEFINE
                #error This should NOT happen
                #endif

                layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
                void main() {
                }
            )glsl"}},
            .name = APPNAME_PREFIX("compute_pipeline"),
        });

        if (compilation_result.is_err())
        {
            std::cerr << "Failed to compile the compute_pipeline!\n";
            std::cerr << compilation_result.message() << std::endl;
            return -1;
        }

        std::shared_ptr<daxa::ComputePipeline> const compute_pipeline = compilation_result.value();

        return 0;
    }

    auto tesselation_shaders(daxa::Device & device) -> i32
    {
        daxa::PipelineManager pipeline_manager = daxa::PipelineManager({
            .device = device,
            .shader_compile_options = {
                .root_paths = {
                    DAXA_SHADER_INCLUDE_DIR,
                    DAXA_SAMPLE_PATH "/shaders/test",
                },
                .language = daxa::ShaderLanguage::GLSL,
            },
            .debug_name = APPNAME_PREFIX("pipeline_manager"),
        });

        auto compilation_result = pipeline_manager.add_raster_pipeline({
            .vertex_shader_info = daxa::ShaderCompileInfo{.source = daxa::ShaderFile{"tesselation_test.glsl"}},
            .tesselation_control_shader_info = daxa::ShaderCompileInfo{.source = daxa::ShaderFile{"tesselation_test.glsl"}},
            .tesselation_evaluation_shader_info = daxa::ShaderCompileInfo{.source = daxa::ShaderFile{"tesselation_test.glsl"}},
            .fragment_shader_info = daxa::ShaderCompileInfo{.source = daxa::ShaderFile{"tesselation_test.glsl"}},
            .raster = {.primitive_topology = daxa::PrimitiveTopology::PATCH_LIST},
            .tesselation = {.control_points = 3},
        });

        if (compilation_result.is_err())
        {
            std::cerr << "Failed to compile the tesselation_test_pipeline!\n";
            std::cerr << compilation_result.message() << std::endl;
            return -1;
        }

        std::shared_ptr<daxa::RasterPipeline> const tesselation_test_pipeline = compilation_result.value();

        return 0;
    }
} // namespace tests

auto main() -> int
{
    daxa::Context daxa_ctx = daxa::create_context({
        .enable_validation = false,
    });
    daxa::Device device = daxa_ctx.create_device({
        .name = APPNAME_PREFIX("device"),
    });

    i32 ret = 0;

    if (ret = tests::simplest(device); ret != 0)
    {
        return ret;
    }
    if (ret = tests::virtual_includes(device); ret != 0)
    {
        return ret;
    }
    if (ret = tests::tesselation_shaders(device); ret != 0)
    {
        return ret;
    }

    std::cout << "Success!" << std::endl;
    return ret;
}
