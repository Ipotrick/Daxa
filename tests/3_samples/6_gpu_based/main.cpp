#include <0_common/window.hpp>
#include <0_common/player.hpp>
#include <thread>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "util.hpp"

using namespace daxa::types;
using Clock = std::chrono::high_resolution_clock;

#include "config.inl"

struct MipMapGenInfo
{
    daxa::ImageId image;
    u32 base_array_layer = 0;
    u32 layer_count = 1;
};

void generate_mip_levels(daxa::Device & device, daxa::CommandList & cmd_list, MipMapGenInfo const & info)
{
    auto image_info = device.info_image(info.image);
    std::array<i32, 3> mip_size = {
        static_cast<i32>(image_info.size[0]),
        static_cast<i32>(image_info.size[1]),
        static_cast<i32>(image_info.size[2]),
    };
    for (u32 i = 0; i < image_info.mip_level_count - 1; ++i)
    {
        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::BLIT_READ,
            .before_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .after_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
            .image_id = info.image,
            .image_slice = {
                .image_aspect = image_info.aspect,
                .base_mip_level = i,
                .level_count = 1,
                .base_array_layer = info.base_array_layer,
                .layer_count = info.layer_count,
            },
        });
        cmd_list.pipeline_barrier_image_transition({
            .waiting_pipeline_access = daxa::AccessConsts::BLIT_READ,
            .after_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .image_id = info.image,
            .image_slice = {
                .image_aspect = image_info.aspect,
                .base_mip_level = i + 1,
                .level_count = 1,
                .base_array_layer = info.base_array_layer,
                .layer_count = info.layer_count,
            },
        });
        std::array<i32, 3> next_mip_size = {
            std::max<i32>(1, mip_size[0] / 2),
            std::max<i32>(1, mip_size[1] / 2),
            std::max<i32>(1, mip_size[2] / 2),
        };
        cmd_list.blit_image_to_image({
            .src_image = info.image,
            .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
            .dst_image = info.image,
            .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .src_slice = {
                .image_aspect = image_info.aspect,
                .mip_level = i,
                .base_array_layer = info.base_array_layer,
                .layer_count = info.layer_count,
            },
            .src_offsets = {{{0, 0, 0}, {mip_size[0], mip_size[1], mip_size[2]}}},
            .dst_slice = {
                .image_aspect = image_info.aspect,
                .mip_level = i + 1,
                .base_array_layer = info.base_array_layer,
                .layer_count = info.layer_count,
            },
            .dst_offsets = {{{0, 0, 0}, {next_mip_size[0], next_mip_size[1], next_mip_size[2]}}},
            .filter = daxa::Filter::LINEAR,
        });
        mip_size = next_mip_size;
    }
    for (u32 i = 0; i < image_info.mip_level_count - 1; ++i)
    {
        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_READ_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::READ_WRITE,
            .before_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
            .after_layout = daxa::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
            .image_id = info.image,
            .image_slice = {
                .image_aspect = image_info.aspect,
                .base_mip_level = i,
                .level_count = 1,
                .base_array_layer = info.base_array_layer,
                .layer_count = info.layer_count,
            },
        });
    }
    cmd_list.pipeline_barrier_image_transition({
        .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_READ_WRITE,
        .waiting_pipeline_access = daxa::AccessConsts::READ_WRITE,
        .before_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
        .after_layout = daxa::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
        .image_id = info.image,
        .image_slice = {
            .image_aspect = image_info.aspect,
            .base_mip_level = image_info.mip_level_count - 1,
            .level_count = 1,
            .base_array_layer = info.base_array_layer,
            .layer_count = info.layer_count,
        },
    });
}

enum class BlockID : u32
{
    Debug,
    Air,
    Bedrock,
    Brick,
    Cactus,
    Cobblestone,
    CompressedStone,
    DiamondOre,
    Dirt,
    DriedShrub,
    Grass,
    Gravel,
    Lava,
    Leaves,
    Log,
    MoltenRock,
    Planks,
    Rose,
    Sand,
    Sandstone,
    Stone,
    TallGrass,
    Water,
};

struct Vertex
{
    u32 data;
    Vertex(i32 x, i32 y, i32 z, u32 side, BlockID id)
    {
        data = 0;
        data |= (static_cast<u32>(x) & 0x3f) << 0;
        data |= (static_cast<u32>(y) & 0x3f) << 6;
        data |= (static_cast<u32>(z) & 0x3f) << 12;
        data |= (side & 0x7) << 18;
        data |= (static_cast<u32>(id) & 0x07ff) << 21;
    }
};

struct GpuInput
{
    glm::mat4 view_mat;
    glm::mat4 shadow_view_mat[3];
    daxa::ImageId shadow_depth_image[3];
    float time;
};

struct RasterPush
{
    glm::vec3 chunk_pos;
    daxa::BufferId input_buffer_id;
    daxa::BufferId face_buffer_id;
    daxa::ImageId texture_array_id;
    daxa::SamplerId sampler_id;
    u32 mode;
    u32 data0;
};

struct ChunkgenComputePush
{
    glm::vec3 chunk_pos;
    daxa::BufferId buffer_id;
};

static inline constexpr u64 CHUNK_SIZE = 64;
static inline constexpr u64 CHUNK_VOXEL_N = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
static inline constexpr u64 CHUNK_MAX_VERTS = CHUNK_VOXEL_N * 6;
static inline constexpr u64 CHUNK_MAX_SIZE = CHUNK_MAX_VERTS * sizeof(Vertex);

