#pragma once

#include <glm/glm.hpp>
#include <variant>

enum class BlockID : uint32_t {
    Air,

    Brick,
    Cactus,
    Cobblestone,
    DiamondOre,
    Dirt,
    DriedShrub,
    Grass,
    Gravel,
    Leaves,
    Log,
    Planks,
    Rose,
    Sand,
    Sandstone,
    Stone,
    TallGrass,
    Water,
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
        case BlockID::DriedShrub:
        case BlockID::Leaves:
        case BlockID::Rose:
        case BlockID::TallGrass:
            return true;
        default: return false;
        }
    }

    bool is_solid() const {
        switch (id) {
        case BlockID::Air:
        case BlockID::DriedShrub:
        case BlockID::Leaves:
        case BlockID::Rose:
        case BlockID::TallGrass:
            return false;
        default: return true;
        }
    }

    bool is_not_drawn() const {
        switch (id) {
        case BlockID::Air:
            return true;
        default: return false;
        }
    }

    bool is_cube() const {
        switch (id) {
        case BlockID::DriedShrub:
        case BlockID::Rose:
        case BlockID::TallGrass:
            return false;
        default: return true;
        }
    }

    int texture_index(BlockFace face) const {
        switch (id) {
        case BlockID::Brick: return 0;
        case BlockID::Cactus: return 1;
        case BlockID::Cobblestone: return 2;
        case BlockID::DiamondOre: return 3;
        case BlockID::Dirt: return 4;
        case BlockID::DriedShrub: return 5;
        case BlockID::Grass:
            switch (face) {
            case BlockFace::Back:
            case BlockFace::Front:
            case BlockFace::Left:
            case BlockFace::Right: return 6;
            case BlockFace::Bottom: return 4;
            case BlockFace::Top: return 7;
            default: return 0;
            }
        case BlockID::Gravel: return 8;
        case BlockID::Leaves: return 9;
        case BlockID::Log:
            switch (face) {
            case BlockFace::Back:
            case BlockFace::Front:
            case BlockFace::Left:
            case BlockFace::Right: return 10;
            case BlockFace::Bottom:
            case BlockFace::Top: return 11;
            default: return 0;
            }
        case BlockID::Planks: return 12;
        case BlockID::Rose: return 13;
        case BlockID::Sand: return 14;
        case BlockID::Sandstone: return 15;
        case BlockID::Stone: return 16;
        case BlockID::TallGrass: return 17;
        case BlockID::Water: return 18;
        default: return 0;
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
    static inline glm::ivec3 MIN = glm::ivec3{-2, -1, -2};
    static inline glm::ivec3 MAX = glm::ivec3{2, 5, 2};

    static void transform_tile(glm::ivec3 i, BlockID &out_id) {
        auto cutout = abs(i.x) + abs(i.z);
        if (i.y <= 0) {
            out_id = BlockID::Cobblestone;
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

    void biome_ground(int random_int, glm::ivec3 pos, const std::array<Block *, 16> &above, BlockID &out_id) {
        int depth;
        bool under_water = false;
        for (depth = 0; depth < above.size(); ++depth) {
            if (above[depth]->is_transparent()) {
                break;
            } else if (above[depth]->id == BlockID::Water) {
                under_water = true;
                break;
            }
        }
        switch (biome) {
        case BiomeID::Plains:
        case BiomeID::Forest: {
            if (depth < 3 + random_int % 3) {
                if (under_water) {
                    out_id = BlockID::Gravel;
                } else {
                    out_id = BlockID::Dirt;
                }
            }
        } break;
        case BiomeID::Desert: {
            if (depth < 3 + random_int % 3) {
                out_id = BlockID::Sand;
            } else if (depth < 6 + random_int % 2) {
                if (under_water) {
                    out_id = BlockID::Gravel;
                } else {
                    out_id = BlockID::Sandstone;
                }
            }
        } break;
        }
    }
    BlockID biome_surface(int random_int, glm::ivec3 pos) {
        switch (biome) {
        case BiomeID::Plains:
        case BiomeID::Forest: return BlockID::Grass;
        case BiomeID::Desert: return BlockID::Sand;
        }
    }

    void biome_structures(int random_int, glm::ivec3 pos, const std::array<Block *, 16> &above, std::vector<Structure> &structures, Chunk &chunk) {
        switch (biome) {
        case BiomeID::Plains: {
            if (random_int < 40000) {
                above[0]->id = BlockID::TallGrass;
            } else if (random_int < 41000) {
                above[0]->id = BlockID::Rose;
            } else if (random_int < 41100) {
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
            } else if (random_int < 5000) {
                above[0]->id = BlockID::DriedShrub;
            }
        } break;
        }
    }
};
