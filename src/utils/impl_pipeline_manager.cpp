#if DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_GLSLANG || DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_SLANG
#include "daxa/utils/pipeline_manager.hpp"

#include "../impl_core.hpp"
#include "impl_pipeline_manager.hpp"

#include <tuple>

#include <thread>
#include <utility>
#include <iostream>
#include <string>
// for std::hash<std::string>
#include <unordered_map>

#include "glslang_wrapper/wrapper.hpp"

namespace
{
    auto stage_string(daxa::ImplPipelineManager::ShaderStage stage) -> std::string_view
    {
        switch (stage)
        {
        case daxa::ImplPipelineManager::ShaderStage::COMP: return "comp";
        case daxa::ImplPipelineManager::ShaderStage::VERT: return "vert";
        case daxa::ImplPipelineManager::ShaderStage::FRAG: return "frag";
        case daxa::ImplPipelineManager::ShaderStage::TESS_CONTROL: return "tess_ctrl";
        case daxa::ImplPipelineManager::ShaderStage::TESS_EVAL: return "tess_eval";
        case daxa::ImplPipelineManager::ShaderStage::TASK: return "task";
        case daxa::ImplPipelineManager::ShaderStage::MESH: return "mesh";
        case daxa::ImplPipelineManager::ShaderStage::RAY_GEN: return "rgen";
        case daxa::ImplPipelineManager::ShaderStage::RAY_INTERSECT: return "rint";
        case daxa::ImplPipelineManager::ShaderStage::RAY_ANY_HIT: return "rahit";
        case daxa::ImplPipelineManager::ShaderStage::RAY_CLOSEST_HIT: return "rchit";
        case daxa::ImplPipelineManager::ShaderStage::RAY_MISS: return "rmiss";
        case daxa::ImplPipelineManager::ShaderStage::RAY_CALLABLE: return "rcall";
        default: return "none";
        }
    }
} // namespace

namespace daxa
{
    void ShaderCompileInfo::inherit(ShaderCompileInfo const & other)
    {
        if (!this->entry_point.has_value())
        {
            this->entry_point = other.entry_point;
        }
        if (!this->write_out_preprocessed_code.has_value())
        {
            this->write_out_preprocessed_code = other.write_out_preprocessed_code;
        }
        if (!this->write_out_shader_binary.has_value())
        {
            this->write_out_shader_binary = other.write_out_shader_binary;
        }
        if (!this->spirv_cache_folder.has_value())
        {
            this->spirv_cache_folder = other.spirv_cache_folder;
        }
        if (!this->language.has_value())
        {
            this->language = other.language;
        }
        if (!this->enable_debug_info.has_value())
        {
            this->enable_debug_info = other.enable_debug_info;
        }
        if (!this->create_flags.has_value())
        {
            this->create_flags = other.create_flags;
        }
        if (!this->required_subgroup_size.has_value())
        {
            this->required_subgroup_size = other.required_subgroup_size;
        }

        this->root_paths.insert(this->root_paths.begin(), other.root_paths.begin(), other.root_paths.end());
        this->defines.insert(this->defines.end(), other.defines.begin(), other.defines.end());
    }

    PipelineManager::PipelineManager(PipelineManagerInfo info)
    {
        this->object = new ImplPipelineManager{std::move(info)};
    }

    auto PipelineManager::add_ray_tracing_pipeline(RayTracingPipelineCompileInfo const & info) -> Result<std::shared_ptr<RayTracingPipeline>>
    {
        auto & impl = *r_cast<ImplPipelineManager *>(this->object);
        return impl.add_ray_tracing_pipeline(info);
    }

    auto PipelineManager::add_compute_pipeline(ComputePipelineCompileInfo const & info) -> Result<std::shared_ptr<ComputePipeline>>
    {
        auto & impl = *r_cast<ImplPipelineManager *>(this->object);
        return impl.add_compute_pipeline(info);
    }

    auto PipelineManager::add_raster_pipeline(RasterPipelineCompileInfo const & info) -> Result<std::shared_ptr<RasterPipeline>>
    {
        auto & impl = *r_cast<ImplPipelineManager *>(this->object);
        return impl.add_raster_pipeline(info);
    }

    void PipelineManager::remove_compute_pipeline(std::shared_ptr<ComputePipeline> const & pipeline)
    {
        auto & impl = *r_cast<ImplPipelineManager *>(this->object);
        return impl.remove_compute_pipeline(pipeline);
    }

    void PipelineManager::remove_ray_tracing_pipeline(std::shared_ptr<RayTracingPipeline> const & pipeline)
    {
        auto & impl = *r_cast<ImplPipelineManager *>(this->object);
        return impl.remove_ray_tracing_pipeline(pipeline);
    }

    void PipelineManager::remove_raster_pipeline(std::shared_ptr<RasterPipeline> const & pipeline)
    {
        auto & impl = *r_cast<ImplPipelineManager *>(this->object);
        return impl.remove_raster_pipeline(pipeline);
    }

    auto PipelineManager::reload_all() -> PipelineReloadResult
    {
        auto & impl = *r_cast<ImplPipelineManager *>(this->object);
        return impl.reload_all();
    }

    auto PipelineManager::all_pipelines_valid() const -> bool
    {
        auto const & impl = *r_cast<ImplPipelineManager *>(this->object);
        return impl.all_pipelines_valid();
    }

    static std::mutex glslang_init_mtx;
    static i32 pipeline_manager_count = 0;

    ImplPipelineManager::ImplPipelineManager(PipelineManagerInfo && a_info)
        : info{std::move(a_info)}
    {
        if (!this->info.shader_compile_options.entry_point.has_value())
        {
            this->info.shader_compile_options.entry_point = std::optional<std::string>{"main"};
        }
        if (!this->info.shader_compile_options.language.has_value())
        {
            this->info.shader_compile_options.language = std::optional<ShaderLanguage>{ShaderLanguage::GLSL};
        }
        if (!this->info.shader_compile_options.enable_debug_info.has_value())
        {
            this->info.shader_compile_options.enable_debug_info = {false};
        }

        {
            auto lock = std::lock_guard{glslang_init_mtx};
            if (pipeline_manager_count == 0)
            {
#if DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_GLSLANG
                glslang_wrapper_init();
#endif
#if DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_SLANG
                auto ret = slang::createGlobalSession(slang_backend.global_session.writeRef());
#endif
            }
            ++pipeline_manager_count;
        }
    }

    ImplPipelineManager::~ImplPipelineManager()
    {
#if DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_GLSLANG
        {
            auto lock = std::lock_guard{glslang_init_mtx};
            --pipeline_manager_count;
            if (pipeline_manager_count == 0)
            {
                glslang_wrapper_deinit();
            }
        }
#endif
    }