static inline constexpr u64 CHUNK_N = 4;

struct Voxel
{
    BlockID id;

    bool is_occluding()
    {
        switch (id)
        {
        case BlockID::Air:
        case BlockID::Rose:
        case BlockID::TallGrass:
        case BlockID::Water: return false;
        default: return true;
        }
    }
    bool is_cross()
    {
        switch (id)
        {
        case BlockID::Rose:
        case BlockID::TallGrass: return true;
        default: return false;
        }
    }

    bool is_occluding_nx() { return is_occluding(); }
    bool is_occluding_px() { return is_occluding(); }
    bool is_occluding_ny() { return is_occluding(); }
    bool is_occluding_py() { return is_occluding(); }
    bool is_occluding_nz() { return is_occluding(); }
    bool is_occluding_pz() { return is_occluding(); }
};

struct VoxelChunk
{
    std::array<Voxel, CHUNK_VOXEL_N> voxels;
    glm::vec3 pos;
};

struct RenderableVoxelWorld;

struct RenderableChunk
{
    VoxelChunk voxel_chunk = {};
    daxa::Device device;
    daxa::BufferId face_buffer;
    daxa::BufferId water_face_buffer;
    u32 face_n = 0, water_face_n = 0;
    RenderableVoxelWorld * renderable_world = nullptr;
    bool valid = false;

    RenderableChunk(daxa::Device & a_device) : device{a_device}
    {
        face_buffer = device.create_buffer(daxa::BufferInfo{
            .size = CHUNK_MAX_SIZE,
            .debug_name = "Chunk Face buffer",
        });
        water_face_buffer = device.create_buffer(daxa::BufferInfo{
            .size = CHUNK_MAX_SIZE,
            .debug_name = "Chunk Water Face buffer",
        });
    }

    ~RenderableChunk()
    {
        device.destroy_buffer(face_buffer);
        device.destroy_buffer(water_face_buffer);
    }

    void update_chunk_mesh(daxa::CommandList & cmd_list);

    void draw(daxa::CommandList & cmd_list, daxa::BufferId gpu_input_buffer_id, daxa::ImageId texture_array_id, daxa::SamplerId atlas_sampler, u32 data0)
    {
        if (valid && face_n > 0)
        {
            cmd_list.push_constant(RasterPush{
                .chunk_pos = voxel_chunk.pos,
                .input_buffer_id = gpu_input_buffer_id,
                .face_buffer_id = face_buffer,
                .texture_array_id = texture_array_id,
                .sampler_id = atlas_sampler,
                .mode = 0,
                .data0 = data0,
            });
            cmd_list.draw({.vertex_count = face_n * 6});
        }
    }
    void draw_water(daxa::CommandList & cmd_list, daxa::BufferId gpu_input_buffer_id, daxa::ImageId texture_array_id, daxa::SamplerId atlas_sampler, u32 data0)
    {
        if (valid && water_face_n > 0)
        {
            cmd_list.push_constant(RasterPush{
                .chunk_pos = voxel_chunk.pos,
                .input_buffer_id = gpu_input_buffer_id,
                .face_buffer_id = water_face_buffer,
                .texture_array_id = texture_array_id,
                .sampler_id = atlas_sampler,
                .mode = 1,
                .data0 = data0,
            });
            cmd_list.draw({.vertex_count = water_face_n * 6});
        }
    }
};

struct RenderableVoxelWorld
{
    daxa::Device & device;
    std::array<std::unique_ptr<RenderableChunk>, CHUNK_N * CHUNK_N * CHUNK_N> chunks = {};

    daxa::ImageId atlas_texture_array;
    daxa::SamplerId atlas_sampler;

    daxa::BufferId voxel_gpu_buffer = device.create_buffer({
        .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
        .size = CHUNK_VOXEL_N * sizeof(u32),
        .debug_name = "Playground Vertex Staging buffer",
    });

    RenderableVoxelWorld(daxa::Device & a_device)
        : device{a_device}
    {
        for (u64 z = 0; z < CHUNK_N; ++z)
        {
            for (u64 y = 0; y < CHUNK_N; ++y)
            {
                for (u64 x = 0; x < CHUNK_N; ++x)
                {
                    chunks[x + y * CHUNK_N + z * CHUNK_N * CHUNK_N] = std::make_unique<RenderableChunk>(device);
                    auto & chunk = *chunks[x + y * CHUNK_N + z * CHUNK_N * CHUNK_N];
                    chunk.voxel_chunk.pos = glm::vec3(static_cast<f32>(x * CHUNK_SIZE), static_cast<f32>(y * CHUNK_SIZE), static_cast<f32>(z * CHUNK_SIZE));
                    chunk.renderable_world = this;
                }
            }
        }

        load_textures("tests/0_common/assets/textures");

        atlas_sampler = device.create_sampler({
            .magnification_filter = daxa::Filter::NEAREST,
            .minification_filter = daxa::Filter::NEAREST,
            .mipmap_filter = daxa::Filter::NEAREST,
            .min_lod = 0,
            .max_lod = 4,
        });
    }

    ~RenderableVoxelWorld()
    {
        device.destroy_image(atlas_texture_array);
        device.destroy_sampler(atlas_sampler);
        device.destroy_buffer(voxel_gpu_buffer);
    }

