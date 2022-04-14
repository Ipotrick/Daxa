#if USE_NSIGHT_AFTERMATH
//*********************************************************
//
// Copyright (c) 2019-2020, NVIDIA CORPORATION. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//
//*********************************************************

#include <fstream>
#include <iomanip>

#include "NsightAftermathShaderDatabase.h"

//*********************************************************
// ShaderDatabase implementation
//*********************************************************

ShaderDatabase::ShaderDatabase()
    : m_shaderBinaries()
    , m_shaderBinariesWithDebugInfo()
{
    // Add shader binaries to database
    AddShaderBinary("cube.vert.spirv");
    AddShaderBinary("cube.frag.spirv");

    // Add the not stripped shader binaries to the database, too.
    AddShaderBinaryWithDebugInfo("cube.vert.spirv", "cube.vert.full.spirv");
    AddShaderBinaryWithDebugInfo("cube.frag.spirv", "cube.frag.full.spirv");
}

ShaderDatabase::~ShaderDatabase()
{
}

bool ShaderDatabase::ReadFile(const char* filename, std::vector<uint8_t>& data)
{
    std::ifstream fs(filename, std::ios::in | std::ios::binary);
    if (!fs)
    {
        return false;
    }

    fs.seekg(0, std::ios::end);
    data.resize(fs.tellg());
    fs.seekg(0, std::ios::beg);
    fs.read(reinterpret_cast<char*>(data.data()), data.size());
    fs.close();

    return true;
}

void ShaderDatabase::AddShaderBinary(const char* shaderFilePath)
{
    // Read the shader binary code from the file
    std::vector<uint8_t> data;
    if (!ReadFile(shaderFilePath, data))
    {
        return;
    }

   // Create shader hash for the shader
    const GFSDK_Aftermath_SpirvCode shader{ data.data(), uint32_t(data.size()) };
    GFSDK_Aftermath_ShaderBinaryHash shaderHash;
    AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GetShaderHashSpirv(
        GFSDK_Aftermath_Version_API,
        &shader,
        &shaderHash));

    // Store the data for shader mapping when decoding GPU crash dumps.
    // cf. FindShaderBinary()
    m_shaderBinaries[shaderHash].swap(data);
}

void ShaderDatabase::AddShaderBinaryWithDebugInfo(const char* strippedShaderFilePath, const char* shaderFilePath)
{
    // Read the shader debug data from the file
    std::vector<uint8_t> data;
    if (!ReadFile(shaderFilePath, data))
    {
        return;
    }
    std::vector<uint8_t> strippedData;
    if (!ReadFile(strippedShaderFilePath, strippedData))
    {
        return;
    }

    // Generate shader debug name.
    GFSDK_Aftermath_ShaderDebugName debugName;
    const GFSDK_Aftermath_SpirvCode shader{ data.data(), uint32_t(data.size()) };
    const GFSDK_Aftermath_SpirvCode strippedShader{ strippedData.data(), uint32_t(strippedData.size()) };
    AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GetShaderDebugNameSpirv(
        GFSDK_Aftermath_Version_API,
        &shader,
        &strippedShader,
        &debugName));

    // Store the data for shader instruction address mapping when decoding GPU crash dumps.
    // cf. FindShaderBinaryWithDebugData()
    m_shaderBinariesWithDebugInfo[debugName].swap(data);
}

// Find a shader binary by shader hash.
bool ShaderDatabase::FindShaderBinary(const GFSDK_Aftermath_ShaderBinaryHash& shaderHash, std::vector<uint8_t>& shader) const
{
    // Find shader binary data for the shader hash
    auto i_shader = m_shaderBinaries.find(shaderHash);
    if (i_shader == m_shaderBinaries.end())
    {
        // Nothing found.
        return false;
    }

    shader = i_shader->second;
    return true;
}

// Find a shader binary with debug information by shader debug name.
bool ShaderDatabase::FindShaderBinaryWithDebugData(const GFSDK_Aftermath_ShaderDebugName& shaderDebugName, std::vector<uint8_t>& shader) const
{
    // Find shader binary for the shader debug name.
    auto i_shader = m_shaderBinariesWithDebugInfo.find(shaderDebugName);
    if (i_shader == m_shaderBinariesWithDebugInfo.end())
    {
        // Nothing found.
        return false;
    }

    shader = i_shader->second;
    return true;
}
#endif