    auto ImplPipelineManager::create_ray_tracing_pipeline(RayTracingPipelineCompileInfo const & a_info) -> Result<RayTracingPipelineState>
    {
        if (a_info.push_constant_size > MAX_PUSH_CONSTANT_BYTE_SIZE)
        {
            return Result<RayTracingPipelineState>(std::string("push constant size of ") + std::to_string(a_info.push_constant_size) + std::string(" exceeds the maximum size of ") + std::to_string(MAX_PUSH_CONSTANT_BYTE_SIZE));
        }
        if (a_info.push_constant_size % 4 != 0)
        {
            return Result<RayTracingPipelineState>(std::string("push constant size of ") + std::to_string(a_info.push_constant_size) + std::string(" is not a multiple of 4(bytes)"));
        }
        auto pipe_result = RayTracingPipelineState{
            .pipeline_ptr = std::make_shared<RayTracingPipeline>(),
            .info = a_info,
            .last_hotload_time = std::chrono::file_clock::now(),
            .observed_hotload_files = {},
        };
        this->current_observed_hotload_files = &pipe_result.observed_hotload_files;
        auto ray_tracing_pipeline_info = RayTracingPipelineInfo{
            .ray_gen_shaders = {},
            .intersection_shaders = {},
            .any_hit_shaders = {},
            .callable_shaders = {},
            .closest_hit_shaders = {},
            .miss_hit_shaders = {},
            .shader_groups = {a_info.shader_groups_infos.data(), a_info.shader_groups_infos.size()},
            .max_ray_recursion_depth = a_info.max_ray_recursion_depth,
            .push_constant_size = a_info.push_constant_size,
            .name = a_info.name,
        };
        auto ray_gen_spirv_result = std::vector<daxa::Result<std::vector<unsigned int>>>();
        auto intersection_spirv_result = std::vector<daxa::Result<std::vector<unsigned int>>>();
        auto any_hit_spirv_result = std::vector<daxa::Result<std::vector<unsigned int>>>();
        auto callable_spirv_result = std::vector<daxa::Result<std::vector<unsigned int>>>();
        auto closest_hit_spirv_result = std::vector<daxa::Result<std::vector<unsigned int>>>();
        auto miss_hit_spirv_result = std::vector<daxa::Result<std::vector<unsigned int>>>();
        auto ray_gen_shader_infos = std::vector<ShaderInfo>{};
        auto intersection_shader_infos = std::vector<ShaderInfo>{};
        auto any_hit_shader_infos = std::vector<ShaderInfo>{};
        auto callable_shader_infos = std::vector<ShaderInfo>{};
        auto closest_hit_shader_infos = std::vector<ShaderInfo>{};
        auto miss_hit_shader_infos = std::vector<ShaderInfo>{};
        using ElemT = std::tuple<std::vector<ShaderCompileInfo> *, std::vector<ShaderInfo> *, std::vector<daxa::Result<std::vector<unsigned int>>> *, ShaderStage>;
        auto const result_shader_compile_infos = std::array<ElemT, 6>{
            ElemT{&pipe_result.info.ray_gen_infos, &ray_gen_shader_infos, &ray_gen_spirv_result, ShaderStage::RAY_GEN},
            ElemT{&pipe_result.info.intersection_infos, &intersection_shader_infos, &intersection_spirv_result, ShaderStage::RAY_INTERSECT},
            ElemT{&pipe_result.info.any_hit_infos, &any_hit_shader_infos, &any_hit_spirv_result, ShaderStage::RAY_ANY_HIT},
            ElemT{&pipe_result.info.callable_infos, &callable_shader_infos, &callable_spirv_result, ShaderStage::RAY_CALLABLE},
            ElemT{&pipe_result.info.closest_hit_infos, &closest_hit_shader_infos, &closest_hit_spirv_result, ShaderStage::RAY_CLOSEST_HIT},
            ElemT{&pipe_result.info.miss_hit_infos, &miss_hit_shader_infos, &miss_hit_spirv_result, ShaderStage::RAY_MISS},
        };

        for (auto [pipe_result_shader_info, final_shader_info, spv_results, stage] : result_shader_compile_infos)
        {
            for (auto & shader_compile_info : *pipe_result_shader_info)
            {
                auto spv_result = get_spirv(shader_compile_info, pipe_result.info.name, stage);
                if (spv_result.is_err())
                {
                    if (this->info.register_null_pipelines_when_first_compile_fails)
                    {
                        auto result = Result<RayTracingPipelineState>(pipe_result);
                        result.m = spv_result.message();
                        return result;
                    }
                    else
                    {
                        return Result<RayTracingPipelineState>(spv_result.message());
                    }
                }
                spv_results->push_back(std::move(spv_result));
                final_shader_info->push_back(daxa::ShaderInfo{
                    .byte_code = spv_results->back().value().data(),
                    .byte_code_size = static_cast<u32>(spv_results->back().value().size()),
                    .create_flags = shader_compile_info.create_flags.value_or(ShaderCreateFlagBits::NONE),
                    .required_subgroup_size =
                        shader_compile_info.required_subgroup_size.has_value() ? Optional{shader_compile_info.required_subgroup_size.value()} : daxa::None,
                });
            }
        }

        ray_tracing_pipeline_info.ray_gen_shaders = {ray_gen_shader_infos.data(), ray_gen_shader_infos.size()};
        ray_tracing_pipeline_info.intersection_shaders = {intersection_shader_infos.data(), intersection_shader_infos.size()};
        ray_tracing_pipeline_info.any_hit_shaders = {any_hit_shader_infos.data(), any_hit_shader_infos.size()};
        ray_tracing_pipeline_info.callable_shaders = {callable_shader_infos.data(), callable_shader_infos.size()};
        ray_tracing_pipeline_info.closest_hit_shaders = {closest_hit_shader_infos.data(), closest_hit_shader_infos.size()};
        ray_tracing_pipeline_info.miss_hit_shaders = {miss_hit_shader_infos.data(), miss_hit_shader_infos.size()};

        (*pipe_result.pipeline_ptr) = this->info.device.create_ray_tracing_pipeline(ray_tracing_pipeline_info);
        return Result<RayTracingPipelineState>(std::move(pipe_result));
    }

    auto ImplPipelineManager::create_compute_pipeline(ComputePipelineCompileInfo const & a_info) -> Result<ComputePipelineState>
    {
        if (a_info.push_constant_size > MAX_PUSH_CONSTANT_BYTE_SIZE)
        {
            return Result<ComputePipelineState>(std::string("push constant size of ") + std::to_string(a_info.push_constant_size) + std::string(" exceeds the maximum size of ") + std::to_string(MAX_PUSH_CONSTANT_BYTE_SIZE));
        }
        if (a_info.push_constant_size % 4 != 0)
        {
            return Result<ComputePipelineState>(std::string("push constant size of ") + std::to_string(a_info.push_constant_size) + std::string(" is not a multiple of 4(bytes)"));
        }
        auto pipe_result = ComputePipelineState{
            .pipeline_ptr = std::make_shared<ComputePipeline>(),
            .info = a_info,
            .last_hotload_time = std::chrono::file_clock::now(),
            .observed_hotload_files = {},
        };
        this->current_observed_hotload_files = &pipe_result.observed_hotload_files;
        auto spirv_result = get_spirv(pipe_result.info.shader_info, pipe_result.info.name, ShaderStage::COMP);
        if (spirv_result.is_err())
        {
            if (this->info.register_null_pipelines_when_first_compile_fails)
            {
                auto result = Result<ComputePipelineState>(pipe_result);
                result.m = spirv_result.message();
                return result;
            }
            else
            {
                return Result<ComputePipelineState>(spirv_result.message());
            }
        }
        char const * entry_point = "main";
        (*pipe_result.pipeline_ptr) = this->info.device.create_compute_pipeline({
            .shader_info = {
                .byte_code = spirv_result.value().data(),
                .byte_code_size = static_cast<u32>(spirv_result.value().size()),
                .create_flags = a_info.shader_info.create_flags.value_or(ShaderCreateFlagBits::NONE),
                .required_subgroup_size =
                    a_info.shader_info.required_subgroup_size.has_value() ? Optional{a_info.shader_info.required_subgroup_size.value()} : daxa::None,
                .entry_point = entry_point,
            },
            .push_constant_size = a_info.push_constant_size,
            .name = a_info.name.c_str(),
        });
        return Result<ComputePipelineState>(std::move(pipe_result));
    }