    void regenerate_chunk(RenderableChunk & chunk, daxa::ComputePipeline & compute_pipeline)
    {
        auto cmd_list = device.create_command_list({
            .debug_name = "Playground Command List",
        });

        cmd_list.set_pipeline(compute_pipeline);
        cmd_list.push_constant(ChunkgenComputePush{
            .chunk_pos = chunk.voxel_chunk.pos,
            .buffer_id = voxel_gpu_buffer,
        });
        cmd_list.dispatch(8, 8, 8);

        cmd_list.complete();
        device.submit_commands({
            .command_lists = {std::move(cmd_list)},
        });
        device.wait_idle();

        std::array<Voxel, CHUNK_VOXEL_N> * buffer_ptr = device.map_memory_as<std::array<Voxel, CHUNK_VOXEL_N>>(voxel_gpu_buffer);
        chunk.voxel_chunk.voxels = *buffer_ptr;
        device.unmap_memory(voxel_gpu_buffer);
    }

    void update_meshes(daxa::ComputePipeline & compute_pipeline)
    {
        for (u64 z = 0; z < CHUNK_N; ++z)
        {
            for (u64 y = 0; y < CHUNK_N; ++y)
            {
                for (u64 x = 0; x < CHUNK_N; ++x)
                {
                    auto & chunk = *chunks[x + y * CHUNK_N + z * CHUNK_N * CHUNK_N];
                    regenerate_chunk(chunk, compute_pipeline);
                }
            }
        }

        for (u64 z = 0; z < CHUNK_N; ++z)
        {
            for (u64 y = 0; y < CHUNK_N; ++y)
            {
                for (u64 x = 0; x < CHUNK_N; ++x)
                {
                    auto & chunk = *chunks[x + y * CHUNK_N + z * CHUNK_N * CHUNK_N];
                    auto cmd_list = device.create_command_list({
                        .debug_name = "Playground Command List",
                    });
                    chunk.update_chunk_mesh(cmd_list);
                    cmd_list.complete();
                    device.submit_commands({
                        .command_lists = {std::move(cmd_list)},
                    });
                }
            }
        }
    }

    void update(daxa::ComputePipeline & compute_pipeline, glm::vec3)
    {
        // auto invalid_chunk_iter = std::min_element(
        //     chunks.begin(), chunks.end(),
        //     [&](std::unique_ptr<RenderableChunk> const & a, std::unique_ptr<RenderableChunk> const & b)
        //     {
        //         if (a->valid)
        //             return false;
        //         if (b->valid)
        //             return true;
        //         glm::vec3 del_a = pos - (a->voxel_chunk.pos + 16.0f);
        //         glm::vec3 del_b = pos - (b->voxel_chunk.pos + 16.0f);
        //         return glm::dot(del_a, del_a) < glm::dot(del_b, del_b);
        //     });
        auto invalid_chunk_iter = std::find_if(
            chunks.begin(), chunks.end(),
            [&](std::unique_ptr<RenderableChunk> const & a)
            {
                return !a->valid;
            });

        if (invalid_chunk_iter != chunks.end())
        {
            auto & chunk = **invalid_chunk_iter;
            regenerate_chunk(chunk, compute_pipeline);

            auto cmd_list = device.create_command_list({
                .debug_name = "Playground Command List",
            });
            chunk.update_chunk_mesh(cmd_list);
            cmd_list.complete();
            device.submit_commands({
                .command_lists = {std::move(cmd_list)},
            });
            device.wait_idle();
        }
    }

    void invalidate_all()
    {
        for (auto & chunk : chunks)
        {
            chunk->valid = false;
        }
    }

    void draw(daxa::CommandList & cmd_list, daxa::BufferId gpu_input_buffer_id, u32 data0)
    {
        for (auto & chunk : chunks)
            chunk->draw(cmd_list, gpu_input_buffer_id, atlas_texture_array, atlas_sampler, data0);
        for (auto & chunk : chunks)
            chunk->draw_water(cmd_list, gpu_input_buffer_id, atlas_texture_array, atlas_sampler, data0);
    }

    Voxel get_voxel(glm::ivec3 p)
    {
        i32 x = p.x / static_cast<i32>(CHUNK_SIZE);
        i32 y = p.y / static_cast<i32>(CHUNK_SIZE);
        i32 z = p.z / static_cast<i32>(CHUNK_SIZE);

        if (p.x < 0 || x >= static_cast<i32>(CHUNK_N))
            return {.id = BlockID::Air};
        if (p.y < 0 || y >= static_cast<i32>(CHUNK_N))
            return {.id = BlockID::Air};
        if (p.z < 0 || z >= static_cast<i32>(CHUNK_N))
            return {.id = BlockID::Air};

        auto & chunk = chunks[static_cast<u64>(x) + static_cast<u64>(y) * CHUNK_N + static_cast<u64>(z) * CHUNK_N * CHUNK_N];

        u64 xi = static_cast<u64>(p.x) % CHUNK_SIZE;
        u64 yi = static_cast<u64>(p.y) % CHUNK_SIZE;
        u64 zi = static_cast<u64>(p.z) % CHUNK_SIZE;
        u64 i = xi + yi * CHUNK_SIZE + zi * CHUNK_SIZE * CHUNK_SIZE;
        return chunk->voxel_chunk.voxels[i];
    }

