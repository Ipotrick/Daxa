#include <0_common/window.hpp>
#include <0_common/player.hpp>
#include <0_common/hlsl_util.hpp>
#include <thread>
#include <iostream>

#include <daxa/utils/task_list.hpp>

#include <daxa/utils/imgui.hpp>
#include <0_common/imgui/imgui_impl_glfw.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace daxa::types;
using Clock = std::chrono::high_resolution_clock;

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
        data |= (static_cast<u32>(x) & 0x1f) << 0;
        data |= (static_cast<u32>(y) & 0x1f) << 5;
        data |= (static_cast<u32>(z) & 0x1f) << 10;
        data |= (side & 0x7) << 15;
        data |= (static_cast<u32>(id) & 0x3fff) << 18;
    }
};

struct RasterPush
{
    glm::mat4 view_mat;
    glm::vec3 chunk_pos;
    daxa::BufferId vertex_buffer_id;
    daxa::ImageId texture_array_id;
    daxa::SamplerId sampler_id;
    u32 mode;
};

static inline constexpr u64 CHUNK_SIZE = 32;
static inline constexpr u64 CHUNK_VOXEL_N = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
static inline constexpr u64 CHUNK_MAX_VERTS = CHUNK_VOXEL_N * 6;
static inline constexpr u64 CHUNK_MAX_SIZE = CHUNK_MAX_VERTS * sizeof(Vertex);

static inline constexpr u64 CHUNK_N = 2;

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
    Voxel voxels[CHUNK_VOXEL_N];
    glm::vec3 pos;

    BlockID generate_block_id(glm::vec3 p)
    {
        p = p * 0.5f;
        FractalNoiseConfig noise_conf = {
            /* .amplitude   = */ 1.0f,
            /* .persistance = */ 0.12f,
            /* .scale       = */ 0.03f,
            /* .lacunarity  = */ 4.7f,
            /* .octaves     = */ 2,
        };
        float val = fractal_noise(p + 100.0f, noise_conf);
        val = val - (-p.y + 30.0f) * 0.04f;
        val -= std::pow(smoothstep(-1.0f, 1.0f, -p.y + 32.0f), 2.0f) * 0.15f;
        val = std::max(val, 0.0f);
        if (val > 0.0f)
            return BlockID::Stone;
        if (p.y > 33.0f)
            return BlockID::Water;
        return BlockID::Air;
    }

    void generate()
    {
        for (u64 zi = 0; zi < 32; ++zi)
            for (u64 yi = 0; yi < 32; ++yi)
                for (u64 xi = 0; xi < 32; ++xi)
                {
                    u64 i = xi + yi * CHUNK_SIZE + zi * CHUNK_SIZE * CHUNK_SIZE;
                    glm::vec3 block_pos = glm::vec3(xi, yi, zi) + pos;
                    voxels[i].id = generate_block_id(block_pos);
                    if (voxels[i].id == BlockID::Stone)
                    {
                        u32 above_i;
                        for (above_i = 0; above_i < 6; ++above_i)
                        {
                            if (generate_block_id(block_pos - glm::vec3(0, above_i + 1, 0)) == BlockID::Air)
                                break;
                        }
                        switch (rand() % 10)
                        {
                        case 0: voxels[i].id = BlockID::Gravel; break;
                        case 1: voxels[i].id = BlockID::Cobblestone; break;
                        default: break;
                        }
                        if (above_i == 0)
                            voxels[i].id = BlockID::Grass;
                        else if (above_i < 4)
                            voxels[i].id = BlockID::Dirt;
                    }
                    else if (voxels[i].id == BlockID::Air)
                    {
                        u32 below_i;
                        for (below_i = 0; below_i < 6; ++below_i)
                        {
                            if (generate_block_id(block_pos + glm::vec3(0, below_i + 1, 0)) == BlockID::Stone)
                                break;
                        }
                        if (below_i == 0)
                        {
                            i32 r = rand() % 100;
                            if (r < 50)
                            {
                                switch (r)
                                {
                                case 0: voxels[i].id = BlockID::Rose; break;
                                default:
                                    voxels[i].id = BlockID::TallGrass;
                                    break;
                                }
                            }
                        }
                    }
                }
    }
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

    void draw(daxa::CommandList & cmd_list, glm::mat4 const & view_mat, daxa::ImageId texture_array_id, daxa::SamplerId atlas_sampler)
    {
        if (face_n > 0)
        {
            cmd_list.push_constant(RasterPush{
                .view_mat = view_mat,
                .chunk_pos = voxel_chunk.pos,
                .vertex_buffer_id = face_buffer,
                .texture_array_id = texture_array_id,
                .sampler_id = atlas_sampler,
                .mode = 0,
            });
            cmd_list.draw({.vertex_count = face_n * 6});
        }
    }
    void draw_water(daxa::CommandList & cmd_list, glm::mat4 const & view_mat, daxa::ImageId texture_array_id, daxa::SamplerId atlas_sampler)
    {
        if (water_face_n > 0)
        {
            cmd_list.push_constant(RasterPush{
                .view_mat = view_mat,
                .chunk_pos = voxel_chunk.pos,
                .vertex_buffer_id = water_face_buffer,
                .texture_array_id = texture_array_id,
                .sampler_id = atlas_sampler,
                .mode = 1,
            });
            cmd_list.draw({.vertex_count = water_face_n * 6});
        }
    }
};

