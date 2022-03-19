#pragma once

#include <glm/glm.hpp>
#include <variant>

enum class BlockID : uint32_t {
    Air,
    Dirt,
    Grass,
    TallGrass,
    Rose,
    Log,
    Leaves,
    Sand,
    Gravel,
    Planks,
    Bricks,
    Stone,
    DiamondOre,
    Water,
    Cactus,
};

enum class BlockFace : uint32_t {
    Back,
    Front,
    Left,
    Right,
    Bottom,
    Top,
};

struct Block {
    BlockID id;

    bool is_transparent() const {
        switch (id) {
        case BlockID::Air:
        case BlockID::Leaves:
        case BlockID::TallGrass:
        case BlockID::Rose: return true;
        default: return false;
        }
    }

    bool is_solid() const {
        switch (id) {
        case BlockID::Air:
        case BlockID::Leaves:
        case BlockID::TallGrass:
        case BlockID::Rose: return false;
        default: return true;
        }
    }

    bool is_not_drawn() const {
        switch (id) {
        case BlockID::Air: return true;
        default: return false;
        }
    }

    bool is_cube() const {
        switch (id) {
        case BlockID::TallGrass:
        case BlockID::Rose: return false;
        default: return true;
        }
    }

    glm::vec2 texture_face_offset(BlockFace face) const {
        switch (id) {
        case BlockID::Dirt:
            switch (face) {
            case BlockFace::Back:
            case BlockFace::Front:
            case BlockFace::Left:
            case BlockFace::Right:
            case BlockFace::Bottom:
            case BlockFace::Top:
            default: return glm::vec2{0, 0};
            }
            break;
        case BlockID::Grass:
            switch (face) {
            case BlockFace::Back:
            case BlockFace::Front:
            case BlockFace::Left:
            case BlockFace::Right: return glm::vec2{1, 0};
            case BlockFace::Bottom: return glm::vec2{0, 0};
            case BlockFace::Top: return glm::vec2{2, 0};
            default: return glm::vec2{0, 0};
            }
            break;
        case BlockID::TallGrass:
            switch (face) {
            case BlockFace::Back:
            case BlockFace::Front:
            case BlockFace::Left:
            case BlockFace::Right:
            case BlockFace::Bottom:
            case BlockFace::Top:
            default: return glm::vec2{3, 0};
            }
            break;
        case BlockID::Rose:
            switch (face) {
            case BlockFace::Back:
            case BlockFace::Front:
            case BlockFace::Left:
            case BlockFace::Right:
            case BlockFace::Bottom:
            case BlockFace::Top:
            default: return glm::vec2{3, 1};
            }
            break;
        case BlockID::Log:
            switch (face) {
            case BlockFace::Back:
            case BlockFace::Front:
            case BlockFace::Left:
            case BlockFace::Right: return glm::vec2{0, 1};
            case BlockFace::Bottom:
            case BlockFace::Top: return glm::vec2{1, 1};
            default: return glm::vec2{1, 1};
            }
            break;
        case BlockID::Leaves:
            switch (face) {
            case BlockFace::Back:
            case BlockFace::Front:
            case BlockFace::Left:
            case BlockFace::Right:
            case BlockFace::Bottom:
            case BlockFace::Top:
            default: return glm::vec2{2, 1};
            }
            break;
        case BlockID::Sand:
            switch (face) {
            case BlockFace::Back:
            case BlockFace::Front:
            case BlockFace::Left:
            case BlockFace::Right:
            case BlockFace::Bottom:
            case BlockFace::Top:
            default: return glm::vec2{0, 2};
            }
            break;
        case BlockID::Gravel:
            switch (face) {
            case BlockFace::Back:
            case BlockFace::Front:
            case BlockFace::Left:
            case BlockFace::Right:
            case BlockFace::Bottom:
            case BlockFace::Top:
            default: return glm::vec2{1, 2};
            }
            break;
        case BlockID::Planks:
            switch (face) {
            case BlockFace::Back:
            case BlockFace::Front:
            case BlockFace::Left:
            case BlockFace::Right:
            case BlockFace::Bottom:
            case BlockFace::Top:
            default: return glm::vec2{2, 2};
            }
            break;
        case BlockID::Bricks:
            switch (face) {
            case BlockFace::Back:
            case BlockFace::Front:
            case BlockFace::Left:
            case BlockFace::Right:
            case BlockFace::Bottom:
            case BlockFace::Top:
            default: return glm::vec2{3, 2};
            }
            break;
        case BlockID::Stone:
            switch (face) {
            case BlockFace::Back:
            case BlockFace::Front:
            case BlockFace::Left:
            case BlockFace::Right:
            case BlockFace::Bottom:
            case BlockFace::Top:
            default: return glm::vec2{0, 3};
            }
            break;
        case BlockID::DiamondOre:
            switch (face) {
            case BlockFace::Back:
            case BlockFace::Front:
            case BlockFace::Left:
            case BlockFace::Right:
            case BlockFace::Bottom:
            case BlockFace::Top:
            default: return glm::vec2{1, 3};
            }
            break;
        case BlockID::Water:
            switch (face) {
            case BlockFace::Back:
            case BlockFace::Front:
            case BlockFace::Left:
            case BlockFace::Right:
            case BlockFace::Bottom:
            case BlockFace::Top:
            default: return glm::vec2{2, 3};
            }
            break;
        case BlockID::Cactus:
            switch (face) {
            case BlockFace::Back:
            case BlockFace::Front:
            case BlockFace::Left:
            case BlockFace::Right:
            case BlockFace::Bottom:
            case BlockFace::Top:
            default: return glm::vec2{3, 3};
            }
            break;
        default: return glm::vec2{0, 0};
        }
    }
};

