#pragma once

#include <engine/world/vertex.hpp>
#include <engine/world/block.hpp>
#include <engine/world/noise.hpp>

#include <cstdint>
#include <vector>
#include <random>

struct Chunk {
    static constexpr size_t NX = 16, NY = 16, NZ = 16;
    static constexpr size_t BLOCK_N = NX * NY * NZ;
    static constexpr size_t VERT_N = BLOCK_N * 36;
    static constexpr size_t MAX_SIZE = VERT_N * sizeof(Vertex);
    static constexpr glm::vec3 FLOAT_DIM = glm::vec3(NX, NY, NZ);
    static constexpr glm::ivec3 INT_DIM = glm::ivec3(NX, NY, NZ);

    static inline Block null_block{.id = BlockID::Air};
    static inline TileInfo null_tile{};

    template <typename T>
    using Buffer = std::array<std::array<std::array<T, NX>, NY>, NZ>;
    using BlockBuffer = Buffer<Block>;
    using TileBuffer = Buffer<TileInfo>;

    using NeighborChunks = std::array<Chunk *, 6>;
    NeighborChunks neighbors;

    BlockBuffer blocks;
    TileBuffer tiles;
    glm::ivec3 pos;
    bool needs_remesh = true;
    std::size_t vert_n;

#define SAMPLE_BUFFER(buf, nul)                 \
    if (x < 0) {                                \
        if (!neighbors[0])                      \
            return nul;                         \
        return neighbors[0]->buf[z][y][x + NX]; \
    } else if (x > NX - 1) {                    \
        if (!neighbors[1])                      \
            return nul;                         \
        return neighbors[1]->buf[z][y][x - NX]; \
    }                                           \
    if (y < 0) {                                \
        if (!neighbors[2])                      \
            return nul;                         \
        return neighbors[2]->buf[z][y + NY][x]; \
    } else if (y > NY - 1) {                    \
        if (!neighbors[3])                      \
            return nul;                         \
        return neighbors[3]->buf[z][y - NY][x]; \
    }                                           \
    if (z < 0) {                                \
        if (!neighbors[4])                      \
            return nul;                         \
        return neighbors[4]->buf[z + NZ][y][x]; \
    } else if (z > NZ - 1) {                    \
        if (!neighbors[5])                      \
            return nul;                         \
        return neighbors[5]->buf[z - NZ][y][x]; \
    }                                           \
    return buf[z][y][x]

    auto get_block(int x, int y, int z) -> Block & { SAMPLE_BUFFER(blocks, null_block); }
    auto get_tile(int x, int y, int z) -> TileInfo & { SAMPLE_BUFFER(tiles, null_tile); }
#undef SAMPLE_BUFFER

    void copy_block_data(const Chunk::BlockBuffer &src_blocks) {
        for (int zi = 0; zi < NZ; ++zi) {
            for (int yi = 0; yi < NY; ++yi) {
                for (int xi = 0; xi < NX; ++xi) {
                    auto &current_block = blocks[zi][yi][xi];
                    auto &current_src_block = src_blocks[zi][yi][xi];

                    auto &current_tile = tiles[zi][yi][xi];
                    auto b_pos = glm::vec3(xi, yi, zi) + FLOAT_DIM * glm::vec3(pos);

                    current_block = current_src_block;

                    float bval = biome_noise(glm::ivec3(b_pos.x, 0, b_pos.z));
                    if (bval > 0.1f) {
                        current_tile.biome = BiomeID::Desert;
                    } else if (bval > 0.02f) {
                        current_tile.biome = BiomeID::Plains;
                    } else {
                        current_tile.biome = BiomeID::Forest;
                    }
                }
            }
        }
    }