struct RenderableVoxelWorld
{
    daxa::Device & device;
    std::unique_ptr<RenderableChunk> chunks[CHUNK_N * CHUNK_N * CHUNK_N] = {};

    daxa::ImageId atlas_texture_array;
    daxa::SamplerId atlas_sampler;

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
                    chunk.voxel_chunk.generate();
                    chunk.renderable_world = this;
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

        load_textures("tests/0_common/assets/textures");

        atlas_sampler = device.create_sampler({
            .magnification_filter = daxa::Filter::NEAREST,
            .minification_filter = daxa::Filter::LINEAR,
            .min_lod = 0,
            .max_lod = 4,
        });
    }

    ~RenderableVoxelWorld()
    {
        device.destroy_image(atlas_texture_array);
        device.destroy_sampler(atlas_sampler);
    }

    void draw(daxa::CommandList & cmd_list, glm::mat4 const & view_mat)
    {
        for (auto & chunk : chunks)
            chunk->draw(cmd_list, view_mat, atlas_texture_array, atlas_sampler);
        for (auto & chunk : chunks)
            chunk->draw_water(cmd_list, view_mat, atlas_texture_array, atlas_sampler);
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
            .mip_level_count = 4,
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

        // cmd_list.pipeline_barrier_image_transition({
        //     .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
        //     .waiting_pipeline_access = daxa::AccessConsts::ALL_GRAPHICS_READ,
        //     .before_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
        //     .after_layout = daxa::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
        //     .image_id = atlas_texture_array,
        //     .image_slice = {
        //         .base_mip_level = 0,
        //         .level_count = 1,
        //         .base_array_layer = 0,
        //         .layer_count = static_cast<u32>(texture_names.size()),
        //     },
        // });

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
        for (i32 zi = 0; zi < 32; ++zi)
        {
            for (i32 yi = 0; yi < 32; ++yi)
            {
                for (i32 xi = 0; xi < 32; ++xi)
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
        for (i32 zi = 0; zi < 32; ++zi)
        {
            for (i32 yi = 0; yi < 32; ++yi)
            {
                for (i32 xi = 0; xi < 32; ++xi)
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

    daxa::PipelineCompiler pipeline_compiler = device.create_pipeline_compiler({
        .root_paths = {
            "tests/0_common/shaders",
            "tests/3_samples/0_playground/shaders",
            "include",
        },
        .debug_name = "Playground Compiler",
    });

    // clang-format off
    daxa::RasterPipeline raster_pipeline = pipeline_compiler.create_raster_pipeline({
        .vertex_shader_info = {.source = daxa::ShaderFile{"vert.hlsl"}},
        .fragment_shader_info = {.source = daxa::ShaderFile{"frag.hlsl"}},
        .color_attachments = {{.format = swapchain.get_format(), .blend = {.blend_enable = true, .src_color_blend_factor = daxa::BlendFactor::SRC_ALPHA, .dst_color_blend_factor = daxa::BlendFactor::ONE_MINUS_SRC_ALPHA}}},
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
    // clang-format on

    daxa::ImGuiRenderer imgui_renderer = create_imgui_renderer();
    auto create_imgui_renderer() -> daxa::ImGuiRenderer
    {
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForVulkan(glfw_window_ptr, true);
        return daxa::ImGuiRenderer({
            .device = device,
            .pipeline_compiler = pipeline_compiler,
            .format = swapchain.get_format(),
        });
    }

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

    f32 render_scl = 1.0f;
    daxa::ImageId color_image, motion_vectors_image, depth_image;
    u32 render_size_x, render_size_y;

    daxa::ImageId swapchain_image;
    daxa::TaskImageId task_swapchain_image;
    daxa::TaskImageId task_color_image;
    daxa::TaskImageId task_depth_image;
    daxa::TaskList loop_task_list = record_loop_task_list();

    void create_render_images()
    {
        render_size_x = std::max<u32>(1, static_cast<u32>(static_cast<f32>(size_x) * render_scl));
        render_size_y = std::max<u32>(1, static_cast<u32>(static_cast<f32>(size_y) * render_scl));

        color_image = device.create_image({
            .format = swapchain.get_format(),
            .aspect = daxa::ImageAspectFlagBits::COLOR,
            .size = {render_size_x, render_size_y, 1},
            .usage = daxa::ImageUsageFlagBits::COLOR_ATTACHMENT,
        });
        motion_vectors_image = device.create_image({
            .format = daxa::Format::R16G16_SFLOAT,
            .aspect = daxa::ImageAspectFlagBits::COLOR,
            .size = {render_size_x, render_size_y, 1},
            .usage = daxa::ImageUsageFlagBits::COLOR_ATTACHMENT,
        });
        depth_image = device.create_image({
            .format = daxa::Format::D32_SFLOAT,
            .aspect = daxa::ImageAspectFlagBits::DEPTH,
            .size = {render_size_x, render_size_y, 1},
            .usage = daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT,
        });
    }
    void destroy_render_images()
    {
        device.destroy_image(color_image);
        device.destroy_image(motion_vectors_image);
        device.destroy_image(depth_image);
    }

    App() : AppWindow<App>("Samples: Playground")
    {
        create_render_images();
    }

    ~App()
    {
        ImGui_ImplGlfw_Shutdown();
        destroy_render_images();
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
        prev_time = now;

        ui_update();

        player.camera.resize(static_cast<i32>(size_x), static_cast<i32>(size_y));
        player.camera.set_pos(player.pos);
        player.camera.set_rot(player.rot.x, player.rot.y);
        player.update(elapsed_s);

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

        if (should_resize)
        {
            do_resize();
        }

        swapchain_image = swapchain.acquire_next_image();
        loop_task_list.execute();
        auto command_lists = loop_task_list.command_lists();
        auto cmd_list = device.create_command_list({});
        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = loop_task_list.last_access(task_swapchain_image),
            .before_layout = loop_task_list.last_layout(task_swapchain_image),
            .after_layout = daxa::ImageLayout::PRESENT_SRC,
            .image_id = swapchain_image,
        });
        cmd_list.complete();
        ++cpu_framecount;
        command_lists.push_back(cmd_list);
        device.submit_commands({
            .command_lists = command_lists,
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
        swapchain.resize(size_x, size_y);
        destroy_render_images();
        create_render_images();
        draw();
    }

    void toggle_pause()
    {
        set_mouse_capture(paused);
        paused = !paused;
    }

    void ui_update()
    {
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Debug");
        f32 new_scl = render_scl;
        ImGui::SliderFloat("Render Scl", &new_scl, 1.0f / static_cast<f32>(std::min(size_x, size_y)), 1.0f);
        if (new_scl != render_scl)
        {
            render_scl = new_scl;
            destroy_render_images();
            create_render_images();
        }
        ImGui::End();
        ImGui::Render();
    }

    auto record_loop_task_list() -> daxa::TaskList
    {
        daxa::TaskList new_task_list = daxa::TaskList({
            .device = device,
            .debug_name = "TaskList task list",
        });
        task_swapchain_image = new_task_list.create_task_image({
            .fetch_callback = [this]()
            { return swapchain_image; },
            .debug_name = "TaskList Task Swapchain Image",
        });
        task_color_image = new_task_list.create_task_image({
            .fetch_callback = [this]()
            { return color_image; },
            .debug_name = "TaskList Task Draw Color Image",
        });
        task_depth_image = new_task_list.create_task_image({
            .fetch_callback = [this]()
            { return depth_image; },
            .slice = {.image_aspect = daxa::ImageAspectFlagBits::DEPTH},
            .debug_name = "TaskList Task Draw Depth Image",
        });

        new_task_list.add_task({
            .resources = {
                .images = {
                    {task_color_image, daxa::TaskImageAccess::COLOR_ATTACHMENT},
                    {task_depth_image, daxa::TaskImageAccess::DEPTH_ATTACHMENT},
                },
            },
            .task = [this](daxa::TaskInterface interf)
            {
                auto cmd_list = interf.get_command_list();
                cmd_list.begin_renderpass({
                    .color_attachments = {{
                        .image_view = color_image.default_view(),
                        .load_op = daxa::AttachmentLoadOp::CLEAR,
                        .clear_value = std::array<f32, 4>{0.2f, 0.4f, 1.0f, 1.0f},
                    }},
                    .depth_attachment = {{
                        .image_view = depth_image.default_view(),
                        .load_op = daxa::AttachmentLoadOp::CLEAR,
                        .clear_value = daxa::DepthValue{1.0f, 0},
                    }},
                    .render_area = {.x = 0, .y = 0, .width = render_size_x, .height = render_size_y},
                });
                cmd_list.set_pipeline(raster_pipeline);
                renderable_world.draw(cmd_list, player.camera.get_vp());
                cmd_list.end_renderpass();
            },
            .debug_name = "TaskList Draw Task",
        });

        new_task_list.add_task({
            .resources = {
                .images = {
                    {task_color_image, daxa::TaskImageAccess::TRANSFER_READ},
                    {task_swapchain_image, daxa::TaskImageAccess::TRANSFER_WRITE},
                },
            },
            .task = [this](daxa::TaskInterface interf)
            {
                auto cmd_list = interf.get_command_list();
                cmd_list.blit_image_to_image({
                    .src_image = color_image,
                    .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
                    .dst_image = swapchain_image,
                    .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .src_slice = {.image_aspect = daxa::ImageAspectFlagBits::COLOR},
                    .src_offsets = {{{0, 0, 0}, {static_cast<i32>(render_size_x), static_cast<i32>(render_size_y), 1}}},
                    .dst_slice = {.image_aspect = daxa::ImageAspectFlagBits::COLOR},
                    .dst_offsets = {{{0, 0, 0}, {static_cast<i32>(size_x), static_cast<i32>(size_y), 1}}},
                });
            },
            .debug_name = "TaskList Blit Task",
        });

        new_task_list.add_task({
            .resources = {
                .images = {
                    {task_swapchain_image, daxa::TaskImageAccess::COLOR_ATTACHMENT},
                },
            },
            .task = [this](daxa::TaskInterface interf)
            {
                auto cmd_list = interf.get_command_list();
                imgui_renderer.record_commands(ImGui::GetDrawData(), cmd_list, swapchain_image, size_x, size_y);
            },
            .debug_name = "TaskList ImGui Task",
        });
        new_task_list.compile();
        return new_task_list;
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
