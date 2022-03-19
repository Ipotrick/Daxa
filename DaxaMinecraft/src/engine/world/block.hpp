#pragma once
#include <glm/glm.hpp>

enum class BlockID : uint32_t
{
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
};

enum class BlockFace : uint32_t
{
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
        default: return glm::vec2{0, 0};
        }
    }
};