    void load_textures(std::filesystem::path const & filepath)
    {
        std::array<std::filesystem::path, 32> texture_names{
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

        atlas_texture_array = device.create_image({
            .format = daxa::Format::R8G8B8A8_SRGB,
            .size = {16, 16, 1},
            .mip_level_count = 5,
            .array_layer_count = static_cast<u32>(texture_names.size()),
            .usage = daxa::ImageUsageFlagBits::SHADER_READ_ONLY | daxa::ImageUsageFlagBits::TRANSFER_SRC | daxa::ImageUsageFlagBits::TRANSFER_DST,
        });

        usize image_size = 16 * 16 * sizeof(u8) * 4;

        auto texture_staging_buffer = device.create_buffer({
            .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
            .size = static_cast<u32>(image_size * texture_names.size()),
        });

        u8 * staging_buffer_data = device.map_memory_as<u8>(texture_staging_buffer);
        for (usize i = 0; i < texture_names.size(); ++i)
        {
            stbi_set_flip_vertically_on_load(true);
            auto path = filepath / texture_names[i];
            int size_x, size_y, num_channels;
            std::uint8_t * data = stbi_load(path.string().c_str(), &size_x, &size_y, &num_channels, 4);
            if (!data)
            {
                std::cout << "could not find file: \"" << path << "\"" << std::endl;
                continue;
            }
            usize offset = i * 16 * 16 * 4;
            for (usize j = 0; j < 16 * 16; ++j)
            {
                usize data_i = j * 4;
                staging_buffer_data[offset + data_i + 0] = data[data_i + 0];
                staging_buffer_data[offset + data_i + 1] = data[data_i + 1];
                staging_buffer_data[offset + data_i + 2] = data[data_i + 2];
                staging_buffer_data[offset + data_i + 3] = data[data_i + 3];
            }
        }
        device.unmap_memory(texture_staging_buffer);

        auto cmd_list = device.create_command_list({});

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::HOST_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
        });

        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::HOST_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::UNDEFINED,
            .after_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .image_id = atlas_texture_array,
            .image_slice = {
                .base_mip_level = 0,
                .level_count = 1,
                .base_array_layer = 0,
                .layer_count = static_cast<u32>(texture_names.size()),
            },
        });

        for (usize i = 0; i < texture_names.size(); ++i)
        {
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

        generate_mip_levels(device, cmd_list, {.image = atlas_texture_array, .base_array_layer = 0, .layer_count = static_cast<u32>(texture_names.size())});

        cmd_list.complete();

        device.submit_commands({
            .command_lists = {std::move(cmd_list)},
        });
        device.wait_idle();

        device.destroy_buffer(texture_staging_buffer);
    }
};

