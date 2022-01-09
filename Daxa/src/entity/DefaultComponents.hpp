#pragma once

#include "../DaxaCore.hpp"

namespace daxa {
    struct TransformComp {
        glm::mat4 mat = {};
    };

    struct ChildComp {
        daxa::EntityHandle parent = {};
    };
}