#pragma once

#include <daxa/types.hpp>
using namespace daxa::types;

#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/math_operators.hpp>

#include <voxels.inl>

#include <stb_image.h>
#include <thread>
#include <mutex>
#include <memory>

static inline constexpr usize CHUNK_SIZE = 32;
static inline constexpr usize CHUNK_VOXEL_N = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
static inline constexpr usize WORLD_SIZE = 4;
static inline constexpr usize CHUNK_N = WORLD_SIZE * WORLD_SIZE * WORLD_SIZE;
static inline constexpr usize CHUNK_MAX_VERTS = CHUNK_VOXEL_N * 6;
static const std::filesystem::path textures_filepath = "tests/0_common/assets/textures";
static const std::array<std::filesystem::path, 32> texture_names{
    "debug.png",
    "air.png",
    "bedrock.png",
    "brick.png",
    "cactus.png",
    "cobblestone.png",
    "compressed_stone.png",
    "diamond_ore.png",
    "dirt.png",
    "dried_shrub.png",
    "grass-side.png",
    "grass-top.png",
    "gravel.png",
    "lava_0.png",
    "lava_1.png",
    "lava_2.png",
    "lava_3.png",
    "lava_4.png",
    "lava_5.png",
    "lava_6.png",
    "lava_7.png",
    "leaves.png",
    "bark.png",
    "log-top.png",
    "molten_rock.png",
    "planks.png",
    "rose.png",
    "sand.png",
    "sandstone.png",
    "stone.png",
    "tallgrass.png",
    "water.png",
};

using BlockID = u32;

struct VoxelFace
{
    u32 data = 0;

    VoxelFace(i32 x, i32 y, i32 z, u32 side, BlockID id)
    {
        data |= (static_cast<u32>(x) & 0x1f) << 0;
        data |= (static_cast<u32>(y) & 0x1f) << 5;
        data |= (static_cast<u32>(z) & 0x1f) << 10;
        data |= (side & 0x7) << 15;
        data |= (id & 0x3fff) << 18;
    }
};

struct Voxel
{
    BlockID id;

    [[nodiscard]] auto is_occluding() const -> bool
    {
        switch (id)
        {
        case BlockID_Air:
        case BlockID_Rose:
        case BlockID_TallGrass:
        case BlockID_Water: return false;
        default: return true;
        }
    }
    [[nodiscard]] auto is_cross() const -> bool
    {
        switch (id)
        {
        case BlockID_Rose:
        case BlockID_TallGrass: return true;
        default: return false;
        }
    }

    [[nodiscard]] auto is_occluding_nx() const -> bool { return is_occluding(); }
    [[nodiscard]] auto is_occluding_px() const -> bool { return is_occluding(); }
    [[nodiscard]] auto is_occluding_ny() const -> bool { return is_occluding(); }
    [[nodiscard]] auto is_occluding_py() const -> bool { return is_occluding(); }
    [[nodiscard]] auto is_occluding_nz() const -> bool { return is_occluding(); }
    [[nodiscard]] auto is_occluding_pz() const -> bool { return is_occluding(); }
};

struct VoxelChunk
{
    Voxel voxels[CHUNK_VOXEL_N];
    f32vec3 pos;
    bool is_generated = false;

    static auto generate_block_id(f32vec3 p) -> BlockID
    {
        f32 val = std::sin(p.x * 0.05f) * 20.0f + std::sin(p.z * 0.06f) * 16.0f + 40.0f - p.y;
        if (val < 0.0f)
        {
            return static_cast<BlockID>(rand() % BlockID_Water);
        }
        else
        {
            return static_cast<BlockID>(BlockID_Air);
        }
    }
    void generate()
    {
        using namespace daxa::math_operators;
        for (u64 zi = 0; zi < CHUNK_SIZE; ++zi)
        {
            for (u64 yi = 0; yi < CHUNK_SIZE; ++yi)
            {
                for (u64 xi = 0; xi < CHUNK_SIZE; ++xi)
                {
                    u64 const i = xi + yi * CHUNK_SIZE + zi * CHUNK_SIZE * CHUNK_SIZE;
                    f32vec3 const block_pos = f32vec3{static_cast<f32>(xi), static_cast<f32>(yi), static_cast<f32>(zi)} + pos;
                    voxels[i].id = generate_block_id(block_pos);
                }
            }
        }
        is_generated = true;
    }
};
struct RenderableChunk
{
    std::unique_ptr<VoxelChunk> chunk = std::make_unique<VoxelChunk>();
    daxa::BufferId face_buffer;
    daxa::TaskBufferHandle task_face_buffer;
    std::mutex update_mtx{};
    // daxa::BufferId water_face_buffer;
    // daxa::TaskBufferId task_water_face_buffer;
    u32 face_n = 0;
    // u32 water_face_n = 0;

