#pragma once

#include <utils/math.hpp>

struct Chunk {
    static constexpr glm::ivec3 DIM = glm::ivec3(64);

    template <typename T>
    using BlockArray = std::array<std::array<std::array<T, DIM.x>, DIM.y>, DIM.z>;
};