void RenderableChunk::update_chunk_mesh(daxa::CommandList & cmd_list)
{
    {
        auto face_staging_buffer = device.create_buffer({
            .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
            .size = CHUNK_MAX_SIZE,
            .debug_name = "Playground Vertex Staging buffer",
        });
        cmd_list.destroy_buffer_deferred(face_staging_buffer);

        Vertex * buffer_ptr = device.map_memory_as<Vertex>(face_staging_buffer);
        face_n = 0;
        for (i32 zi = 0; zi < static_cast<i32>(CHUNK_SIZE); ++zi)
        {
            for (i32 yi = 0; yi < static_cast<i32>(CHUNK_SIZE); ++yi)
            {
                for (i32 xi = 0; xi < static_cast<i32>(CHUNK_SIZE); ++xi)
                {
                    glm::ivec3 voxel_pos = glm::ivec3{xi, yi, zi} + glm::ivec3(voxel_chunk.pos);
                    Voxel current_voxel = renderable_world->get_voxel(voxel_pos);
                    if (current_voxel.is_occluding())
                    {
                        if (!renderable_world->get_voxel(voxel_pos + glm::ivec3(+1, 0, 0)).is_occluding_nx())
                            *buffer_ptr = Vertex(xi, yi, zi, 0, current_voxel.id), ++buffer_ptr, ++face_n;
                        if (!renderable_world->get_voxel(voxel_pos + glm::ivec3(-1, 0, 0)).is_occluding_px())
                            *buffer_ptr = Vertex(xi, yi, zi, 1, current_voxel.id), ++buffer_ptr, ++face_n;
                        if (!renderable_world->get_voxel(voxel_pos + glm::ivec3(0, +1, 0)).is_occluding_ny())
                            *buffer_ptr = Vertex(xi, yi, zi, 2, current_voxel.id), ++buffer_ptr, ++face_n;
                        if (!renderable_world->get_voxel(voxel_pos + glm::ivec3(0, -1, 0)).is_occluding_py())
                            *buffer_ptr = Vertex(xi, yi, zi, 3, current_voxel.id), ++buffer_ptr, ++face_n;
                        if (!renderable_world->get_voxel(voxel_pos + glm::ivec3(0, 0, +1)).is_occluding_nz())
                            *buffer_ptr = Vertex(xi, yi, zi, 4, current_voxel.id), ++buffer_ptr, ++face_n;
                        if (!renderable_world->get_voxel(voxel_pos + glm::ivec3(0, 0, -1)).is_occluding_pz())
                            *buffer_ptr = Vertex(xi, yi, zi, 5, current_voxel.id), ++buffer_ptr, ++face_n;
                    }
                    else if (current_voxel.is_cross())
                    {
                        *buffer_ptr = Vertex(xi, yi, zi, 6, current_voxel.id), ++buffer_ptr, ++face_n;
                        *buffer_ptr = Vertex(xi, yi, zi, 6, current_voxel.id), ++buffer_ptr, ++face_n;
                        *buffer_ptr = Vertex(xi, yi, zi, 7, current_voxel.id), ++buffer_ptr, ++face_n;
                        *buffer_ptr = Vertex(xi, yi, zi, 7, current_voxel.id), ++buffer_ptr, ++face_n;
                    }
                }
            }
        }
        device.unmap_memory(face_staging_buffer);

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::HOST_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
        });

        cmd_list.copy_buffer_to_buffer({
            .src_buffer = face_staging_buffer,
            .dst_buffer = face_buffer,
            .size = CHUNK_MAX_SIZE,
        });

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::VERTEX_SHADER_READ,
        });
    }

    {
        auto face_staging_buffer = device.create_buffer({
            .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
            .size = CHUNK_MAX_SIZE,
            .debug_name = "Playground Vertex Staging buffer",
        });
        cmd_list.destroy_buffer_deferred(face_staging_buffer);

        Vertex * buffer_ptr = device.map_memory_as<Vertex>(face_staging_buffer);
        water_face_n = 0;
        for (i32 zi = 0; zi < static_cast<i32>(CHUNK_SIZE); ++zi)
        {
            for (i32 yi = 0; yi < static_cast<i32>(CHUNK_SIZE); ++yi)
            {
                for (i32 xi = 0; xi < static_cast<i32>(CHUNK_SIZE); ++xi)
                {
                    glm::ivec3 voxel_pos = glm::ivec3{xi, yi, zi} + glm::ivec3(voxel_chunk.pos);
                    Voxel current_voxel = renderable_world->get_voxel(voxel_pos);
                    if (current_voxel.id == BlockID::Water)
                    {
                        if (renderable_world->get_voxel(voxel_pos + glm::ivec3(+1, 0, 0)).id == BlockID::Air)
                            *buffer_ptr = Vertex(xi, yi, zi, 0, current_voxel.id), ++buffer_ptr, ++water_face_n;
                        if (renderable_world->get_voxel(voxel_pos + glm::ivec3(-1, 0, 0)).id == BlockID::Air)
                            *buffer_ptr = Vertex(xi, yi, zi, 1, current_voxel.id), ++buffer_ptr, ++water_face_n;
                        if (renderable_world->get_voxel(voxel_pos + glm::ivec3(0, +1, 0)).id == BlockID::Air)
                            *buffer_ptr = Vertex(xi, yi, zi, 2, current_voxel.id), ++buffer_ptr, ++water_face_n;
                        if (renderable_world->get_voxel(voxel_pos + glm::ivec3(0, -1, 0)).id == BlockID::Air)
                            *buffer_ptr = Vertex(xi, yi, zi, 3, current_voxel.id), ++buffer_ptr, ++water_face_n;
                        if (renderable_world->get_voxel(voxel_pos + glm::ivec3(0, 0, +1)).id == BlockID::Air)
                            *buffer_ptr = Vertex(xi, yi, zi, 4, current_voxel.id), ++buffer_ptr, ++water_face_n;
                        if (renderable_world->get_voxel(voxel_pos + glm::ivec3(0, 0, -1)).id == BlockID::Air)
                            *buffer_ptr = Vertex(xi, yi, zi, 5, current_voxel.id), ++buffer_ptr, ++water_face_n;
                    }
                }
            }
        }
        device.unmap_memory(face_staging_buffer);

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::HOST_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
        });

        cmd_list.copy_buffer_to_buffer({
            .src_buffer = face_staging_buffer,
            .dst_buffer = water_face_buffer,
            .size = CHUNK_MAX_SIZE,
        });

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::VERTEX_SHADER_READ,
        });
    }

    valid = true;
}

struct App : AppWindow<App>
{
    daxa::Context daxa_ctx = daxa::create_context({
        .enable_validation = false,
    });
    daxa::Device device = daxa_ctx.create_device({});

    daxa::Swapchain swapchain = device.create_swapchain({
        .native_window = get_native_handle(),
        .width = size_x,
        .height = size_y,
        .surface_format_selector = [](daxa::Format format)
        {
            switch (format)
            {
            case daxa::Format::R8G8B8A8_UINT: return 100;
            default: return daxa::default_format_score(format);
            }
        },
        .present_mode = daxa::PresentMode::DO_NOT_WAIT_FOR_VBLANK,
        .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
        .debug_name = "Playground Swapchain",
    });

    daxa::ImageId depth_image = device.create_image({
        .format = daxa::Format::D32_SFLOAT,
        .aspect = daxa::ImageAspectFlagBits::DEPTH,
        .size = {size_x, size_y, 1},
        .usage = daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT,
    });

    daxa::ImageId shadow_depth_image0 = device.create_image({
        .format = daxa::Format::D32_SFLOAT,
        .aspect = daxa::ImageAspectFlagBits::DEPTH,
        .size = {SHADOW_RES, SHADOW_RES, 1},
        .usage = daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT | daxa::ImageUsageFlagBits::SHADER_READ_ONLY,
    });
    daxa::ImageId shadow_depth_image1 = device.create_image({
        .format = daxa::Format::D32_SFLOAT,
        .aspect = daxa::ImageAspectFlagBits::DEPTH,
        .size = {SHADOW_RES, SHADOW_RES, 1},
        .usage = daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT | daxa::ImageUsageFlagBits::SHADER_READ_ONLY,
    });
    daxa::ImageId shadow_depth_image2 = device.create_image({
        .format = daxa::Format::D32_SFLOAT,
        .aspect = daxa::ImageAspectFlagBits::DEPTH,
        .size = {SHADOW_RES, SHADOW_RES, 1},
        .usage = daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT | daxa::ImageUsageFlagBits::SHADER_READ_ONLY,
    });

