#pragma once
#include <engine/world/renderable_chunk.hpp>

struct World {
    static constexpr int        RENDER_DIST_XZ = 8;
    static constexpr glm::ivec3 CHUNK_MAX{RENDER_DIST_XZ * 2, 4, RENDER_DIST_XZ * 2};

    std::array<std::array<std::array<RenderableChunk *, CHUNK_MAX.x>, CHUNK_MAX.y>, CHUNK_MAX.z> chunks;

    static constexpr glm::ivec3 chunk_min{-RENDER_DIST_XZ, -2, -RENDER_DIST_XZ};
    static constexpr glm::ivec3 chunk_max{RENDER_DIST_XZ, 2, RENDER_DIST_XZ};

    World() {
        for (int zi = chunk_min.z; zi < chunk_max.z; ++zi) {
            for (int yi = chunk_min.y; yi < chunk_max.y; ++yi) {
                for (int xi = chunk_min.x; xi < chunk_max.x; ++xi) {
                    auto & current_chunk_ptr = chunks[xi - chunk_min.x][yi - chunk_min.y][zi - chunk_min.z];
                    current_chunk_ptr        = new RenderableChunk({xi, yi, zi});
                }
            }
        }
        update_neighbors();

        for (int zi = chunk_min.z; zi < chunk_max.z; ++zi) {
            for (int yi = chunk_min.y; yi < chunk_max.y; ++yi) {
                for (int xi = chunk_min.x; xi < chunk_max.x; ++xi) {
                    auto & current_chunk_ptr = chunks[xi - chunk_min.x][yi - chunk_min.y][zi - chunk_min.z];
                    current_chunk_ptr->chunk.generate_block_data_pass2();
                }
            }
        }
    }

    ~World() {
        for (int zi = chunk_min.z; zi < chunk_max.z; ++zi) {
            for (int yi = chunk_min.y; yi < chunk_max.y; ++yi) {
                for (int xi = chunk_min.x; xi < chunk_max.x; ++xi) {
                    auto & current_chunk_ptr = chunks[xi - chunk_min.x][yi - chunk_min.y][zi - chunk_min.z];
                    delete current_chunk_ptr;
                }
            }
        }
    }

    void update_neighbors() {
        for (int zi = chunk_min.z; zi < chunk_max.z; ++zi) {
            for (int yi = chunk_min.y; yi < chunk_max.y; ++yi) {
                for (int xi = chunk_min.x; xi < chunk_max.x; ++xi) {
                    glm::ivec3 index(xi - chunk_min.x, yi - chunk_min.y, zi - chunk_min.z);

                    auto current_chunk_ptr = chunks[index.x][index.y][index.z];

                    if (xi != chunk_min.x) {
                        auto neighbor_chunk_ptr               = chunks[index.x - 1][index.y][index.z];
                        current_chunk_ptr->chunk.neighbors.nx = &(neighbor_chunk_ptr->chunk);
                    } else {
                        current_chunk_ptr->chunk.neighbors.nx = nullptr;
                    }
                    if (xi != chunk_max.x - 1) {
                        auto neighbor_chunk_ptr               = chunks[index.x + 1][index.y][index.z];
                        current_chunk_ptr->chunk.neighbors.px = &(neighbor_chunk_ptr->chunk);
                    } else {
                        current_chunk_ptr->chunk.neighbors.px = nullptr;
                    }

                    if (yi != chunk_min.y) {
                        auto neighbor_chunk_ptr               = chunks[index.x][index.y - 1][index.z];
                        current_chunk_ptr->chunk.neighbors.ny = &(neighbor_chunk_ptr->chunk);
                    } else {
                        current_chunk_ptr->chunk.neighbors.ny = nullptr;
                    }
                    if (yi != chunk_max.y - 1) {
                        auto neighbor_chunk_ptr               = chunks[index.x][index.y + 1][index.z];
                        current_chunk_ptr->chunk.neighbors.py = &(neighbor_chunk_ptr->chunk);
                    } else {
                        current_chunk_ptr->chunk.neighbors.py = nullptr;
                    }

                    if (zi != chunk_min.z) {
                        auto neighbor_chunk_ptr               = chunks[index.x][index.y][index.z - 1];
                        current_chunk_ptr->chunk.neighbors.nz = &(neighbor_chunk_ptr->chunk);
                    } else {
                        current_chunk_ptr->chunk.neighbors.nz = nullptr;
                    }
                    if (zi != chunk_max.z - 1) {
                        auto neighbor_chunk_ptr               = chunks[index.x][index.y][index.z + 1];
                        current_chunk_ptr->chunk.neighbors.pz = &(neighbor_chunk_ptr->chunk);
                    } else {
                        current_chunk_ptr->chunk.neighbors.pz = nullptr;
                    }
                }
            }
        }
    }

    void draw(const glm::mat4 & viewproj_mat) {
        for (int zi = chunk_min.z; zi < chunk_max.z; ++zi) {
            for (int yi = chunk_min.y; yi < chunk_max.y; ++yi) {
                for (int xi = chunk_min.x; xi < chunk_max.x; ++xi) {
                    auto & current_chunk_ptr = chunks[xi - chunk_min.x][yi - chunk_min.y][zi - chunk_min.z];
                    current_chunk_ptr->draw(viewproj_mat);
                }
            }
        }
    }
};
