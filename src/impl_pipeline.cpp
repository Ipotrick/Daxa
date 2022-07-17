#include "impl_pipeline.hpp"
#include "impl_swapchain.hpp"
#include "impl_device.hpp"

#include <regex>
#include <iostream>
#include <thread>

static const std::regex PRAGMA_ONCE_REGEX = std::regex(R"reg(#\s*pragma\s*once\s*)reg");
static const std::regex REPLACE_REGEX = std::regex(R"reg(\W)reg");
static void shader_preprocess(std::string & file_str, std::filesystem::path const & path)
{
    std::smatch matches = {};
    std::string line = {};
    std::stringstream file_ss{file_str};
    std::stringstream result_ss = {};
    bool has_pragma_once = false;
    auto abspath_str = std::filesystem::absolute(path).string();
    for (std::size_t line_num = 0; std::getline(file_ss, line); ++line_num)
    {
        if (std::regex_match(line, matches, PRAGMA_ONCE_REGEX))
        {
            result_ss << "#if !defined(";
            std::regex_replace(std::ostreambuf_iterator<char>(result_ss), abspath_str.begin(), abspath_str.end(), REPLACE_REGEX, "_");
            result_ss << ")\n";
            has_pragma_once = true;
        }
        else
        {
            result_ss << line << "\n";
        }
    }
    if (has_pragma_once)
    {
        result_ss << "\n#define ";
        std::regex_replace(std::ostreambuf_iterator<char>(result_ss), abspath_str.begin(), abspath_str.end(), REPLACE_REGEX, "_");
        result_ss << "\n#endif";
    }
    file_str = result_ss.str();
}

namespace daxa
{

    struct DxcCustomIncluder : public IDxcIncludeHandler
    {
        IDxcIncludeHandler * default_includer;
        ImplPipelineCompiler * impl_pipeline_compiler;

        virtual ~DxcCustomIncluder() {}
        HRESULT LoadSource(LPCWSTR filename, IDxcBlob ** include_source) override
        {
            if (filename[0] == '.')
            {
                filename += 2;
            }

            auto result = impl_pipeline_compiler->full_path_to_file(filename);
            if (result.isErr())
            {
                *include_source = nullptr;
                return SCARD_E_FILE_NOT_FOUND;
            }
            auto full_path = result.value();
            auto search_pred = [&](std::filesystem::path const & p)
            { return p == full_path; };

            ComPtr<IDxcBlobEncoding> dxc_blob_encoding = {};
            // if (std::find_if(sharedData->currentShaderSeenFiles.begin(), sharedData->currentShaderSeenFiles.end(), search_pred) != sharedData->currentShaderSeenFiles.end())
            // {
            //     // Return empty string blob if this file has been included before
            //     static const char nullStr[] = " ";
            //     pUtils->CreateBlob(nullStr, ARRAYSIZE(nullStr), CP_UTF8, pEncoding.GetAddressOf());
            //     *ppIncludeSource = pEncoding.Detach();
            //     return S_OK;
            // }
            // else
            // {
            //     sharedData->observedHotLoadFiles->insert({fullPath, std::chrono::file_clock::now()});
            // }

            auto str = impl_pipeline_compiler->load_shader_source_from_file(full_path);
            // if (str_result.isErr())
            // {
            //     *ppIncludeSource = nullptr;
            //     return SCARD_E_INVALID_PARAMETER;
            // }
            // std::string str = str_result.value();

            impl_pipeline_compiler->dxc_utils->CreateBlob(str.string.c_str(), static_cast<u32>(str.string.size()), CP_UTF8, dxc_blob_encoding.GetAddressOf());
            *include_source = dxc_blob_encoding.Detach();
            return S_OK;
        }

        HRESULT QueryInterface(REFIID riid, void ** object) override
        {
            return default_includer->QueryInterface(riid, object);
        }

        unsigned long STDMETHODCALLTYPE AddRef(void) override { return 0; }
        unsigned long STDMETHODCALLTYPE Release(void) override { return 0; }
    };

    ComputePipeline::ComputePipeline(std::shared_ptr<void> a_impl) : Handle(std::move(a_impl)) {}

    ComputePipeline::~ComputePipeline()
    {
        if (this->impl.use_count() == 1)
        {
            std::shared_ptr<ImplComputePipeline> impl = std::static_pointer_cast<ImplComputePipeline>(this->impl);
            std::unique_lock lock{DAXA_LOCK_WEAK(impl->impl_device)->main_queue_zombies_mtx};
            u64 main_queue_cpu_timeline_value = DAXA_LOCK_WEAK(impl->impl_device)->main_queue_cpu_timeline.load(std::memory_order::relaxed);
            DAXA_LOCK_WEAK(impl->impl_device)->main_queue_compute_pipeline_zombies.push_back({main_queue_cpu_timeline_value, impl});
        }
    }

    PipelineCompiler::PipelineCompiler(std::shared_ptr<void> a_impl) : Handle(std::move(a_impl)) {}