    daxa::BufferId gpu_input_buffer_id = device.create_buffer({
        .size = sizeof(GpuInput),
    });

    daxa::PipelineCompiler pipeline_compiler = device.create_pipeline_compiler({
        .root_paths = {
            "tests/0_common/shaders",
            "tests/3_samples/6_gpu_based/shaders",
            "include",
        },
        .debug_name = "Playground Compiler",
    });

    // clang-format off
    daxa::RasterPipeline raster_pipeline = pipeline_compiler.create_raster_pipeline({
        .vertex_shader_info = {.source = daxa::ShaderFile{"vert.hlsl"}},
        .fragment_shader_info = {.source = daxa::ShaderFile{"frag.hlsl"}},
#if VISUALIZE_OVERDRAW
        .color_attachments = {{.format = swapchain.get_format(), .blend = {.blend_enable = true, .src_color_blend_factor = daxa::BlendFactor::ONE, .dst_color_blend_factor = daxa::BlendFactor::ONE}}},
#else
        .color_attachments = {{.format = swapchain.get_format(), .blend = {.blend_enable = true, .src_color_blend_factor = daxa::BlendFactor::SRC_ALPHA, .dst_color_blend_factor = daxa::BlendFactor::ONE_MINUS_SRC_ALPHA}}},
#endif
        .depth_test = {
            .depth_attachment_format = daxa::Format::D32_SFLOAT,
#if VISUALIZE_OVERDRAW
            .enable_depth_test = false,
            .enable_depth_write = false,
#else
            .enable_depth_test = true,
            .enable_depth_write = true,
#endif
        },
        .raster = {
            .face_culling = daxa::FaceCullFlagBits::BACK_BIT,
        },
        .push_constant_size = sizeof(RasterPush),
        .debug_name = "Playground Pipeline",
    }).value();
    daxa::RasterPipeline shadow_pipeline = pipeline_compiler.create_raster_pipeline({
        .vertex_shader_info = {.source = daxa::ShaderFile{"shadow_vert.hlsl"}},
        .fragment_shader_info = {.source = daxa::ShaderFile{"shadow_frag.hlsl"}},
        .depth_test = {
            .depth_attachment_format = daxa::Format::D32_SFLOAT,
            .enable_depth_test = true,
            .enable_depth_write = true,
        },
        .raster = {
            .face_culling = daxa::FaceCullFlagBits::BACK_BIT,
        },
        .push_constant_size = sizeof(RasterPush),
        .debug_name = "Playground Pipeline",
    }).value();
    daxa::ComputePipeline chunkgen_compute_pipeline = pipeline_compiler.create_compute_pipeline({
        .shader_info = {.source = daxa::ShaderFile{"chunkgen.hlsl"}},
        .push_constant_size = sizeof(ChunkgenComputePush),
        .debug_name = "Playground Chunkgen Compute Pipeline",
    }).value();
    // clang-format on

    daxa::BinarySemaphore binary_semaphore = device.create_binary_semaphore({
        .debug_name = "Playground Present Semaphore",
    });

    static inline constexpr u64 FRAMES_IN_FLIGHT = 1;
    daxa::TimelineSemaphore gpu_framecount_timeline_sema = device.create_timeline_semaphore(daxa::TimelineSemaphoreInfo{
        .initial_value = 0,
        .debug_name = "Playground gpu framecount Timeline Semaphore",
    });
    u64 cpu_framecount = FRAMES_IN_FLIGHT - 1;

    Clock::time_point start = Clock::now(), prev_time = start;

    RenderableVoxelWorld renderable_world{device};
    Player3D player = {
        .rot = {2.0f, 0.0f, 0.0f},
    };
    bool should_resize = false, paused = true;
    u32 camera_index = 0;

    App() : AppWindow<App>("Samples: Playground")
    {
        renderable_world.update_meshes(chunkgen_compute_pipeline);
    }

    ~App()
    {
        device.destroy_image(depth_image);
        device.destroy_image(shadow_depth_image0);
        device.destroy_image(shadow_depth_image1);
        device.destroy_image(shadow_depth_image2);
        device.destroy_buffer(gpu_input_buffer_id);
    }

    bool update()
    {
        glfwPollEvents();
        if (glfwWindowShouldClose(glfw_window_ptr))
        {
            return true;
        }

        if (!minimized)
        {
            draw();
        }
        else
        {
            using namespace std::literals;
            std::this_thread::sleep_for(1ms);
        }

        return false;
    }