struct Chunk;

enum class StructureID : uint32_t {
    None,
    Tree,
    VillageSpawn,
};

struct Structure {
    StructureID id;
    glm::ivec3 pos;
    Chunk *home_chunk;
};

struct InvalidStructure {
    static inline glm::ivec3 MIN = glm::ivec3{0, 0, 0};
    static inline glm::ivec3 MAX = glm::ivec3{0, 0, 0};

    static void transform_tile(glm::ivec3, BlockID &) {}
};

struct Tree {
    static inline glm::ivec3 MIN = glm::ivec3{-2, 0, -2};
    static inline glm::ivec3 MAX = glm::ivec3{2, 7, 2};

    static void transform_tile(glm::ivec3 i, BlockID &out_id) {
        auto cutout = abs(i.x) + abs(i.z);
        if (i.y < 5 && i.x == 0 && i.z == 0) {
            out_id = BlockID::Log;
        } else if (i.y > 1) {
            if (-i.y + 11 - cutout * 2 > 3)
                out_id = BlockID::Leaves;
        }
    }
};

struct Village {
    static inline glm::ivec3 MIN = glm::ivec3{-2, 0, -2};
    static inline glm::ivec3 MAX = glm::ivec3{2, 5, 2};

    static void transform_tile(glm::ivec3 i, BlockID &out_id) {
        auto cutout = abs(i.x) + abs(i.z);
        if (i.y == 0) {
            out_id = BlockID::Stone;
        } else if (abs(i.x) == 2 && abs(i.z) == 2) {
            out_id = BlockID::Log;
        } else if (abs(i.x) == 2 || abs(i.z) == 2) {
            out_id = BlockID::Planks;
        } else {
            out_id = BlockID::Air;
        }
    }
};

using StructureTypes = std::variant<InvalidStructure, Tree, Village>;

enum class BiomeID : uint32_t {
    Plains,
    Forest,
    Desert,
};

struct TileInfo {
    BiomeID biome;

    BlockID biome_ground() {
        switch (biome) {
        case BiomeID::Plains:
        case BiomeID::Forest: return BlockID::Dirt;
        case BiomeID::Desert: return BlockID::Gravel;
        }
    }
    BlockID biome_surface() {
        switch (biome) {
        case BiomeID::Plains:
        case BiomeID::Forest: return BlockID::Grass;
        case BiomeID::Desert: return BlockID::Sand;
        }
    }

    void biome_structures(int random_int, glm::ivec3 pos,
                          const std::array<Block *, 5> &above,
                          std::vector<Structure> &structures, Chunk &chunk) {
        switch (biome) {
        case BiomeID::Plains: {
            if (random_int < 40000) {
                above[0]->id = BlockID::TallGrass;
            } else if (random_int < 41000) {
                above[0]->id = BlockID::Rose;
            } else if (random_int < 41030) {
                structures.push_back(Structure{
                    .id = StructureID::VillageSpawn,
                    .pos = pos + glm::ivec3{0, 1, 0},
                    .home_chunk = &chunk,
                });
            }
        } break;
        case BiomeID::Forest: {
            if (random_int < 20000) {
                above[0]->id = BlockID::TallGrass;
            } else if (random_int < 21000) {
                structures.push_back(Structure{
                    .id = StructureID::Tree,
                    .pos = pos + glm::ivec3{0, 1, 0},
                    .home_chunk = &chunk,
                });
            }
        } break;
        case BiomeID::Desert: {
            if (random_int < 1000) {
                for (int i = 0; i < 2 + random_int % 4; ++i) {
                    above[i]->id = BlockID::Cactus;
                }
            }
        } break;
        }
    }
};
