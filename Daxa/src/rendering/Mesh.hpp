#pragma once

#include "../DaxaCore.hpp"

#include "../gpu/Device.hpp"

#include <optional>
#include <filesystem>

#define GLM_DEPTH_ZERO_TO_ONEW
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace daxa {

    class Mesh {
    public:
        static std::optional<Mesh> tryLoadFromGLTF2(std::filesystem::path path);
    private:
        glm::mat4 transform = {};
        BufferHandle indices = {};
        BufferHandle vertecies = {};
        std::vector<ImageHandle> textures = {};
    };

}