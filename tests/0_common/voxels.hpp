#pragma once

#include <daxa/daxa.hpp>
using namespace daxa::types;

#include <daxa/utils/math_operators.hpp>
using namespace daxa::math_operators;

static inline constexpr u64 CHUNK_SIZE = 32;
static inline constexpr u64 CHUNK_VOXEL_N = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
static inline constexpr u64 CHUNK_MAX_VERTS = CHUNK_VOXEL_N * 6;

static inline constexpr u64 CHUNK_N = 2;

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

static inline constexpr u64 CHUNK_MAX_SIZE = CHUNK_MAX_VERTS * sizeof(Vertex);

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
    f32vec3 pos;

    BlockID generate_block_id(f32vec3 p)
    {
        p = p * 0.5f;
        FractalNoiseConfig noise_conf = {
            /* .amplitude   = */ 1.0f,
            /* .persistance = */ 0.12f,
            /* .scale       = */ 0.03f,
            /* .lacunarity  = */ 4.7f,
            /* .octaves     = */ 2,
        };
        auto noise_p = p + 100.0f;
        f32 val = fractal_noise(noise_p, noise_conf);
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
                    f32vec3 block_pos = f32vec3{static_cast<f32>(xi), static_cast<f32>(yi), static_cast<f32>(zi)} + pos;
                    voxels[i].id = generate_block_id(block_pos);
                    if (voxels[i].id == BlockID::Stone)
                    {
                        u32 above_i;
                        for (above_i = 0; above_i < 6; ++above_i)
                        {
                            if (generate_block_id(block_pos - f32vec3{static_cast<f32>(0), static_cast<f32>(above_i + 1), static_cast<f32>(0)}) == BlockID::Air)
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
                            if (generate_block_id(block_pos + f32vec3{static_cast<f32>(0), static_cast<f32>(below_i + 1), static_cast<f32>(0)}) == BlockID::Stone)
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
        static_cast<i32>(image_info.size.x),
        static_cast<i32>(image_info.size.y),
        static_cast<i32>(image_info.size.z),
    };
    for (u32 i = 0; i < image_info.mip_level_count - 1; ++i)
    {
        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::BLIT_READ,
            .before_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .after_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
            .image_slice = {
                .image_aspect = image_info.aspect,
                .base_mip_level = i,
                .level_count = 1,
                .base_array_layer = info.base_array_layer,
                .layer_count = info.layer_count,
            },
            .image_id = info.image,
        });
        cmd_list.pipeline_barrier_image_transition({
            .waiting_pipeline_access = daxa::AccessConsts::BLIT_READ,
            .after_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .image_slice = {
                .image_aspect = image_info.aspect,
                .base_mip_level = i + 1,
                .level_count = 1,
                .base_array_layer = info.base_array_layer,
                .layer_count = info.layer_count,
            },
            .image_id = info.image,
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
            .after_layout = daxa::ImageLayout::READ_ONLY_OPTIMAL,
            .image_slice = {
                .image_aspect = image_info.aspect,
                .base_mip_level = i,
                .level_count = 1,
                .base_array_layer = info.base_array_layer,
                .layer_count = info.layer_count,
            },
            .image_id = info.image,
        });
    }
    cmd_list.pipeline_barrier_image_transition({
        .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_READ_WRITE,
        .waiting_pipeline_access = daxa::AccessConsts::READ_WRITE,
        .before_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
        .after_layout = daxa::ImageLayout::READ_ONLY_OPTIMAL,
        .image_slice = {
            .image_aspect = image_info.aspect,
            .base_mip_level = image_info.mip_level_count - 1,
            .level_count = 1,
            .base_array_layer = info.base_array_layer,
            .layer_count = info.layer_count,
        },
        .image_id = info.image,
    });
}

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
            .debug_name = APPNAME_PREFIX("face_buffer"),
        });
        water_face_buffer = device.create_buffer(daxa::BufferInfo{
            .size = CHUNK_MAX_SIZE,
            .debug_name = APPNAME_PREFIX("water_face_buffer"),
        });
    }

    ~RenderableChunk()
    {
        device.destroy_buffer(face_buffer);
        device.destroy_buffer(water_face_buffer);
    }

    void update_chunk_mesh(daxa::CommandList & cmd_list);

    void draw(daxa::CommandList & cmd_list, auto push)
    {
        if (face_n > 0)
        {
            push.chunk_pos = f32vec3{voxel_chunk.pos.x, voxel_chunk.pos.y, voxel_chunk.pos.z};
            push.face_buffer_id = face_buffer;
            push.mode = 0;
            cmd_list.push_constant(push);
            cmd_list.draw({.vertex_count = face_n * 6});
        }
    }
    void draw_water(daxa::CommandList & cmd_list, auto push)
    {
        if (water_face_n > 0)
        {
            push.chunk_pos = f32vec3{voxel_chunk.pos.x, voxel_chunk.pos.y, voxel_chunk.pos.z};
            push.face_buffer_id = face_buffer;
            push.mode = 1;
            cmd_list.push_constant(push);
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
                    chunk.voxel_chunk.pos = f32vec3{static_cast<f32>(x * CHUNK_SIZE), static_cast<f32>(y * CHUNK_SIZE), static_cast<f32>(z * CHUNK_SIZE)};
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
                        .debug_name = APPNAME_PREFIX("cmd_list (RenderableVoxelWorld update_chunk_mesh)"),
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
            .max_lod = 0,
            .debug_name = APPNAME_PREFIX("atlas_sampler"),
        });
    }

    ~RenderableVoxelWorld()
    {
        device.destroy_image(atlas_texture_array);
        device.destroy_sampler(atlas_sampler);
    }

    void draw(daxa::CommandList & cmd_list, auto push)
    {
        for (auto & chunk : chunks)
            chunk->draw(cmd_list, push);
        for (auto & chunk : chunks)
            chunk->draw_water(cmd_list, push);
    }

    Voxel get_voxel(i32vec3 p)
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
            .debug_name = APPNAME_PREFIX("atlas_texture_array"),
        });

        usize image_size = 16 * 16 * sizeof(u8) * 4;

        auto texture_staging_buffer = device.create_buffer({
            .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
            .size = static_cast<u32>(image_size * texture_names.size()),
            .debug_name = APPNAME_PREFIX("texture_staging_buffer"),
        });

        u8 * staging_buffer_data = device.map_memory_as<u8>(texture_staging_buffer);
        for (usize i = 0; i < texture_names.size(); ++i)
        {
            stbi_set_flip_vertically_on_load(true);
            auto path = filepath / texture_names[i];
            i32 size_x, size_y, num_channels;
            u8 * data = stbi_load(path.string().c_str(), &size_x, &size_y, &num_channels, 4);
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

        auto cmd_list = device.create_command_list({
            .debug_name = APPNAME_PREFIX("cmd_list"),
        });

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::HOST_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
        });

        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::HOST_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::UNDEFINED,
            .after_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .image_slice = {
                .base_mip_level = 0,
                .level_count = 1,
                .base_array_layer = 0,
                .layer_count = static_cast<u32>(texture_names.size()),
            },
            .image_id = atlas_texture_array,
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
        //     .after_layout = daxa::ImageLayout::READ_ONLY_OPTIMAL,
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
            .debug_name = APPNAME_PREFIX("face_staging_buffer"),
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
                    i32vec3 voxel_pos = i32vec3{xi, yi, zi} + i32vec3{static_cast<i32>(voxel_chunk.pos.x), static_cast<i32>(voxel_chunk.pos.y), static_cast<i32>(voxel_chunk.pos.z)};
                    ;
                    Voxel current_voxel = renderable_world->get_voxel(voxel_pos);
                    if (current_voxel.is_occluding())
                    {
                        if (!renderable_world->get_voxel(voxel_pos + i32vec3{+1, 0, 0}).is_occluding_nx())
                            *buffer_ptr = Vertex(xi, yi, zi, 0, current_voxel.id), ++buffer_ptr, ++face_n;
                        if (!renderable_world->get_voxel(voxel_pos + i32vec3{-1, 0, 0}).is_occluding_px())
                            *buffer_ptr = Vertex(xi, yi, zi, 1, current_voxel.id), ++buffer_ptr, ++face_n;
                        if (!renderable_world->get_voxel(voxel_pos + i32vec3{0, +1, 0}).is_occluding_ny())
                            *buffer_ptr = Vertex(xi, yi, zi, 2, current_voxel.id), ++buffer_ptr, ++face_n;
                        if (!renderable_world->get_voxel(voxel_pos + i32vec3{0, -1, 0}).is_occluding_py())
                            *buffer_ptr = Vertex(xi, yi, zi, 3, current_voxel.id), ++buffer_ptr, ++face_n;
                        if (!renderable_world->get_voxel(voxel_pos + i32vec3{0, 0, +1}).is_occluding_nz())
                            *buffer_ptr = Vertex(xi, yi, zi, 4, current_voxel.id), ++buffer_ptr, ++face_n;
                        if (!renderable_world->get_voxel(voxel_pos + i32vec3{0, 0, -1}).is_occluding_pz())
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
            .debug_name = APPNAME_PREFIX("face_staging_buffer (2)"),
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
                    i32vec3 voxel_pos = i32vec3{xi, yi, zi} + i32vec3{static_cast<i32>(voxel_chunk.pos.x), static_cast<i32>(voxel_chunk.pos.y), static_cast<i32>(voxel_chunk.pos.z)};
                    Voxel current_voxel = renderable_world->get_voxel(voxel_pos);
                    if (current_voxel.id == BlockID::Water)
                    {
                        if (renderable_world->get_voxel(voxel_pos + i32vec3{+1, 0, 0}).id == BlockID::Air)
                            *buffer_ptr = Vertex(xi, yi, zi, 0, current_voxel.id), ++buffer_ptr, ++water_face_n;
                        if (renderable_world->get_voxel(voxel_pos + i32vec3{-1, 0, 0}).id == BlockID::Air)
                            *buffer_ptr = Vertex(xi, yi, zi, 1, current_voxel.id), ++buffer_ptr, ++water_face_n;
                        if (renderable_world->get_voxel(voxel_pos + i32vec3{0, +1, 0}).id == BlockID::Air)
                            *buffer_ptr = Vertex(xi, yi, zi, 2, current_voxel.id), ++buffer_ptr, ++water_face_n;
                        if (renderable_world->get_voxel(voxel_pos + i32vec3{0, -1, 0}).id == BlockID::Air)
                            *buffer_ptr = Vertex(xi, yi, zi, 3, current_voxel.id), ++buffer_ptr, ++water_face_n;
                        if (renderable_world->get_voxel(voxel_pos + i32vec3{0, 0, +1}).id == BlockID::Air)
                            *buffer_ptr = Vertex(xi, yi, zi, 4, current_voxel.id), ++buffer_ptr, ++water_face_n;
                        if (renderable_world->get_voxel(voxel_pos + i32vec3{0, 0, -1}).id == BlockID::Air)
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