    void draw()
    {
        auto now = Clock::now();
        auto elapsed_s = std::chrono::duration<f32>(now - prev_time).count();
        auto time = std::chrono::duration<f32>(now - start).count();
        prev_time = now;

        player.camera.resize(static_cast<i32>(size_x), static_cast<i32>(size_y));
        player.camera.set_pos(player.pos);
        player.camera.set_rot(player.rot.x, player.rot.y);
        player.update(elapsed_s);

        renderable_world.update(chunkgen_compute_pipeline, player.pos);

        if (pipeline_compiler.check_if_sources_changed(raster_pipeline))
        {
            auto new_pipeline = pipeline_compiler.recreate_raster_pipeline(raster_pipeline);
            if (new_pipeline.is_ok())
            {
                raster_pipeline = new_pipeline.value();
            }
            else
            {
                std::cout << new_pipeline.message() << std::endl;
            }
        }

        if (pipeline_compiler.check_if_sources_changed(chunkgen_compute_pipeline))
        {
            auto new_pipeline = pipeline_compiler.recreate_compute_pipeline(chunkgen_compute_pipeline);
            if (new_pipeline.is_ok())
            {
                chunkgen_compute_pipeline = new_pipeline.value();
                // renderable_world.update_meshes(chunkgen_compute_pipeline);
                renderable_world.invalidate_all();
            }
            else
            {
                std::cout << new_pipeline.message() << std::endl;
            }
        }

        if (should_resize)
        {
            do_resize();
        }

        auto swapchain_image = swapchain.acquire_next_image();

        auto cmd_list = device.create_command_list({
            .debug_name = "Playground Command List",
        });

        auto shadow_map_view0 = glm::ortho(-16.0f, 16.0f, -16.0f, 16.0f, -64.0f, 64.0f);
        auto shadow_map_view1 = glm::ortho(-64.0f, 64.0f, -64.0f, 64.0f, -256.0f, 256.0f);
        auto shadow_map_view2 = glm::ortho(-256.0f, 256.0f, -256.0f, 256.0f, -1024.0f, 1024.0f);
        glm::vec3 offset0 = -player.pos - glm::vec3(1, 3, 2) * 4.0f;
        glm::vec3 offset1 = -player.pos - glm::vec3(1, 3, 2) * 16.0f;
        glm::vec3 offset2 = -player.pos - glm::vec3(1, 3, 2) * 64.0f;
        shadow_map_view0 = shadow_map_view0 * glm::lookAt(glm::vec3(0, 0, 0) + offset0, glm::vec3(1, 3, 2) + offset0, glm::vec3(0, -1, 0));
        shadow_map_view1 = shadow_map_view1 * glm::lookAt(glm::vec3(0, 0, 0) + offset1, glm::vec3(1, 3, 2) + offset1, glm::vec3(0, -1, 0));
        shadow_map_view2 = shadow_map_view2 * glm::lookAt(glm::vec3(0, 0, 0) + offset2, glm::vec3(1, 3, 2) + offset2, glm::vec3(0, -1, 0));
        glm::mat4 player_view = player.camera.get_vp();
        switch (camera_index)
        {
        case 0: player_view = player.camera.get_vp(); break;
        case 1: player_view = shadow_map_view0; break;
        case 2: player_view = shadow_map_view1; break;
        case 3: player_view = shadow_map_view2; break;
        default: break;
        }
        auto gpu_input_staging_buffer = device.create_buffer({
            .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
            .size = sizeof(GpuInput),
            .debug_name = "Playground Input Staging buffer",
        });
        cmd_list.destroy_buffer_deferred(gpu_input_staging_buffer);
        GpuInput * buffer_ptr = device.map_memory_as<GpuInput>(gpu_input_staging_buffer);
        *buffer_ptr = GpuInput{
            .view_mat = player_view,
            .shadow_view_mat = {shadow_map_view0, shadow_map_view1, shadow_map_view2},
            .shadow_depth_image = {shadow_depth_image0, shadow_depth_image1, shadow_depth_image2},
            .time = time,
        };
        device.unmap_memory(gpu_input_staging_buffer);
        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::HOST_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
        });
        cmd_list.copy_buffer_to_buffer({
            .src_buffer = gpu_input_staging_buffer,
            .dst_buffer = gpu_input_buffer_id,
            .size = sizeof(GpuInput),
        });
        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::VERTEX_SHADER_READ,
        });

        cmd_list.pipeline_barrier_image_transition({
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::UNDEFINED,
            .after_layout = daxa::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
            .image_id = swapchain_image,
        });

        cmd_list.pipeline_barrier_image_transition({
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::UNDEFINED,
            .after_layout = daxa::ImageLayout::DEPTH_ATTACHMENT_OPTIMAL,
            .image_id = depth_image,
            .image_slice = {.image_aspect = daxa::ImageAspectFlagBits::DEPTH},
        });

        // shadow mapping pass
        cmd_list.pipeline_barrier_image_transition({
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::UNDEFINED,
            .after_layout = daxa::ImageLayout::DEPTH_ATTACHMENT_OPTIMAL,
            .image_id = shadow_depth_image0,
            .image_slice = {.image_aspect = daxa::ImageAspectFlagBits::DEPTH},
        });
        cmd_list.pipeline_barrier_image_transition({
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::UNDEFINED,
            .after_layout = daxa::ImageLayout::DEPTH_ATTACHMENT_OPTIMAL,
            .image_id = shadow_depth_image1,
            .image_slice = {.image_aspect = daxa::ImageAspectFlagBits::DEPTH},
        });
        cmd_list.pipeline_barrier_image_transition({
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::UNDEFINED,
            .after_layout = daxa::ImageLayout::DEPTH_ATTACHMENT_OPTIMAL,
            .image_id = shadow_depth_image2,
            .image_slice = {.image_aspect = daxa::ImageAspectFlagBits::DEPTH},
        });

        cmd_list.begin_renderpass({
            .depth_attachment = {{
                .image_view = shadow_depth_image0.default_view(),
                .load_op = daxa::AttachmentLoadOp::CLEAR,
                .clear_value = daxa::DepthValue{1.0f, 0},
            }},
            .render_area = {.x = 0, .y = 0, .width = SHADOW_RES, .height = SHADOW_RES},
        });
        cmd_list.set_pipeline(shadow_pipeline);
        renderable_world.draw(cmd_list, gpu_input_buffer_id, 0);
        cmd_list.end_renderpass();

        cmd_list.begin_renderpass({
            .depth_attachment = {{
                .image_view = shadow_depth_image1.default_view(),
                .load_op = daxa::AttachmentLoadOp::CLEAR,
                .clear_value = daxa::DepthValue{1.0f, 0},
            }},
            .render_area = {.x = 0, .y = 0, .width = SHADOW_RES, .height = SHADOW_RES},
        });
        cmd_list.set_pipeline(shadow_pipeline);
        renderable_world.draw(cmd_list, gpu_input_buffer_id, 1);
        cmd_list.end_renderpass();

        cmd_list.begin_renderpass({
            .depth_attachment = {{
                .image_view = shadow_depth_image2.default_view(),
                .load_op = daxa::AttachmentLoadOp::CLEAR,
                .clear_value = daxa::DepthValue{1.0f, 0},
            }},
            .render_area = {.x = 0, .y = 0, .width = SHADOW_RES, .height = SHADOW_RES},
        });
        cmd_list.set_pipeline(shadow_pipeline);
        renderable_world.draw(cmd_list, gpu_input_buffer_id, 2);
        cmd_list.end_renderpass();

        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::DEPTH_ATTACHMENT_OPTIMAL,
            .after_layout = daxa::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
            .image_id = shadow_depth_image0,
        });
        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::DEPTH_ATTACHMENT_OPTIMAL,
            .after_layout = daxa::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
            .image_id = shadow_depth_image1,
        });
        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::DEPTH_ATTACHMENT_OPTIMAL,
            .after_layout = daxa::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
            .image_id = shadow_depth_image2,
        });
        // end shadow mapping pass

        cmd_list.begin_renderpass({
            .color_attachments = {{
                .image_view = swapchain_image.default_view(),
                .load_op = daxa::AttachmentLoadOp::CLEAR,
#if VISUALIZE_OVERDRAW
                .clear_value = std::array<f32, 4>{0.0f, 0.0f, 0.0f, 1.0f},
#else
                .clear_value = std::array<f32, 4>{0.2f, 0.4f, 1.0f, 1.0f},
#endif
            }},
            .depth_attachment = {{
                .image_view = depth_image.default_view(),
                .load_op = daxa::AttachmentLoadOp::CLEAR,
                .clear_value = daxa::DepthValue{1.0f, 0},
            }},
            .render_area = {.x = 0, .y = 0, .width = size_x, .height = size_y},
        });

        cmd_list.set_pipeline(raster_pipeline);
        renderable_world.draw(cmd_list, gpu_input_buffer_id, 0);
        cmd_list.end_renderpass();

        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
            .after_layout = daxa::ImageLayout::PRESENT_SRC,
            .image_id = swapchain_image,
        });

        cmd_list.complete();

        ++cpu_framecount;
        device.submit_commands({
            .command_lists = {std::move(cmd_list)},
            .signal_binary_semaphores = {binary_semaphore},
            .signal_timeline_semaphores = {{gpu_framecount_timeline_sema, cpu_framecount}},
        });

        device.present_frame({
            .wait_binary_semaphores = {binary_semaphore},
            .swapchain = swapchain,
        });

        gpu_framecount_timeline_sema.wait_for_value(cpu_framecount - 1);
    }

    void on_mouse_move(f32 x, f32 y)
    {
        if (!paused)
        {
            f32 center_x = static_cast<f32>(size_x / 2);
            f32 center_y = static_cast<f32>(size_y / 2);
            auto offset = glm::vec2{x - center_x, center_y - y};
            player.on_mouse_move(static_cast<f64>(offset.x), static_cast<f64>(offset.y));
            set_mouse_pos(center_x, center_y);
        }
    }

    void on_key(int key, int action)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            toggle_pause();
        }

        if (!paused)
        {
            player.on_key(key, action);
            if (key == GLFW_KEY_F5 && action == GLFW_PRESS)
            {
                toggle_view();
            }
        }
    }

    void on_resize(u32 sx, u32 sy)
    {
        size_x = sx;
        size_y = sy;
        minimized = (sx == 0 || sy == 0);

        if (!minimized)
        {
            should_resize = true;
            do_resize();
        }
    }

    void do_resize()
    {
        should_resize = false;
        device.destroy_image(depth_image);
        depth_image = device.create_image({
            .format = daxa::Format::D32_SFLOAT,
            .aspect = daxa::ImageAspectFlagBits::DEPTH,
            .size = {size_x, size_y, 1},
            .usage = daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT,
        });
        swapchain.resize(size_x, size_y);
        draw();
    }

    void toggle_pause()
    {
        set_mouse_capture(paused);
        paused = !paused;
    }

    void toggle_view()
    {
        camera_index = (camera_index + 1) % 4;
    }
};

int main()
{
    App app = {};
    while (true)
    {
        if (app.update())
            break;
    }
}