    bool mesh_invalid = true;

    void construct_mesh(VoxelFace *& buffer_ptr, auto get_voxel)
    {
        using namespace daxa::math_operators;

        auto & chunk_pos = chunk->pos;
        face_n = 0;
        for (i32 zi = 0; zi < static_cast<i32>(CHUNK_SIZE); ++zi)
        {
            for (i32 yi = 0; yi < static_cast<i32>(CHUNK_SIZE); ++yi)
            {
                for (i32 xi = 0; xi < static_cast<i32>(CHUNK_SIZE); ++xi)
                {
                    i32vec3 const voxel_pos = i32vec3{xi, yi, zi} + i32vec3{static_cast<i32>(chunk_pos.x), static_cast<i32>(chunk_pos.y), static_cast<i32>(chunk_pos.z)};
                    Voxel const current_voxel = get_voxel(voxel_pos);
                    if (current_voxel.is_occluding())
                    {
                        if (!get_voxel(voxel_pos + i32vec3{+1, 0, 0}).is_occluding_nx())
                        {
                            *buffer_ptr = VoxelFace(xi, yi, zi, 0, current_voxel.id), ++buffer_ptr, ++face_n;
                        }
                        if (!get_voxel(voxel_pos + i32vec3{-1, 0, 0}).is_occluding_px())
                        {
                            *buffer_ptr = VoxelFace(xi, yi, zi, 1, current_voxel.id), ++buffer_ptr, ++face_n;
                        }
                        if (!get_voxel(voxel_pos + i32vec3{0, +1, 0}).is_occluding_ny())
                        {
                            *buffer_ptr = VoxelFace(xi, yi, zi, 2, current_voxel.id), ++buffer_ptr, ++face_n;
                        }
                        if (!get_voxel(voxel_pos + i32vec3{0, -1, 0}).is_occluding_py())
                        {
                            *buffer_ptr = VoxelFace(xi, yi, zi, 3, current_voxel.id), ++buffer_ptr, ++face_n;
                        }
                        if (!get_voxel(voxel_pos + i32vec3{0, 0, +1}).is_occluding_nz())
                        {
                            *buffer_ptr = VoxelFace(xi, yi, zi, 4, current_voxel.id), ++buffer_ptr, ++face_n;
                        }
                        if (!get_voxel(voxel_pos + i32vec3{0, 0, -1}).is_occluding_pz())
                        {
                            *buffer_ptr = VoxelFace(xi, yi, zi, 5, current_voxel.id), ++buffer_ptr, ++face_n;
                        }
                    }
                    else if (current_voxel.is_cross())
                    {
                        *buffer_ptr = VoxelFace(xi, yi, zi, 6, current_voxel.id), ++buffer_ptr, ++face_n;
                        *buffer_ptr = VoxelFace(xi, yi, zi, 6, current_voxel.id), ++buffer_ptr, ++face_n;
                        *buffer_ptr = VoxelFace(xi, yi, zi, 7, current_voxel.id), ++buffer_ptr, ++face_n;
                        *buffer_ptr = VoxelFace(xi, yi, zi, 7, current_voxel.id), ++buffer_ptr, ++face_n;
                    }
                }
            }
        }
        mesh_invalid = false;
    }
};

struct RenderableWorld
{
    std::array<RenderableChunk, CHUNK_N> renderable_chunks;

    void generate_chunks()
    {
        for (i32 zi = 0; zi < static_cast<i32>(WORLD_SIZE); ++zi)
        {
            for (i32 yi = 0; yi < static_cast<i32>(WORLD_SIZE); ++yi)
            {
                for (i32 xi = 0; xi < static_cast<i32>(WORLD_SIZE); ++xi)
                {
                    auto const chunk_index = static_cast<usize>(xi) + static_cast<usize>(yi) * WORLD_SIZE + static_cast<usize>(zi) * WORLD_SIZE * WORLD_SIZE;
                    auto & renderable_chunk = renderable_chunks[chunk_index];
                    renderable_chunk.chunk->pos = {static_cast<f32>(xi) * CHUNK_SIZE, static_cast<f32>(yi) * CHUNK_SIZE, static_cast<f32>(zi) * CHUNK_SIZE};
                    // renderable_chunk.chunk->generate();
                }
            }
        }
    }