    auto PipelineCompiler::create_compute_pipeline(ComputePipelineInfo const & info) -> Result<ComputePipeline>
    {
        if (info.push_constant_size > MAX_PUSH_CONSTANT_BYTE_SIZE) 
        {
            return ResultErr{ std::string("push constant size of ") + std::to_string(info.push_constant_size) + std::string(" exceeds the maximum size of ") + std::to_string(MAX_PUSH_CONSTANT_BYTE_SIZE)};
        }
        if (info.push_constant_size % 4 != 0) 
        {
            return ResultErr{ std::string("push constant size of ") + std::to_string(info.push_constant_size) + std::string(" is not a multiple of 4(bytes)") };
        }

        std::vector<u32> spirv = {};
        auto & impl = *reinterpret_cast<ImplPipelineCompiler *>(this->impl.get());

        if (info.shader_info.source.index() == 2)
        {
            ShaderSPIRV const & input_spirv = std::get<ShaderSPIRV>(info.shader_info.source);
            spirv.resize(input_spirv.size);
            for (usize i = 0; i < input_spirv.size; ++i)
            {
                spirv[i] = input_spirv.data[i];
            }
        }
        else
        {
            ShaderCode code = {};
            if (auto shader_source = std::get_if<ShaderFile>(&info.shader_info.source))
            {
                auto ret = impl.full_path_to_file(shader_source->path);
                if (ret.isErr())
                {
                    return ResultErr{ ret.message() };
                }
                
                code = impl.load_shader_source_from_file(ret.value());
            }
            else
            {
                code = std::get<ShaderCode>(info.shader_info.source);
            }

            auto ret = impl.gen_spirv_from_dxc(info.shader_info, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, code);
            if (ret.isErr())
            {
                return ResultErr{ ret.message() };
            }
            spirv = ret.value();
        }

        VkShaderModule shader_module = {};

        VkShaderModuleCreateInfo shader_module_ci{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .codeSize = static_cast<u32>(spirv.size() * sizeof(u32)),
            .pCode = spirv.data(),
        };
        vkCreateShaderModule(DAXA_LOCK_WEAK(impl.impl_device)->vk_device, &shader_module_ci, nullptr, &shader_module);

        auto ret = ComputePipeline{std::make_shared<ImplComputePipeline>(impl.impl_device, info, shader_module)};

        vkDestroyShaderModule(DAXA_LOCK_WEAK(impl.impl_device)->vk_device, shader_module, nullptr);

        return { ret };
    }