    auto ImplPipelineManager::create_raster_pipeline(RasterPipelineCompileInfo const & a_info) -> Result<RasterPipelineState>
    {
        if (a_info.push_constant_size > MAX_PUSH_CONSTANT_BYTE_SIZE)
        {
            return Result<RasterPipelineState>(std::string("push constant size of ") + std::to_string(a_info.push_constant_size) + std::string(" exceeds the maximum size of ") + std::to_string(MAX_PUSH_CONSTANT_BYTE_SIZE));
        }
        if (a_info.push_constant_size % 4 != 0)
        {
            return Result<RasterPipelineState>(std::string("push constant size of ") + std::to_string(a_info.push_constant_size) + std::string(" is not a multiple of 4(bytes)"));
        }
        auto pipe_result = RasterPipelineState{
            .pipeline_ptr = std::make_shared<RasterPipeline>(),
            .info = a_info,
            .last_hotload_time = std::chrono::file_clock::now(),
            .observed_hotload_files = {},
        };
        this->current_observed_hotload_files = &pipe_result.observed_hotload_files;
        auto raster_pipeline_info = RasterPipelineInfo{
            .color_attachments = {a_info.color_attachments.data(), a_info.color_attachments.size()},
            .depth_test = a_info.depth_test,
            .tesselation = a_info.tesselation,
            .raster = a_info.raster,
            .push_constant_size = a_info.push_constant_size,
            .name = a_info.name,
        };
        auto vertex_spirv_result = daxa::Result<std::vector<unsigned int>>("useless string");
        auto fragment_spirv_result = daxa::Result<std::vector<unsigned int>>("useless string");
        auto tesselation_control_spirv_result = daxa::Result<std::vector<unsigned int>>("useless string");
        auto tesselation_evaluation_spirv_result = daxa::Result<std::vector<unsigned int>>("useless string");
        auto task_spirv_result = daxa::Result<std::vector<unsigned int>>("useless string");
        auto mesh_spirv_result = daxa::Result<std::vector<unsigned int>>("useless string");
        using ElemT = std::tuple<Optional<ShaderCompileInfo> *, Optional<ShaderInfo> *, daxa::Result<std::vector<unsigned int>> *, ShaderStage>;
        auto const result_shader_compile_infos = std::array<ElemT, 6>{
            ElemT{&pipe_result.info.vertex_shader_info, &raster_pipeline_info.vertex_shader_info, &vertex_spirv_result, ShaderStage::VERT},
            ElemT{&pipe_result.info.fragment_shader_info, &raster_pipeline_info.fragment_shader_info, &fragment_spirv_result, ShaderStage::FRAG},
            ElemT{&pipe_result.info.tesselation_control_shader_info, &raster_pipeline_info.tesselation_control_shader_info, &tesselation_control_spirv_result, ShaderStage::TESS_CONTROL},
            ElemT{&pipe_result.info.tesselation_evaluation_shader_info, &raster_pipeline_info.tesselation_evaluation_shader_info, &tesselation_evaluation_spirv_result, ShaderStage::TESS_EVAL},
            ElemT{&pipe_result.info.task_shader_info, &raster_pipeline_info.task_shader_info, &task_spirv_result, ShaderStage::TASK},
            ElemT{&pipe_result.info.mesh_shader_info, &raster_pipeline_info.mesh_shader_info, &mesh_spirv_result, ShaderStage::MESH},
        };
        for (auto [pipe_result_shader_info, final_shader_info, spv_result, stage] : result_shader_compile_infos)
        {
            if (pipe_result_shader_info->has_value())
            {
                *spv_result = get_spirv(pipe_result_shader_info->value(), pipe_result.info.name, stage);
                if (spv_result->is_err())
                {
                    if (this->info.register_null_pipelines_when_first_compile_fails)
                    {
                        auto result = Result<RasterPipelineState>(pipe_result);
                        result.m = spv_result->message();
                        return result;
                    }
                    else
                    {
                        return Result<RasterPipelineState>(spv_result->message());
                    }
                }
                *final_shader_info = daxa::ShaderInfo{
                    .byte_code = spv_result->value().data(),
                    .byte_code_size = static_cast<u32>(spv_result->value().size()),
                    .create_flags = pipe_result_shader_info->value().create_flags.value_or(ShaderCreateFlagBits::NONE),
                    .required_subgroup_size =
                        pipe_result_shader_info->value().required_subgroup_size.has_value() ? Optional{pipe_result_shader_info->value().required_subgroup_size.value()} : daxa::None,
                };
            }
        }
        (*pipe_result.pipeline_ptr) = this->info.device.create_raster_pipeline(raster_pipeline_info);
        return Result<RasterPipelineState>(std::move(pipe_result));
    }

    auto ImplPipelineManager::add_ray_tracing_pipeline(RayTracingPipelineCompileInfo const & a_info) -> Result<std::shared_ptr<RayTracingPipeline>>
    {
        // DAXA_DBG_ASSERT_TRUE_M(!daxa::holds_alternative<daxa::Monostate>(a_info.shader_info.source), "must provide shader source");
        auto modified_info = a_info;
        for (auto & shader_compile_info : modified_info.ray_gen_infos)
        {
            shader_compile_info.inherit(this->info.shader_compile_options);
        }
        for (auto & shader_compile_info : modified_info.intersection_infos)
        {
            shader_compile_info.inherit(this->info.shader_compile_options);
        }
        for (auto & shader_compile_info : modified_info.any_hit_infos)
        {
            shader_compile_info.inherit(this->info.shader_compile_options);
        }
        for (auto & shader_compile_info : modified_info.callable_infos)
        {
            shader_compile_info.inherit(this->info.shader_compile_options);
        }
        for (auto & shader_compile_info : modified_info.closest_hit_infos)
        {
            shader_compile_info.inherit(this->info.shader_compile_options);
        }
        for (auto & shader_compile_info : modified_info.miss_hit_infos)
        {
            shader_compile_info.inherit(this->info.shader_compile_options);
        }

        auto pipe_result = create_ray_tracing_pipeline(modified_info);
        if (pipe_result.is_err())
        {
            return Result<std::shared_ptr<RayTracingPipeline>>(pipe_result.m);
        }
        this->ray_tracing_pipelines.push_back(pipe_result.value());
        if (this->info.register_null_pipelines_when_first_compile_fails)
        {
            auto result = Result<std::shared_ptr<RayTracingPipeline>>(std::move(pipe_result.value().pipeline_ptr));
            result.m = std::move(pipe_result.m);
            return result;
        }
        else
        {
            return Result<std::shared_ptr<RayTracingPipeline>>(std::move(pipe_result.value().pipeline_ptr));
        }
    }