    void generate_block_data() {
        for (int zi = 0; zi < NZ; ++zi) {
            for (int yi = 0; yi < NY; ++yi) {
                for (int xi = 0; xi < NX; ++xi) {
                    auto &current_block = blocks[zi][yi][xi];
                    auto &current_tile = tiles[zi][yi][xi];
                    auto b_pos = glm::vec3(xi, yi, zi) + FLOAT_DIM * glm::vec3(pos);
                    current_block.id = BlockID::Dirt;
                    // float val = sin(b_pos.x / 50 + cos(b_pos.z / 8)) * 2 - 10;
                    // val += cos(b_pos.z / 50 + val * 4) * 1;
                    // val *= sin(b_pos.x / 500 * (val + 1) + val / 5) * 0.2;
                    float val = terrain_noise(b_pos);
                    float bval = biome_noise(glm::ivec3(b_pos.x, 0, b_pos.z));

                    if (bval > 0.1f) {
                        current_tile.biome = BiomeID::Desert;
                    } else if (bval > 0.02f) {
                        current_tile.biome = BiomeID::Plains;
                    } else {
                        current_tile.biome = BiomeID::Forest;
                    }

                    if (val > 0.0f) {
                        current_block.id = BlockID::Stone;
                    } else {
                        if (b_pos.y < 0) {
                            current_block.id = BlockID::Water;
                        } else {
                            current_block.id = BlockID::Air;
                        }
                    }
                }
            }
        }
    }