    auto get_voxel(i32vec3 p) -> Voxel
    {
        i32 const x = p.x / static_cast<i32>(CHUNK_SIZE);
        i32 const y = p.y / static_cast<i32>(CHUNK_SIZE);
        i32 const z = p.z / static_cast<i32>(CHUNK_SIZE);

        if (p.x < 0 || x >= static_cast<i32>(WORLD_SIZE))
        {
            return {.id = BlockID_Air};
        }
        if (p.y < 0 || y >= static_cast<i32>(WORLD_SIZE))
        {
            return {.id = BlockID_Air};
        }
        if (p.z < 0 || z >= static_cast<i32>(WORLD_SIZE))
        {
            return {.id = BlockID_Air};
        }

        auto & renderable_chunk = renderable_chunks[static_cast<u64>(x) + static_cast<u64>(y) * WORLD_SIZE + static_cast<u64>(z) * WORLD_SIZE * WORLD_SIZE];

        if (!renderable_chunk.chunk->is_generated)
        {
            return {.id = BlockID_Air};
        }

        u64 const xi = static_cast<u64>(p.x) % CHUNK_SIZE;
        u64 const yi = static_cast<u64>(p.y) % CHUNK_SIZE;
        u64 const zi = static_cast<u64>(p.z) % CHUNK_SIZE;
        u64 const i = xi + yi * CHUNK_SIZE + zi * CHUNK_SIZE * CHUNK_SIZE;
        return renderable_chunk.chunk->voxels[i];
    }

    void chunk_update(usize index)
    {
        auto chunk_i = i32vec3{0, 0, 0};
        chunk_i.x = static_cast<i32>(index % WORLD_SIZE);
        chunk_i.y = static_cast<i32>((index / WORLD_SIZE) % WORLD_SIZE);
        chunk_i.z = static_cast<i32>((index / WORLD_SIZE / WORLD_SIZE) % WORLD_SIZE);

        auto update_chunk = [this](i32vec3 i)
        {
            if (i.x >= 0 && i.x < static_cast<i32>(WORLD_SIZE) &&
                i.y >= 0 && i.y < static_cast<i32>(WORLD_SIZE) &&
                i.z >= 0 && i.z < static_cast<i32>(WORLD_SIZE))
            {
                auto const chunk_index = static_cast<usize>(i.x) + static_cast<usize>(i.y) * WORLD_SIZE + static_cast<usize>(i.z) * WORLD_SIZE * WORLD_SIZE;
                auto lock = std::lock_guard{renderable_chunks[chunk_index].update_mtx};
                renderable_chunks[chunk_index].mesh_invalid = true;
            }
        };

        update_chunk(i32vec3{chunk_i.x - 1, chunk_i.y, chunk_i.z});
        update_chunk(i32vec3{chunk_i.x + 1, chunk_i.y, chunk_i.z});
        update_chunk(i32vec3{chunk_i.x, chunk_i.y - 1, chunk_i.z});
        update_chunk(i32vec3{chunk_i.x, chunk_i.y + 1, chunk_i.z});
        update_chunk(i32vec3{chunk_i.x, chunk_i.y, chunk_i.z - 1});
        update_chunk(i32vec3{chunk_i.x, chunk_i.y, chunk_i.z + 1});
    }
};

void load_textures_commands(daxa::Device & device, daxa::CommandList & cmd_list, daxa::ImageId atlas_texture_array)
{
    usize const image_size = 16 * 16 * sizeof(u8) * 4;
    auto texture_staging_buffer = device.create_buffer({
        .size = static_cast<u32>(image_size * texture_names.size()),
        .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
        .name = "texture_staging_buffer",
    });
    cmd_list.destroy_buffer_deferred(texture_staging_buffer);
    u8 * staging_buffer_data = device.get_host_address_as<u8>(texture_staging_buffer);
    for (usize i = 0; i < texture_names.size(); ++i)
    {
        stbi_set_flip_vertically_on_load(1);
        auto path = textures_filepath / texture_names[i];
        i32 size_x = 0;
        i32 size_y = 0;
        i32 num_channels = 0;
        u8 * data = stbi_load(path.string().c_str(), &size_x, &size_y, &num_channels, 4);
        if (data == nullptr)
        {
            // std::cout << "could not find file: \"" << path << "\"" << std::endl;
            continue;
        }
        usize const offset = i * 16 * 16 * 4;
        for (usize j = 0; j < 16 * 16; ++j)
        {
            usize const data_i = j * 4;
            staging_buffer_data[offset + data_i + 0] = data[data_i + 0];
            staging_buffer_data[offset + data_i + 1] = data[data_i + 1];
            staging_buffer_data[offset + data_i + 2] = data[data_i + 2];
            staging_buffer_data[offset + data_i + 3] = data[data_i + 3];
        }
        cmd_list.copy_buffer_to_image({
            .buffer = texture_staging_buffer,
            .buffer_offset = i * image_size,
            .image = atlas_texture_array,
            .image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .image_slice = {
                .mip_level = 0,
                .base_array_layer = static_cast<u32>(i),
                .layer_count = 1,
            },
            .image_offset = {0, 0, 0},
            .image_extent = {16, 16, 1},
        });
    }
}