    auto ImplPipelineManager::add_compute_pipeline(ComputePipelineCompileInfo const & a_info) -> Result<std::shared_ptr<ComputePipeline>>
    {
        DAXA_DBG_ASSERT_TRUE_M(!a_info.shader_info.source_path.empty(), "must provide shader source");
        auto modified_info = a_info;
        modified_info.shader_info.inherit(this->info.shader_compile_options);
        auto pipe_result = create_compute_pipeline(modified_info);
        if (pipe_result.is_err())
        {
            return Result<std::shared_ptr<ComputePipeline>>(pipe_result.m);
        }
        this->compute_pipelines.push_back(pipe_result.value());
        if (this->info.register_null_pipelines_when_first_compile_fails)
        {
            auto result = Result<std::shared_ptr<ComputePipeline>>(std::move(pipe_result.value().pipeline_ptr));
            result.m = std::move(pipe_result.m);
            return result;
        }
        else
        {
            return Result<std::shared_ptr<ComputePipeline>>(std::move(pipe_result.value().pipeline_ptr));
        }
    }

    auto ImplPipelineManager::add_raster_pipeline(RasterPipelineCompileInfo const & a_info) -> Result<std::shared_ptr<RasterPipeline>>
    {
        auto modified_info = a_info;
        auto const modified_shader_compile_infos = std::array<Optional<ShaderCompileInfo> *, 6>{
            &modified_info.vertex_shader_info,
            &modified_info.tesselation_control_shader_info,
            &modified_info.tesselation_evaluation_shader_info,
            &modified_info.fragment_shader_info,
            &modified_info.mesh_shader_info,
            &modified_info.task_shader_info,
        };
        for (auto * shader_compile_info : modified_shader_compile_infos)
        {
            if (shader_compile_info->has_value())
            {
                shader_compile_info->value().inherit(this->info.shader_compile_options);
            }
        }
        auto pipe_result = create_raster_pipeline(modified_info);
        if (pipe_result.is_err())
        {
            return Result<std::shared_ptr<RasterPipeline>>(pipe_result.m);
        }
        this->raster_pipelines.push_back(pipe_result.value());
        if (this->info.register_null_pipelines_when_first_compile_fails)
        {
            auto result = Result<std::shared_ptr<RasterPipeline>>(std::move(pipe_result.value().pipeline_ptr));
            result.m = std::move(pipe_result.m);
            return result;
        }
        else
        {
            return Result<std::shared_ptr<RasterPipeline>>(std::move(pipe_result.value().pipeline_ptr));
        }
    }

    void ImplPipelineManager::remove_ray_tracing_pipeline(std::shared_ptr<RayTracingPipeline> const & pipeline)
    {
        auto pipeline_iter = std::find_if(
            this->ray_tracing_pipelines.begin(),
            this->ray_tracing_pipelines.end(),
            [&pipeline](RayTracingPipelineState const & other)
            {
                return pipeline.get() == other.pipeline_ptr.get();
            });
        if (pipeline_iter == this->ray_tracing_pipelines.end())
        {
            return;
        }
        this->ray_tracing_pipelines.erase(pipeline_iter);
    }

    void ImplPipelineManager::remove_compute_pipeline(std::shared_ptr<ComputePipeline> const & pipeline)
    {
        auto pipeline_iter = std::find_if(
            this->compute_pipelines.begin(),
            this->compute_pipelines.end(),
            [&pipeline](ComputePipelineState const & other)
            {
                return pipeline.get() == other.pipeline_ptr.get();
            });
        if (pipeline_iter == this->compute_pipelines.end())
        {
            return;
        }
        this->compute_pipelines.erase(pipeline_iter);
    }

    void ImplPipelineManager::remove_raster_pipeline(std::shared_ptr<RasterPipeline> const & pipeline)
    {
        auto pipeline_iter = std::find_if(
            this->raster_pipelines.begin(),
            this->raster_pipelines.end(),
            [&pipeline](RasterPipelineState const & other)
            {
                return pipeline.get() == other.pipeline_ptr.get();
            });
        if (pipeline_iter == this->raster_pipelines.end())
        {
            return;
        }
        this->raster_pipelines.erase(pipeline_iter);
    }

    using FileWriteTimeLookupTable = std::unordered_map<std::string, std::filesystem::file_time_type>;

    static auto check_if_sources_changed(std::chrono::file_clock::time_point & last_hotload_time, ShaderFileTimeSet & observed_hotload_files, FileWriteTimeLookupTable & lookup_table) -> bool
    {
        using namespace std::chrono_literals;
        static constexpr auto HOTRELOAD_MIN_TIME = 250ms;

        auto now = std::chrono::file_clock::now();
        using namespace std::chrono_literals;
        if (now - last_hotload_time < HOTRELOAD_MIN_TIME)
        {
            return false;
        }
        last_hotload_time = now;
        bool reload = false;

        auto get_last_file_write_time = [&](std::filesystem::path const & path)
        {
            auto full_path_str = std::filesystem::absolute(path).string();
            auto iter = lookup_table.find(full_path_str);
            if (iter != lookup_table.end())
            {
                return iter->second;
            }
            else
            {
                auto latest_write_time = std::filesystem::last_write_time(path);
                lookup_table[full_path_str] = latest_write_time;
                return latest_write_time;
            }
        };

        for (auto & [path, recorded_write_time] : observed_hotload_files)
        {
            auto path_str = path.string();
            // if (std::filesystem::exists(path))
            {
                auto latest_write_time = get_last_file_write_time(path);
                if (latest_write_time > recorded_write_time)
                {
                    reload = true;
                }
            }
        }
        if (reload)
        {
            for (auto & pair : observed_hotload_files)
            {
                auto path_str = pair.first.string();
                if (std::filesystem::exists(pair.first))
                {
                    pair.second = get_last_file_write_time(pair.first);
                }
            }
        }
        return reload;
    };

    auto ImplPipelineManager::reload_all() -> PipelineReloadResult
    {
        bool reloaded = false;

        // Optimization for caching the write times so that multiple pipelines don't check the
        // filesystem for the same file's write-time. Filesystem checks are really slow...
        auto lookup_table = FileWriteTimeLookupTable{};

        for (auto & [pipeline, compile_info, last_hotload_time, observed_hotload_files] : this->compute_pipelines)
        {
            if (check_if_sources_changed(last_hotload_time, observed_hotload_files, lookup_table))
            {
                reloaded = true;
                auto new_pipeline = create_compute_pipeline(compile_info);
                bool is_valid = true;
                if (this->info.register_null_pipelines_when_first_compile_fails)
                {
                    is_valid = new_pipeline.is_ok() && new_pipeline.value().pipeline_ptr->is_valid();
                }
                else
                {
                    is_valid = new_pipeline.is_ok();
                }
                if (is_valid)
                {
                    *pipeline = std::move(*new_pipeline.value().pipeline_ptr);
                }
                else
                {
                    return PipelineReloadError{new_pipeline.m};
                }
            }
        }

        for (auto & [pipeline, compile_info, last_hotload_time, observed_hotload_files] : this->raster_pipelines)
        {
            if (check_if_sources_changed(last_hotload_time, observed_hotload_files, lookup_table))
            {
                reloaded = true;
                auto new_pipeline = create_raster_pipeline(compile_info);
                bool is_valid = true;
                if (this->info.register_null_pipelines_when_first_compile_fails)
                {
                    is_valid = new_pipeline.is_ok() && new_pipeline.value().pipeline_ptr->is_valid();
                }
                else
                {
                    is_valid = new_pipeline.is_ok();
                }
                if (is_valid)
                {
                    *pipeline = std::move(*new_pipeline.value().pipeline_ptr);
                }
                else
                {
                    return PipelineReloadError{new_pipeline.m};
                }
            }
        }

        for (auto & [pipeline, compile_info, last_hotload_time, observed_hotload_files] : this->ray_tracing_pipelines)
        {
            if (check_if_sources_changed(last_hotload_time, observed_hotload_files, lookup_table))
            {
                reloaded = true;
                auto new_pipeline = create_ray_tracing_pipeline(compile_info);
                bool is_valid = true;
                if (this->info.register_null_pipelines_when_first_compile_fails)
                {
                    is_valid = new_pipeline.is_ok() && new_pipeline.value().pipeline_ptr->is_valid();
                }
                else
                {
                    is_valid = new_pipeline.is_ok();
                }
                if (is_valid)
                {
                    *pipeline = std::move(*new_pipeline.value().pipeline_ptr);
                }
                else
                {
                    return PipelineReloadError{new_pipeline.m};
                }
            }
        }

        if (reloaded)
        {
            return PipelineReloadSuccess{};
        }
        else
        {
            return NoPipelineChanged{};
        }
    }

