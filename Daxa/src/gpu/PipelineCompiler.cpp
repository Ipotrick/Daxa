#include "PipelineCompiler.hpp"

#include <fstream>

using Path = std::filesystem::path;

namespace daxa {
    void PipelineCompiler::addShaderSourceRootPath(Path const& root) {
        this->rootPaths.push_back(root); 
    }

    Result<Path> PipelineCompiler::findFullPathOfFile(Path const& file) {
        Path potentialPath;
        for (auto& root : this->rootPaths) {
            potentialPath.clear();
            potentialPath = root / file;

            std::ifstream ifs{potentialPath};
            if (ifs.good()) {
                return { potentialPath };
            }
        }
        std::string errorMessage;
        errorMessage += "could not find file :\"";
        errorMessage += file.string();
        errorMessage += "\"";
        return ResultErr{ .message = std::move(errorMessage) };
    }
}