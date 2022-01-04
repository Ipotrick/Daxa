#pragma once

#include "../DaxaCore.hpp"

namespace daxa {
    struct TransformComp {
        glm::mat4 translation = {};
    };

    struct ChildComp {
        daxa::EntityHandle parent = {};
    };
}