    auto ImplPipelineManager::all_pipelines_valid() const -> bool
    {
        for (RasterPipelineState const & raster_pipeline_state : this->raster_pipelines)
        {
            if (!raster_pipeline_state.pipeline_ptr->is_valid())
            {
                return false;
            }
        }

        for (ComputePipelineState const & compute_pipeline_state : this->compute_pipelines)
        {
            if (!compute_pipeline_state.pipeline_ptr->is_valid())
            {
                return false;
            }
        }

        for (RayTracingPipelineState const & raytracing_pipeline_state : this->ray_tracing_pipelines)
        {
            if (!raytracing_pipeline_state.pipeline_ptr->is_valid())
            {
                return false;
            }
        }
        return true;
    }

    static auto hash_shader_info(std::string const & source_string, ShaderCompileInfo const & compile_options, ImplPipelineManager::ShaderStage shader_stage) -> uint64_t
    {
        auto result = uint64_t{};

        auto hash_combine = [](uint64_t h1, uint64_t h2) -> uint64_t
        {
            return h1 ^ (h2 << 1);
        };

        auto hash_shader_compile_options = [&hash_combine](ShaderCompileInfo const & options) -> uint64_t
        {
            auto result = uint64_t{};
            if (options.entry_point.has_value())
            {
                result = hash_combine(result, std::hash<std::string>{}(options.entry_point.value()));
            }
            for (auto const & path : options.root_paths)
            {
                result = hash_combine(result, std::hash<std::string>{}(path.string()));
            }
            if (options.language.has_value())
            {
                result = hash_combine(result, std::hash<uint32_t>{}(static_cast<uint32_t>(options.language.value())));
            }
            for (auto const & define : options.defines)
            {
                result = hash_combine(result, std::hash<std::string>{}(define.name));
                result = hash_combine(result, std::hash<std::string>{}(define.value));
            }
            if (options.enable_debug_info.has_value())
            {
                result = hash_combine(result, std::hash<uint32_t>{}(static_cast<uint32_t>(options.enable_debug_info.value())));
            }
            return result;
        };

        result = hash_combine(result, std::hash<std::string>{}(source_string));
        result = hash_combine(result, hash_shader_compile_options(compile_options));
        result = hash_combine(result, std::hash<uint32_t>{}(static_cast<uint32_t>(shader_stage)));

        return result;
    }

    static constexpr auto CACHE_FILE_MAGIC_NUMBER = std::bit_cast<uint64_t>(std::to_array("daxpipe"));
    static constexpr auto CACHE_FILE_VERSION = uint64_t{3};

    struct ShaderCacheFileHeader
    {
        uint64_t magic_number;
        uint64_t version;
        uint64_t dependency_n;
        uint64_t spirv_size;
    };

    void ImplPipelineManager::save_shader_cache(std::filesystem::path const & cache_folder, uint64_t shader_info_hash, std::vector<u32> const & spirv)
    {
        std::filesystem::create_directories(cache_folder);
        auto out_file = std::ofstream{cache_folder / std::filesystem::path{std::to_string(shader_info_hash)}, std::ios::binary};
        auto header = ShaderCacheFileHeader{};
        header.magic_number = CACHE_FILE_MAGIC_NUMBER;
        header.version = CACHE_FILE_VERSION;
        header.dependency_n = current_observed_hotload_files->size();
        header.spirv_size = spirv.size() * sizeof(u32);

        out_file.write(reinterpret_cast<char const *>(&header), sizeof(header));
        // TODO: Save more granular dependency info
        for (auto const & [path, time_point] : *current_observed_hotload_files)
        {
            auto flags = uint64_t{};
            auto path_string = path.string();
            auto path_string_size = uint64_t{path_string.size()};
            auto time_since_epoch = time_point.time_since_epoch().count();
            out_file.write(reinterpret_cast<char const *>(&flags), sizeof(flags));
            out_file.write(reinterpret_cast<char const *>(&path_string_size), sizeof(path_string_size));
            out_file.write(path_string.data(), path_string.size());
            out_file.write(reinterpret_cast<char const *>(&time_since_epoch), sizeof(time_since_epoch));
        }
        out_file.write(reinterpret_cast<char const *>(spirv.data()), header.spirv_size);
    }

    auto ImplPipelineManager::try_load_shader_cache(std::filesystem::path const & cache_folder, uint64_t shader_info_hash) -> Result<std::vector<u32>>
    {
        auto in_file = std::ifstream{cache_folder / std::filesystem::path{std::to_string(shader_info_hash)}, std::ios::binary};
        if (in_file.good())
        {
            auto header = ShaderCacheFileHeader{};
            in_file.read(reinterpret_cast<char *>(&header), sizeof(header));
            if (header.magic_number != CACHE_FILE_MAGIC_NUMBER)
            {
                return Result<std::vector<u32>>(std::string_view{"bad cache file"});
            }
            if (header.version != CACHE_FILE_VERSION)
            {
                return Result<std::vector<u32>>(std::string_view{"needs update"});
            }

            for (uint64_t dep_i = 0; dep_i < header.dependency_n; ++dep_i)
            {
                auto flags = uint64_t{};
                auto path = std::filesystem::path{};
                auto time_since_epoch = std::chrono::system_clock::rep{};
                auto path_string = std::string{};
                auto path_string_size = uint64_t{};
                in_file.read(reinterpret_cast<char *>(&flags), sizeof(flags));
                in_file.read(reinterpret_cast<char *>(&path_string_size), sizeof(path_string_size));
                path_string.resize(path_string_size);
                in_file.read(path_string.data(), path_string_size);
                path = path_string;
                in_file.read(reinterpret_cast<char *>(&time_since_epoch), sizeof(time_since_epoch));
                if (!std::filesystem::exists(path))
                {
                    return Result<std::vector<u32>>(std::string_view{"needs update"});
                }
                auto write_time = std::filesystem::last_write_time(path);
                if (write_time.time_since_epoch().count() > time_since_epoch)
                {
                    return Result<std::vector<u32>>(std::string_view{"needs update"});
                }
                // NOTE(grundlett): Setting the time to now is fine, as we successfully handle
                // any temporal changes above. This is a bus sus tho.
                current_observed_hotload_files->insert({path, std::chrono::file_clock::now()});
            }

            auto spirv = std::vector<u32>{};
            spirv.resize(header.spirv_size / sizeof(u32));
            in_file.read(reinterpret_cast<char *>(spirv.data()), header.spirv_size);
            return Result<std::vector<u32>>{spirv};
        }
        return Result<std::vector<u32>>(std::string_view{"no cache found"});
    }

