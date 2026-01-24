#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <fstream>
#include <format>
#include <iostream>

#include "impl_resource_viewer.slang"

auto main() -> int
{
    daxa::Instance daxa_ctx = daxa::create_instance({});
    daxa::Device device = daxa_ctx.create_device_2(daxa_ctx.choose_device({}, daxa::DeviceInfo2{}));

    auto pipeline_manager = daxa::PipelineManager(daxa::PipelineManagerInfo2{
        .device = device,
        .root_paths = { 
            DAXA_SHADER_INCLUDE_DIR,
        },
        .write_out_spirv = "./",
        .default_language = daxa::ShaderLanguage::SLANG,
        .default_enable_debug_info = false,
        .name = "offline_compile_tg_resource_viewer",
    });

    auto compilation_result = pipeline_manager.add_compute_pipeline2({
        .source = daxa::ShaderFile{"../src/utils/impl_resource_viewer.slang"},
        .entry_point = "main",
        .push_constant_size = sizeof(TaskGraphDebugUiPush),
        .name = "impl_resource_viewer.slang",
    });

    DAXA_DBG_ASSERT_TRUE_M(compilation_result.is_ok(), std::format("ERROR! Shader did not compile successfully - ERROR: \n {}", compilation_result.to_string()));

    auto spv_cache_file_name = "./impl_resource_viewer.slang.comp.main.spv";

    auto compute_file = std::ifstream{spv_cache_file_name, std::ios::binary};
    auto compute_size = std::filesystem::file_size(spv_cache_file_name);
    auto spirv_code_points = std::vector<uint32_t>(compute_size / sizeof(uint32_t));
    compute_file.read(reinterpret_cast<char *>(spirv_code_points.data()), static_cast<std::streamsize>(compute_size));
    compute_file.close();
    std::filesystem::remove(spv_cache_file_name);

    // ======================================================
    // ================== OUTPUT FILE =======================
    // ======================================================

    auto out_file = std::ofstream{"./src/utils/image_impl_resource_viewer_spv.hpp", std::ofstream::trunc};
    out_file << "#pragma once\n#include <array>\n\n";
    out_file << std::format("static constexpr auto image_impl_resource_viewer_spv = std::array<uint32_t, {}>{{\n    // clang-format off\n   ", compute_size / sizeof(uint32_t));

    size_t iter = 0;
    for (auto const & u : spirv_code_points)
    {
        out_file << std::format(" {:#010x},", u);
        if ((iter % 8) == 7)
        {
            out_file << "\n   ";
        }
        ++iter;
    }

    out_file << "\n    // clang-format on\n};\n";
}