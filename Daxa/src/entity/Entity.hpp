#pragma once

#include "../DaxaCore.hpp"

namespace daxa {
    using EntityIndex = u32;
    using EntityVersion = u32;
    static constexpr inline EntityIndex INVALID_ENTITY_INDEX = u32(-1);
    static constexpr inline EntityIndex INVALID_ENTITY_VERSION = u32(-1);

    struct EntityHandle {
        EntityIndex index = u32(-1);
        EntityVersion version = u32(-1);
    };
}