    auto ImplPipelineManager::get_spirv(ShaderCompileInfo const & shader_info, std::string const & debug_name_opt, ShaderStage shader_stage) -> Result<std::vector<u32>>
    {
        // TODO: Not internally threadsafe
        current_shader_info = &shader_info;
        std::vector<u32> spirv = {};
        // if (daxa::holds_alternative<ShaderByteCode>(shader_info.source))
        // {
        //     auto byte_code = daxa::get<ShaderByteCode>(shader_info.source);
        //     spirv.assign(byte_code.begin(), byte_code.end());
        // }
        // else
        {
            auto file_ret = full_path_to_file(shader_info.source_path);
            if (file_ret.is_err())
            {
                return Result<std::vector<u32>>(file_ret.message());
            }
            // This is a hack. Instead of providing the file as source code, we provide the full path.
            // code = {.string = std::string("#include \"") + ret.value().string() + "\"\n"};
            auto code_ret = load_shader_source_from_file(file_ret.value());
            if (code_ret.is_err())
            {
                return Result<std::vector<u32>>(code_ret.message());
            }
            ShaderCode code = code_ret.value();

            // TODO: Test if this is slow, as it's not needed if there's no shader cache.
            auto shader_info_hash = hash_shader_info(code.string, shader_info, shader_stage);
            if (shader_info.spirv_cache_folder.has_value())
            {
                auto cache_ret = try_load_shader_cache(shader_info.spirv_cache_folder.value(), shader_info_hash);
                if (cache_ret.is_ok())
                {
                    return cache_ret;
                }
            }

            Result<std::vector<u32>> ret = Result<std::vector<u32>>("No shader was compiled");

            DAXA_DBG_ASSERT_TRUE_M(shader_info.language.has_value(), "How did this happen? You mustn't provide a nullopt for the language");

            DAXA_DBG_ASSERT_TRUE_M(shader_info.language.has_value(), "You must have a shader language set when compiling GLSL");
            switch (shader_info.language.value())
            {
#if DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_GLSLANG
            case ShaderLanguage::GLSL:
                ret = get_spirv_glslang(shader_info, debug_name_opt, shader_stage, code, shader_info_hash);
                break;
#endif
#if DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_SLANG
            case ShaderLanguage::SLANG:
                ret = get_spirv_slang(shader_info, shader_stage, code);
                break;
#endif
            default: break;
            }

            if (ret.is_err())
            {
                current_shader_info = nullptr;
                return Result<std::vector<u32>>(ret.message());
            }

            spirv = ret.value();
            if (shader_info.spirv_cache_folder.has_value())
            {
                save_shader_cache(shader_info.spirv_cache_folder.value(), shader_info_hash, spirv);
            }
        }
        current_shader_info = nullptr;

        std::string name = "unnamed-shader";
        if (!debug_name_opt.empty())
        {
            name = debug_name_opt;
        }
        else
        {
            name = shader_info.source_path.string();
        }

        if (shader_info.write_out_shader_binary.has_value())
        {
            std::replace(name.begin(), name.end(), '/', '_');
            std::replace(name.begin(), name.end(), '\\', '_');
            std::replace(name.begin(), name.end(), ':', '_');
            std::replace(name.begin(), name.end(), '.', '_');
            name = name + "_" + std::string{stage_string(shader_stage)} + "_" + shader_info.entry_point.value() + ".spirv";
            auto out_folder = shader_info.write_out_shader_binary.value();
            std::filesystem::create_directories(out_folder);
            std::ofstream ofs(out_folder / name, std::ios_base::trunc | std::ios_base::binary);
            ofs.write(r_cast<char const *>(spirv.data()), static_cast<std::streamsize>(spirv.size() * 4));
            ofs.close();

            // std::cout << std::hex;
            // u32 codepoint = 0;
            // for (auto const & i : spirv)
            // {
            //     std::cout << "0x" << std::setw(8) << std::setfill('0') << i << ", ";
            //     ++codepoint;
            //     if ((codepoint & 0x7) == 0)
            //     {
            //         std::cout << "\n";
            //     }
            // }
            // std::cout << std::endl
            //           << std::endl;
        }

#if DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_SPIRV_VALIDATION
        spirv_tools.SetMessageConsumer(
            [&](spv_message_level_t level, [[maybe_unused]] char const * source, [[maybe_unused]] spv_position_t const & position, char const * message)
            { DAXA_DBG_ASSERT_TRUE_M(level > SPV_MSG_WARNING, std::format("SPIR-V Validation error after compiling {}:\n - {}", debug_name_opt, message)); });
        spvtools::ValidatorOptions options{};
        options.SetScalarBlockLayout(true);
        // spirv_tools.Validate(spirv.data(), spirv.size(), options);
#endif

        return Result<std::vector<u32>>(spirv);
    }

    auto ImplPipelineManager::full_path_to_file(std::filesystem::path const & path) -> Result<std::filesystem::path>
    {
        if (std::filesystem::exists(path))
        {
            return Result<std::filesystem::path>(std::filesystem::canonical(path));
        }
        std::filesystem::path potential_path;
        if (this->current_shader_info != nullptr)
        {
            for (auto const & root : this->current_shader_info->root_paths)
            {
                potential_path.clear();
                potential_path = root / path;
                if (std::filesystem::exists(potential_path))
                {
                    return Result<std::filesystem::path>(std::filesystem::canonical(potential_path));
                }
            }
        }
        else
        {
            for (auto & root : this->info.shader_compile_options.root_paths)
            {
                potential_path.clear();
                potential_path = root / path;
                if (std::filesystem::exists(potential_path))
                {
                    return Result<std::filesystem::path>(std::filesystem::canonical(potential_path));
                }
            }
        }
        std::string error_msg = {};
        error_msg += "could not find file :\"";
        error_msg += path.string();
        error_msg += "\"";
        return Result<std::filesystem::path>(std::string_view(error_msg));
    }

    auto ImplPipelineManager::load_shader_source_from_file(std::filesystem::path const & path) -> Result<ShaderCode>
    {
        auto result_path = full_path_to_file(path);
        if (result_path.is_err())
        {
            return Result<ShaderCode>(result_path.message());
        }
        auto start_time = std::chrono::steady_clock::now();
        using namespace std::chrono_literals;
        while (std::chrono::duration<f32>(std::chrono::steady_clock::now() - start_time) < 0.1s)
        {
            std::ifstream ifs{path};
            DAXA_DBG_ASSERT_TRUE_M(ifs.good(), "Could not open shader file");
            current_observed_hotload_files->insert({
                result_path.value(),
                std::filesystem::last_write_time(result_path.value()),
            });
            std::string str = {};
            ifs.seekg(0, std::ios::end);
            str.reserve(static_cast<usize>(ifs.tellg()));
            ifs.seekg(0, std::ios::beg);
            str.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
            if (str.empty())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            return Result(ShaderCode{.string = str});
        }
        std::string err = "timeout while trying to read file: \"";
        err += result_path.value().string() + "\"";
        return Result<ShaderCode>(err);
    }

