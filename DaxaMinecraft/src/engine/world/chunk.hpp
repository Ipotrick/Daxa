#pragma once

#include <engine/world/vertex.hpp>
#include <engine/world/block.hpp>
#include <engine/world/noise.hpp>

#include <cstdint>
#include <vector>

struct Chunk {
    static constexpr size_t    NX = 16, NY = 16, NZ = 16;
    static constexpr size_t    BLOCK_N   = NX * NY * NZ;
    static constexpr size_t    VERT_N    = BLOCK_N * 36;
    static constexpr size_t    MAX_SIZE  = VERT_N * sizeof(Vertex);
    static constexpr glm::vec3 FLOAT_DIM = glm::vec3(NX, NY, NZ);

    static inline Block null_block{.id = BlockID::Air};

    using BlockBuffer = std::array<std::array<std::array<Block, NX>, NX>, NX>;

    struct NeighborChunks {
        Chunk *nx, *ny, *nz;
        Chunk *px, *py, *pz;
    };
    NeighborChunks neighbors;

    BlockBuffer blocks;
    glm::ivec3  pos;
    bool        needs_remesh = true;
    std::size_t vert_n;

    auto get_block(int x, int y, int z) -> Block & {
        if (x < 0) {
            if (!neighbors.nx) return null_block;
            return neighbors.nx->blocks[x + NX][y][z];
        } else if (x > NX - 1) {
            if (!neighbors.px) return null_block;
            return neighbors.px->blocks[x - NX][y][z];
        }
        if (y < 0) {
            if (!neighbors.ny) return null_block;
            return neighbors.ny->blocks[x][y + NY][z];
        } else if (y > NY - 1) {
            if (!neighbors.py) return null_block;
            return neighbors.py->blocks[x][y - NY][z];
        }
        if (z < 0) {
            if (!neighbors.nz) return null_block;
            return neighbors.nz->blocks[x][y][z + NZ];
        } else if (z > NZ - 1) {
            if (!neighbors.pz) return null_block;
            return neighbors.pz->blocks[x][y][z - NZ];
        }
        return blocks[x][y][z];
    }

    void generate_block_data() {
        for (int zi = 0; zi < NZ; ++zi) {
            for (int yi = 0; yi < NY; ++yi) {
                for (int xi = 0; xi < NX; ++xi) {
                    auto & current_block = blocks[xi][yi][zi];
                    auto   b_pos     = glm::vec3(xi, yi, zi) + FLOAT_DIM * glm::vec3(pos);
                    current_block.id = BlockID::Dirt;
                    // float val = sin(b_pos.x / 50 + cos(b_pos.z / 8)) * 2 - 10;
                    // val += cos(b_pos.z / 50 + val * 4) * 1;
                    // val *= sin(b_pos.x / 500 * (val + 1) + val / 5) * 0.2;
                    float val = terrain_noise(b_pos);

                    if (val > 0.0f) {
                        current_block.id = BlockID::Dirt;
                        // } else if (b_pos.y < -1) {
                        //     current_block.id = BlockID::Log;
                    } else {
                        current_block.id = BlockID::Air;
                    }
                }
            }
        }
    }

    void generate_block_data_pass2() {
        for (int zi = 0; zi < NZ; ++zi) {
            for (int yi = 0; yi < NY; ++yi) {
                for (int xi = 0; xi < NX; ++xi) {
                    auto & current_block = get_block(xi, yi, zi);
                    auto & above         = get_block(xi, yi + 1, zi);
                    if (current_block.id == BlockID::Dirt && above.is_transparent()) {
                        current_block.id = BlockID::Grass;
                        auto random_int  = rand() % 100;
                        if (random_int < 10) {
                            above.id = BlockID::TallGrass;
                        } else if (random_int == 11) {
                            above.id = BlockID::Rose;
                        }
                        null_block.id = BlockID::Air;
                    }
                }
            }
        }
    }