    ImplPipelineCompiler::ImplPipelineCompiler(std::weak_ptr<ImplDevice> a_impl_device, PipelineCompilerInfo const & info)
        : impl_device{std::move(a_impl_device)}, info{info}
    {
        HRESULT dxc_utils_result = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&this->dxc_utils));
        DAXA_DBG_ASSERT_TRUE_M(SUCCEEDED(dxc_utils_result), "Failed to create DXC utils");

        HRESULT dxc_compiler_result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&this->dxc_compiler));
        DAXA_DBG_ASSERT_TRUE_M(SUCCEEDED(dxc_compiler_result), "Failed to create DXC compiler");

        ComPtr<DxcCustomIncluder> dxc_includer = new DxcCustomIncluder();
        dxc_includer->impl_pipeline_compiler = this;
        this->dxc_utils->CreateDefaultIncludeHandler(&(dxc_includer->default_includer));
        this->dxc_includer = dxc_includer.Detach();
    }

    ImplPipelineCompiler::~ImplPipelineCompiler()
    {
    }

    auto ImplPipelineCompiler::full_path_to_file(std::filesystem::path const & file) -> Result<std::filesystem::path>
    {
        if (std::filesystem::exists(file))
        {
            return {file};
        }
        std::filesystem::path potential_path;
        for (auto & root : this->info.root_paths)
        {
            potential_path.clear();
            potential_path = root / file;
            if (std::filesystem::exists(potential_path))
            {
                return {potential_path};
            }
        }
        std::string error_msg = {};
        error_msg += "could not find file :\"";
        error_msg += file.string();
        error_msg += "\"";
        return ResultErr{.message = std::move(error_msg)};
    }

    auto ImplPipelineCompiler::load_shader_source_from_file(std::filesystem::path const & path) -> ShaderCode
    {
        // auto start_time = std::chrono::steady_clock::now();
        // while ((std::chrono::steady_clock::now() - start_time).count() < 100'000'000)
        // {
        // }
        std::ifstream ifs{path};
        DAXA_DBG_ASSERT_TRUE_M(ifs.good(), "Could not open shader file");
        std::string str = {};
        ifs.seekg(0, std::ios::end);
        str.reserve(ifs.tellg());
        ifs.seekg(0, std::ios::beg);
        str.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
        if (str.size() < 1)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            // continue;
        }
        shader_preprocess(str, path);
        return {.string = str};

        // return {.string = {}};
    }

    auto ImplPipelineCompiler::gen_spirv_from_dxc(ShaderInfo const & shader_info, VkShaderStageFlagBits shader_stage, ShaderCode const & code) -> Result<std::vector<u32>>
    {
        auto u8_ascii_to_wstring = [](char const * str) -> std::wstring
        {
            std::wstring ret = {};
            for (int i = 0; i < std::strlen(str) + 1 && str != nullptr; i++)
            {
                ret.push_back(str[i]);
            }
            return ret;
        };

        std::vector<const wchar_t *> args = {};

        std::vector<std::wstring> wstring_buffer = {};

        wstring_buffer.reserve(shader_info.defines.size());

        for (auto & define : shader_info.defines)
        {
            wstring_buffer.push_back(u8_ascii_to_wstring(define.c_str()));
            args.push_back(L"-D");
            args.push_back(wstring_buffer.back().c_str());
        }

        if (shader_info.source.index() == 0)
        {
            wstring_buffer.push_back(std::get<ShaderFile>(shader_info.source).path.wstring());
            args.push_back(wstring_buffer.back().c_str());
        }

        for (auto & root : info.root_paths)
        {
            args.push_back(L"-I");
            wstring_buffer.push_back(root.wstring());
            args.push_back(wstring_buffer.back().c_str());
        }

        // set matrix packing to column major
        args.push_back(L"-Zpc");
        // set warnings as errors
        args.push_back(DXC_ARG_WARNINGS_ARE_ERRORS); //-WX
        // setting target
        args.push_back(L"-spirv");
        args.push_back(L"-fspv-target-env=vulkan1.1");
        // set optimization setting
        args.push_back(L"-O1");
        // setting entry point
        args.push_back(L"-E");
        auto entry_point_wstr = u8_ascii_to_wstring(shader_info.entry_point.c_str());
        args.push_back(entry_point_wstr.c_str());

        // set shader model
        args.push_back(L"-T");
        std::wstring profile;
        switch (shader_stage)
        {
        case VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT: profile = L"vs_6_0"; break;
        case VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT: profile = L"ps_6_0"; break;
        case VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT: profile = L"cs_6_0"; break;
        default: break;
        }
        args.push_back(profile.c_str());
        // set hlsl version to 2021
        args.push_back(L"-HV");
        args.push_back(L"2021");
        DxcBuffer source_buffer{
            .Ptr = code.string.c_str(),
            .Size = static_cast<u32>(code.string.size()),
            .Encoding = static_cast<u32>(0),
        };

        IDxcResult * result;
        // this->dxc_compiler->Compile(nullptr, nullptr, 0, dxc_includer, IID_PPV_ARGS(&result));
        this->dxc_compiler->Compile(&source_buffer, args.data(), static_cast<u32>(args.size()), dxc_includer, IID_PPV_ARGS(&result));
        IDxcBlobUtf8 * error_message;
        result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&error_message), nullptr);

        if (error_message && error_message->GetStringLength() > 0)
        {
            auto str = std::string();
            str.resize(error_message->GetBufferSize());
            for (usize i = 0; i < str.size(); i++)
            {
                str[i] = static_cast<char const *>(error_message->GetBufferPointer())[i];
            }
            str = std::string("DXC: ") + str;
            std::cerr << str << std::endl;
            return daxa::ResultErr{.message = str};
        }

        IDxcBlob * shaderobj;
        result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderobj), nullptr);

        std::vector<u32> spv;
        spv.resize(shaderobj->GetBufferSize() / sizeof(u32));
        for (size_t i = 0; i < spv.size(); i++)
        {
            spv[i] = static_cast<u32 *>(shaderobj->GetBufferPointer())[i];
        }

        return { spv };
    }

    ImplComputePipeline::ImplComputePipeline(std::weak_ptr<ImplDevice> a_impl_device, ComputePipelineInfo const & info, VkShaderModule vk_shader_module)
        : impl_device{std::move(a_impl_device)}, info{info}
    {
        this->vk_pipeline_layout = DAXA_LOCK_WEAK(this->impl_device)->gpu_table.pipeline_layouts[info.push_constant_size / 4];

        VkComputePipelineCreateInfo vk_compute_pipeline_create_info{
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .stage = VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .stage = VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT,
                .module = vk_shader_module,
                .pName = this->info.shader_info.entry_point.c_str(),
                .pSpecializationInfo = nullptr,
            },
            .layout = this->vk_pipeline_layout,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = 0,
        };

        auto result = vkCreateComputePipelines(
            DAXA_LOCK_WEAK(this->impl_device)->vk_device,
            VK_NULL_HANDLE,
            1u,
            &vk_compute_pipeline_create_info,
            nullptr,
            &this->vk_pipeline);

        DAXA_DBG_ASSERT_TRUE_M(result == VK_SUCCESS, "failed to create compute pipeline");

        if (this->info.debug_name.size() > 0)
        {
            VkDebugUtilsObjectNameInfoEXT name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_PIPELINE,
                .objectHandle = reinterpret_cast<uint64_t>(this->vk_pipeline),
                .pObjectName = this->info.debug_name.c_str(),
            };
            vkSetDebugUtilsObjectNameEXT(impl_device.lock()->vk_device, &name_info);
        }
    }

    ImplComputePipeline::~ImplComputePipeline()
    {
        vkDestroyPipeline(DAXA_LOCK_WEAK(this->impl_device)->vk_device, this->vk_pipeline, nullptr);
    }
} // namespace daxa