    void process_include(ImplPipelineManager * impl_pipeline_manager, daxa::Result<daxa::ShaderCode> const & shader_code_result, std::filesystem::path const & full_path, GlslangWrapperHeaderResult & result)
    {
        auto search_pred = [&](std::filesystem::path const & p)
        { return p == full_path; };
        if (std::find_if(
                impl_pipeline_manager->current_seen_shader_files.begin(),
                impl_pipeline_manager->current_seen_shader_files.end(),
                search_pred) != impl_pipeline_manager->current_seen_shader_files.end())
        {
            return;
        }
        if (shader_code_result.is_err())
        {
            return;
        }
        impl_pipeline_manager->current_observed_hotload_files->insert({full_path, std::chrono::file_clock::now()});

        std::string headerName = {};
        char const * headerData = nullptr;

        auto const & shader_code_str = shader_code_result.value().string;
        char * res_content = new char[shader_code_str.size() + 1];
        memcpy(res_content, shader_code_str.data(), shader_code_str.size() + 1);
        headerData = res_content;

        headerName = full_path.string();
        for (auto & c : headerName)
        {
            if (c == '\\')
            {
                c = '/';
            }
        }
        char * res_name = new char[headerName.size() + 1];
        memcpy(res_name, headerName.data(), headerName.size() + 1);

        result.header_name = res_name;
        result.header_name_length = headerName.size();
        result.header_code = res_content;
        result.header_code_length = shader_code_str.size();
    }

    auto includeLocal(void * user_pointer, char const * header_name, char const * includer_name, GlslangWrapperHeaderResult & result)
    {
        auto * impl_pipeline_manager = reinterpret_cast<ImplPipelineManager *>(user_pointer);
        auto header_name_str = std::string{header_name};
        auto result_path = impl_pipeline_manager->full_path_to_file(includer_name);
        if (result_path.is_err())
        {
            return;
        }
        auto full_path = result_path.value().parent_path() / header_name;
        auto shader_code_result = impl_pipeline_manager->load_shader_source_from_file(full_path);
        return process_include(impl_pipeline_manager, shader_code_result, full_path, result);
    }

    auto includeSystem(void * user_pointer, char const * header_name, char const * includer_name, GlslangWrapperHeaderResult & result)
    {
        auto * impl_pipeline_manager = reinterpret_cast<ImplPipelineManager *>(user_pointer);
        auto header_name_str = std::string{header_name};
        auto result_path = impl_pipeline_manager->full_path_to_file(header_name);
        if (result_path.is_err())
        {
            return;
        }
        auto full_path = result_path.value();
        auto shader_code_result = impl_pipeline_manager->load_shader_source_from_file(full_path);
        process_include(impl_pipeline_manager, shader_code_result, full_path, result);
    }

    void release_string_cb(char const * str)
    {
        delete[] str;
    }

    auto ImplPipelineManager::get_spirv_glslang([[maybe_unused]] ShaderCompileInfo const & shader_info, [[maybe_unused]] std::string const & debug_name_opt, [[maybe_unused]] ShaderStage shader_stage, [[maybe_unused]] ShaderCode const & code, uint64_t shader_info_hash) -> Result<std::vector<u32>>
    {
#if DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_GLSLANG

        auto translate_shader_stage = [](ShaderStage stage) -> EShLanguage
        {
            switch (stage)
            {
            case ShaderStage::COMP: return EShLanguage::EShLangCompute;
            case ShaderStage::VERT: return EShLanguage::EShLangVertex;
            case ShaderStage::FRAG: return EShLanguage::EShLangFragment;
            case ShaderStage::TESS_CONTROL: return EShLanguage::EShLangTessControl;
            case ShaderStage::TESS_EVAL: return EShLanguage::EShLangTessEvaluation;
            case ShaderStage::TASK: return EShLanguage::EShLangTask;
            case ShaderStage::MESH: return EShLanguage::EShLangMesh;
            case ShaderStage::RAY_GEN: return EShLanguage::EShLangRayGen;
            case ShaderStage::RAY_ANY_HIT: return EShLanguage::EShLangAnyHit;
            case ShaderStage::RAY_CLOSEST_HIT: return EShLanguage::EShLangClosestHit;
            case ShaderStage::RAY_MISS: return EShLanguage::EShLangMiss;
            case ShaderStage::RAY_INTERSECT: return EShLanguage::EShLangIntersect;
            case ShaderStage::RAY_CALLABLE: return EShLanguage::EShLangCallable;
            default: return EShLanguage::EShLangCount;
            }
        };

        std::string preamble;
        auto spirv_stage = translate_shader_stage(shader_stage);

        for (auto const & shader_define : shader_info.defines)
        {
            if (!shader_define.value.empty())
            {
                preamble += std::string("#define ") + shader_define.name + " " + shader_define.value + "\n";
            }
            else
            {
                preamble += std::string("#define ") + shader_define.name + "\n";
            }
        }
        std::string name = shader_info.source_path.string();
        if (name.empty() && !debug_name_opt.empty())
        {
            name = debug_name_opt;
        }

        auto entry_point = "main";
        auto source_entry = shader_info.entry_point.value_or("main");

        uint32_t * spv_ptr = nullptr;
        uint32_t spv_size = 0;

        char const * error_str_ptr = nullptr;
        uint32_t error_str_size = 0;

        bool print_cmd_command = false;
        if (print_cmd_command)
        {
            std::cout << "glslang --glsl-version 460 --target-env vulkan1.3 -V -g0 ^\n    ";
            for (auto const & root : shader_info.root_paths)
            {
                std::cout << "-I" << root.string() << " ^\n    ";
            }
            for (auto const & shader_define : shader_info.defines)
            {
                std::cout << "--D " << shader_define.name;
                if (!shader_define.value.empty())
                {
                    std::cout << "=" << shader_define.value;
                }
                std::cout << " ^\n    ";
            }
            switch (spirv_stage)
            {
            case EShLanguage::EShLangVertex: std::cout << "-S vert "; break;
            case EShLanguage::EShLangTessControl: std::cout << "-S tesc "; break;
            case EShLanguage::EShLangTessEvaluation: std::cout << "-S tese "; break;
            case EShLanguage::EShLangGeometry: std::cout << "-S geom "; break;
            case EShLanguage::EShLangFragment: std::cout << "-S frag "; break;
            case EShLanguage::EShLangCompute: std::cout << "-S comp "; break;
            case EShLanguage::EShLangRayGen: std::cout << "-S rgen "; break;
            case EShLanguage::EShLangIntersect: std::cout << "-S rint "; break;
            case EShLanguage::EShLangAnyHit: std::cout << "-S rahit "; break;
            case EShLanguage::EShLangClosestHit: std::cout << "-S rchit "; break;
            case EShLanguage::EShLangMiss: std::cout << "-S rmiss "; break;
            case EShLanguage::EShLangCallable: std::cout << "-S rcall "; break;
            case EShLanguage::EShLangMesh: std::cout << "-S mesh "; break;
            case EShLanguage::EShLangTask: std::cout << "-S task "; break;
            default: break;
            }
            auto full_path = full_path_to_file(shader_info.source_path).value();

            std::cout << "--entry-point " << entry_point << " --source-entrypoint " << source_entry << " \"shaders/" << name << "\" -o \"build/spv/" << shader_info_hash << ".spv\"\n\n"
                      << std::flush;
        }

        glslang_wrapper_compile(GlslangWrapperCompileInfo{
            .stage = spirv_stage,
            .preamble = preamble.c_str(),
            .shader_glsl = code.string.c_str(),
            .shader_name = name.c_str(),
            .entry_point = entry_point,
            .source_entry = source_entry.c_str(),
            .use_debug_info = shader_info.enable_debug_info.value_or(false),

            .include_local_cb = includeLocal,
            .include_system_cb = includeSystem,
            .release_string_cb = release_string_cb,
            .user_pointer = this,

            .out_spv_ptr = &spv_ptr,
            .out_spv_size = &spv_size,

            .out_error_str = &error_str_ptr,
            .out_error_str_size = &error_str_size,
        });

        if (spv_ptr == nullptr)
        {
            auto result = Result<std::vector<u32>>(std::string(error_str_ptr, error_str_size));
            glslang_wrapper_release_results(spv_ptr, error_str_ptr);
            return result;
        }

        auto spv = std::span(spv_ptr, spv_size);
        auto result = Result<std::vector<u32>>(std::vector<uint32_t>(spv.begin(), spv.end()));
        glslang_wrapper_release_results(spv_ptr, error_str_ptr);
        return result;
#else
        return Result<std::vector<u32>>("Asked for glslang compilation without enabling glslang");
#endif
    }