    void generate_mesh_data(Vertex * vertex_buffer_ptr) {
        auto fix_uv = [](const auto & block, BlockFace face, glm::vec2 & tex) {
            tex.y = 1.0f - tex.y;
            tex += block.texture_face_offset(face);
            tex *= 0.25f;
            tex.y = 1.0f - tex.y;
        };

        auto * vertex_buffer_head = vertex_buffer_ptr;
        auto   push_back          = [&vertex_buffer_head](const auto & v) {
            *vertex_buffer_head = v;
            ++vertex_buffer_head;
        };

        for (int zi = 0; zi < NZ; ++zi) {
            for (int yi = 0; yi < NY; ++yi) {
                for (int xi = 0; xi < NX; ++xi) {
                    auto   inchunk_tile_offset = glm::vec3(xi, yi, zi);
                    float  x = xi, y = yi, z = zi;
                    auto & current_block = blocks[x][y][z];
                    if (current_block.is_not_drawn()) continue;
                    if (current_block.is_cube()) {
                        if (get_block(x, y, z - 1).is_transparent()) {
                            for (auto v : back_vertices) {
                                v.pos += inchunk_tile_offset;
                                v.nrm = glm::vec3(0, 0, -1);
                                fix_uv(current_block, BlockFace::Back, v.tex);
                                push_back(v);
                            }
                        }
                        if (get_block(x, y, z + 1).is_transparent()) {
                            for (auto v : front_vertices) {
                                v.pos += inchunk_tile_offset;
                                v.nrm = glm::vec3(0, 0, 1);
                                fix_uv(current_block, BlockFace::Front, v.tex);
                                push_back(v);
                            }
                        }

                        if (get_block(x - 1, y, z).is_transparent()) {
                            for (auto v : left_vertices) {
                                v.pos += inchunk_tile_offset;
                                v.nrm = glm::vec3(-1, 0, 0);
                                fix_uv(current_block, BlockFace::Left, v.tex);
                                push_back(v);
                            }
                        }
                        if (get_block(x + 1, y, z).is_transparent()) {
                            for (auto v : right_vertices) {
                                v.pos += inchunk_tile_offset;
                                v.nrm = glm::vec3(1, 0, 0);
                                fix_uv(current_block, BlockFace::Right, v.tex);
                                push_back(v);
                            }
                        }

                        if (get_block(x, y - 1, z).is_transparent()) {
                            for (auto v : bottom_vertices) {
                                v.pos += inchunk_tile_offset;
                                v.nrm = glm::vec3(0, -1, 0);
                                fix_uv(current_block, BlockFace::Bottom, v.tex);
                                push_back(v);
                            }
                        }
                        if (get_block(x, y + 1, z).is_transparent()) {
                            for (auto v : top_vertices) {
                                v.pos += inchunk_tile_offset;
                                v.nrm = glm::vec3(0, 1, 0);
                                fix_uv(current_block, BlockFace::Top, v.tex);
                                push_back(v);
                            }
                        }
                    } else {
                        for (auto v : cross_vertices) {
                            v.pos += inchunk_tile_offset;
                            v.nrm = glm::vec3(0, 1, 0);
                            fix_uv(current_block, BlockFace::Back, v.tex);
                            push_back(v);
                        }
                    }
                }
            }
        }

        vert_n       = vertex_buffer_head - vertex_buffer_ptr;
        needs_remesh = false;
    }

    Block * get_containing_block(glm::vec3 p) {
        if (p.x >= pos.x && p.x < pos.x + FLOAT_DIM.x && //
            p.y >= pos.y && p.y < pos.y + FLOAT_DIM.y && //
            p.z >= pos.z && p.z < pos.z + FLOAT_DIM.z) {
            glm::ivec3 p_index = glm::ivec3(p - glm::vec3(pos) * FLOAT_DIM);
            return &blocks[p_index.x][p_index.y][p_index.z];
        }
        return nullptr;
    }
};