    void generate_block_data_pass2(std::vector<Structure> &structures) {
        std::default_random_engine rng;
        std::uniform_int_distribution dist(0, 100'000);

        for (int zi = 0; zi < NZ; ++zi) {
            for (int yi = 0; yi < NY; ++yi) {
                for (int xi = 0; xi < NX; ++xi) {
                    auto b_pos = glm::vec3(xi, yi, zi) + FLOAT_DIM * glm::vec3(pos);
                    auto &current_block = get_block(xi, yi, zi);
                    auto &current_tile = get_tile(xi, yi, zi);
                    std::array<Block *, 16> above{};
                    for (size_t i = 0; i < above.size(); ++i)
                        above[i] = &get_block(xi, yi + 1 + i, zi);

                    if (current_block.id == BlockID::Stone) {
                        auto random_int = dist(rng);
                        if (b_pos.y < -20 && b_pos.y > -60) {
                            auto c_val = cave_noise(b_pos) + (b_pos.y + 44) * 0.01f;
                            if (c_val < -0.01) {
                                current_block.id = BlockID::Air;
                            }
                        } else {
                            // add surface decorations
                            if (above[0]->is_transparent()) {
                                if (b_pos.y < 2 && b_pos.y > -2) { // beach
                                    current_block.id = BlockID::Sand;
                                } else {
                                    current_block.id = current_tile.biome_surface(random_int, glm::ivec3(xi, yi, zi));
                                    current_tile.biome_structures(random_int, glm::ivec3(xi, yi, zi), above, structures, *this);
                                }
                            } else {
                                if (b_pos.y < 2 && b_pos.y > -2) { // beach
                                    current_block.id = BlockID::Sand;
                                } else {
                                    current_tile.biome_ground(random_int, glm::ivec3(xi, yi, zi), above, current_block.id);
                                }
                            }
                            null_block.id = BlockID::Air;
                        }
                    }
                }
            }
        }
    }

    void generate_block_data_structures(const std::vector<Structure> &structures) {
        for (auto &structure : structures) {
            StructureTypes s;
            switch (structure.id) {
            case StructureID::Tree: {
                s = Tree{};
            } break;
            case StructureID::VillageSpawn: {
                s = Village{};
            } break;
            default: {
                s = InvalidStructure{};
            } break;
            }
            std::visit(
                [&](const auto &s) {
                    using SType = std::decay_t<decltype(s)>;
                    glm::ivec3 min = structure.pos + SType::MIN;
                    min += (structure.home_chunk->pos - pos) * INT_DIM;
                    glm::ivec3 max = structure.pos + SType::MAX;
                    max += (structure.home_chunk->pos - pos) * INT_DIM;
                    glm::ivec3 offset = min;
                    min = {std::max(0, min.x), std::max(0, min.y), std::max(0, min.z)};
                    max = {std::min(INT_DIM.x - 1, max.x), std::min(INT_DIM.y - 1, max.y),
                           std::min(INT_DIM.z - 1, max.z)};
                    offset = min - offset;
                    for (int zi = min.z; zi <= max.z; ++zi) {
                        for (int yi = min.y; yi <= max.y; ++yi) {
                            for (int xi = min.x; xi <= max.x; ++xi) {
                                auto b_pos = glm::ivec3(xi, yi, zi) + INT_DIM * pos;
                                auto &current_block = get_block(xi, yi, zi);
                                auto structure_block_pos =
                                    b_pos -
                                    (structure.pos + structure.home_chunk->pos * INT_DIM);
                                SType::transform_tile(structure_block_pos,
                                                      current_block.id);
                            }
                        }
                    }
                },
                s);
        }
    }

    void generate_mesh_data(Vertex *vertex_buffer_ptr) {
        auto fix_uv = [](const auto &block, BlockFace face, int &tex_id) {
            // tex.y = 1.0f - tex.y;
            // tex += block.texture_face_offset(face);
            // tex *= 0.125f;
            // tex.y = 1.0f - tex.y;
            tex_id = block.texture_index(face);
        };

        auto *vertex_buffer_head = vertex_buffer_ptr;
        auto push_back = [&vertex_buffer_head](const auto &v) {
            *vertex_buffer_head = v;
            ++vertex_buffer_head;
        };

        for (int zi = 0; zi < NZ; ++zi) {
            for (int yi = 0; yi < NY; ++yi) {
                for (int xi = 0; xi < NX; ++xi) {
                    auto inchunk_tile_offset = glm::vec3(xi, yi, zi);
                    float x = xi, y = yi, z = zi;
                    auto &current_block = blocks[z][y][x];
                    if (current_block.is_not_drawn())
                        continue;
                    if (current_block.is_cube()) {
                        if (get_block(x, y, z - 1).is_transparent()) {
                            for (auto v : back_vertices) {
                                v.pos += inchunk_tile_offset;
                                v.nrm = glm::vec3(0, 0, -1);
                                fix_uv(current_block, BlockFace::Back, v.tex_id);
                                push_back(v);
                            }
                        }
                        if (get_block(x, y, z + 1).is_transparent()) {
                            for (auto v : front_vertices) {
                                v.pos += inchunk_tile_offset;
                                v.nrm = glm::vec3(0, 0, 1);
                                fix_uv(current_block, BlockFace::Front, v.tex_id);
                                push_back(v);
                            }
                        }

                        if (get_block(x - 1, y, z).is_transparent()) {
                            for (auto v : left_vertices) {
                                v.pos += inchunk_tile_offset;
                                v.nrm = glm::vec3(-1, 0, 0);
                                fix_uv(current_block, BlockFace::Left, v.tex_id);
                                push_back(v);
                            }
                        }
                        if (get_block(x + 1, y, z).is_transparent()) {
                            for (auto v : right_vertices) {
                                v.pos += inchunk_tile_offset;
                                v.nrm = glm::vec3(1, 0, 0);
                                fix_uv(current_block, BlockFace::Right, v.tex_id);
                                push_back(v);
                            }
                        }

                        if (get_block(x, y - 1, z).is_transparent()) {
                            for (auto v : bottom_vertices) {
                                v.pos += inchunk_tile_offset;
                                v.nrm = glm::vec3(0, -1, 0);
                                fix_uv(current_block, BlockFace::Bottom, v.tex_id);
                                push_back(v);
                            }
                        }
                        if (get_block(x, y + 1, z).is_transparent()) {
                            for (auto v : top_vertices) {
                                v.pos += inchunk_tile_offset;
                                v.nrm = glm::vec3(0, 1, 0);
                                fix_uv(current_block, BlockFace::Top, v.tex_id);
                                push_back(v);
                            }
                        }
                    } else {
                        for (auto v : cross_vertices) {
                            v.pos += inchunk_tile_offset;
                            v.nrm = glm::vec3(0, 1, 0);
                            fix_uv(current_block, BlockFace::Back, v.tex_id);
                            push_back(v);
                        }
                    }
                }
            }
        }

        vert_n = vertex_buffer_head - vertex_buffer_ptr;
        needs_remesh = false;
    }

    Block *get_containing_block(glm::vec3 p) {
        if (p.x >= pos.x && p.x < pos.x + FLOAT_DIM.x && //
            p.y >= pos.y && p.y < pos.y + FLOAT_DIM.y && //
            p.z >= pos.z && p.z < pos.z + FLOAT_DIM.z) {
            glm::ivec3 p_index = glm::ivec3(p - glm::vec3(pos) * FLOAT_DIM);
            return &blocks[p_index.z][p_index.y][p_index.x];
        }
        return nullptr;
    }
};