    auto ImplPipelineManager::get_spirv_slang([[maybe_unused]] ShaderCompileInfo const & shader_info, [[maybe_unused]] ShaderStage shader_stage, [[maybe_unused]] ShaderCode const & code) -> Result<std::vector<u32>>
    {
#if DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_SLANG
        auto session = Slang::ComPtr<slang::ISession>{};

        {
            auto search_paths_strings = std::vector<std::string>{};
            auto search_paths = std::vector<char const *>{};
            search_paths_strings.reserve(shader_info.root_paths.size());
            search_paths.reserve(shader_info.root_paths.size());
            for (auto const & path : shader_info.root_paths)
            {
                search_paths_strings.push_back(path.string());
                search_paths.push_back(search_paths_strings.back().c_str());
            }

            auto macros = std::vector<slang::PreprocessorMacroDesc>{};
            macros.reserve(shader_info.defines.size());
            for (auto const & [name, value] : shader_info.defines)
            {
                macros.push_back({name.c_str(), value.c_str()});
            }

            auto target_desc = slang::TargetDesc{};
            target_desc.format = SlangCompileTarget::SLANG_SPIRV;
            auto session_lock = std::lock_guard{slang_backend.session_mtx};
            target_desc.profile = slang_backend.global_session->findProfile("spirv_1_4");
            target_desc.flags = SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;

            // NOTE(grundlett): Does GLSL here refer to SPIR-V?
            target_desc.forceGLSLScalarBufferLayout = true;

            auto session_desc = slang::SessionDesc{};
            session_desc.targets = &target_desc;
            session_desc.targetCount = 1;
            session_desc.searchPaths = search_paths.data();
            session_desc.searchPathCount = search_paths.size();
            session_desc.preprocessorMacros = macros.data();
            session_desc.preprocessorMacroCount = macros.size();
            session_desc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;

            slang_backend.global_session->createSession(session_desc, session.writeRef());
        }

        auto name = std::string{"test"};
        auto error_message_prefix = std::string("SLANG [") + name + "] ";

        Slang::ComPtr<SlangCompileRequest> slangRequest = nullptr;
        session->createCompileRequest(slangRequest.writeRef());
        if (slangRequest == nullptr)
        {
            return Result<std::vector<u32>>(std::string_view{"internal error: session->createCompileRequest(&slangRequest) returned nullptr"});
        }
        std::array<char const *, 3> cmd_args = {
            // https://github.com/shader-slang/slang/issues/3532
            // Disables warning for aliasing bindings.
            // clang-format off
            "-warnings-disable", "39001",
            "-O0",
            // clang-format on
        };
        slangRequest->processCommandLineArguments(cmd_args.data(), static_cast<int>(cmd_args.size()));

        auto const filename = "_daxa_file";

        int translationUnitIndex = slangRequest->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, filename);
        slangRequest->addTranslationUnitSourceString(translationUnitIndex, "_daxa_slang_main", code.string.c_str());

        SlangResult const compileRes = slangRequest->compile();
        auto diagnostics = slangRequest->getDiagnosticOutput();

        if (SLANG_FAILED(compileRes))
        {
            return Result<std::vector<u32>>(error_message_prefix + diagnostics);
        }
        auto const dependency_n = slangRequest->getDependencyFileCount();
        for (int32_t dependency_i = 0; dependency_i < dependency_n; ++dependency_i)
        {
            auto const * const dep_path = slangRequest->getDependencyFilePath(dependency_i);
            if (std::strcmp(dep_path, "_daxa_slang_main") != 0)
            {
                current_observed_hotload_files->insert({dep_path, std::chrono::file_clock::now()});
            }
        }

        Slang::ComPtr<slang::IModule> shader_module = {};
        auto * refl = slangRequest->getReflection();
        auto entry_point_n = spReflection_getEntryPointCount(refl);
        int32_t entry_point_index = -1;
        for (uint32_t i = 0; i < entry_point_n; ++i)
        {
            auto * entry_refl = spReflection_getEntryPointByIndex(refl, i);
            auto const * entry_name = spReflectionEntryPoint_getName(entry_refl);
            if (strcmp(entry_name, shader_info.entry_point.value().c_str()) == 0)
            {
                entry_point_index = i;
                break;
            }
        }
        if (entry_point_index == -1)
        {
            return Result<std::vector<u32>>(error_message_prefix + "Failed to find entry point '" + shader_info.entry_point.value() + "' in module");
        }

        slangRequest->getModule(translationUnitIndex, shader_module.writeRef());

        auto spirv_code = Slang::ComPtr<slang::IBlob>{};
        {
            auto result = slangRequest->getEntryPointCodeBlob(entry_point_index, 0, spirv_code.writeRef());
            if (result != 0)
            {
                return Result<std::vector<u32>>(error_message_prefix + slangRequest->getDiagnosticOutput());
            }
        }

        auto result_span = std::span<u32 const>{static_cast<u32 const *>(spirv_code->getBufferPointer()), spirv_code->getBufferSize() / sizeof(u32)};
        auto result = std::vector<u32>{};
        result.insert(result.begin(), result_span.begin(), result_span.end());

        return Result<std::vector<u32>>(result);
#else
        return Result<std::vector<u32>>("Asked for Slang compilation without enabling Slang");
#endif
    }

    auto ImplPipelineManager::zero_ref_callback(ImplHandle const * handle)
    {
        auto const * self = r_cast<ImplPipelineManager const *>(handle);
        delete self;
    }

    auto PipelineManager::inc_refcnt(ImplHandle const * object) -> u64
    {
        return object->inc_refcnt();
    }

    auto PipelineManager::dec_refcnt(ImplHandle const * object) -> u64
    {
        return object->dec_refcnt(
            ImplPipelineManager::zero_ref_callback,
            nullptr);
    }

} // namespace daxa

#endif
