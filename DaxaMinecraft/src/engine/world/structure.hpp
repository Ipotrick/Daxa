#pragma once
#include <glm/glm.hpp>
#include "block.hpp"

struct Chunk;

enum class StructureID : uint32_t
{
    Tree,
    VillageSpawn,
};

struct Structure {
    StructureID id;
    glm::ivec3  pos;
    Chunk *     home_chunk;
};

struct Tree {
    static BlockID get_tile(glm::ivec3 i) {
        int index = i.x + i.y + i.z;
        if (index < 0) {
            return BlockID::Air;
        }
        switch (index) {
        case 0: return BlockID::Log;
        case 1: return BlockID::Planks;
        case 2: return BlockID::Leaves;
        case 3: return BlockID::Bricks;
        case 4: return BlockID::DiamondOre;
        case 5: return BlockID::Gravel;
        default: return BlockID::Sand;
        }
    }
};
