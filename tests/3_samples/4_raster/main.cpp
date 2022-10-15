#define DAXA_SHADERLANG DAXA_SHADERLANG_GLSL
#define APPNAME "Daxa Sample: Raster"
#include <0_common/base_app.hpp>

#include <0_common/player.hpp>
#include <0_common/noise.hpp>

using namespace daxa::types;
#include "shaders/shared.inl"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using BlockID = u32;
struct VoxelFace
{
    u32 data;

    VoxelFace(i32 x, i32 y, i32 z, u32 side, BlockID id)
    {
        data = 0;
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

    bool is_occluding()
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
    bool is_cross()
    {
        switch (id)
        {
        case BlockID_Rose:
        case BlockID_TallGrass: return true;
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

static inline constexpr u64 CHUNK_SIZE = 32;
static inline constexpr u64 CHUNK_VOXEL_N = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
static inline constexpr u64 WORLD_SIZE = 2;
static inline constexpr u64 CHUNK_N = WORLD_SIZE * WORLD_SIZE * WORLD_SIZE;
static inline constexpr u64 CHUNK_MAX_VERTS = CHUNK_VOXEL_N * 6;
static inline constexpr u64 CHUNK_MAX_SIZE = CHUNK_MAX_VERTS * sizeof(VoxelFace);

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

struct VoxelChunk
{
    Voxel voxels[CHUNK_VOXEL_N];
    f32vec3 pos;

    BlockID generate_block_id(f32vec3 p)
    {
        FractalNoiseConfig noise_conf{
            .amplitude = 1.0f,
            .persistance = 0.5f,
            .scale = 0.04f,
            .lacunarity = 2.0f,
            .octaves = 4,
        };
        f32 value = fractal_noise(p, noise_conf);
        if (value < 0.0f)
        {
            if (value < -0.2f)
                return BlockID_Gravel;
            else if (value < -0.1f)
                return BlockID_Stone;
            else
                return BlockID_Dirt;
        }
        return BlockID_Air;
    }
    void generate()
    {
        for (u64 zi = 0; zi < CHUNK_SIZE; ++zi)
        {
            for (u64 yi = 0; yi < CHUNK_SIZE; ++yi)
            {
                for (u64 xi = 0; xi < CHUNK_SIZE; ++xi)
                {
                    u64 i = xi + yi * CHUNK_SIZE + zi * CHUNK_SIZE * CHUNK_SIZE;
                    f32vec3 block_pos = f32vec3{static_cast<f32>(xi), static_cast<f32>(yi), static_cast<f32>(zi)} + pos;
                    voxels[i].id = generate_block_id(block_pos);
                }
            }
        }
    }
};
struct RenderableChunk
{
    std::unique_ptr<VoxelChunk> chunk = std::make_unique<VoxelChunk>();
    daxa::BufferId face_buffer;
    daxa::TaskBufferId task_face_buffer;
    daxa::BufferId water_face_buffer;
    daxa::TaskBufferId task_water_face_buffer;
    u32 face_n = 0, water_face_n = 0;
    bool invalid = true;
};
struct RenderableVoxelWorld
{
    daxa::Device device;
    std::array<RenderableChunk, CHUNK_N> renderable_chunks{};

    daxa::ImageId atlas_texture_array;
    daxa::SamplerId atlas_sampler;
    daxa::TaskImageId task_atlas_texture_array;
    bool textures_valid = false;

    RenderableVoxelWorld(daxa::Device a_device) : device{a_device}
    {
        for (u64 z = 0; z < WORLD_SIZE; ++z)
        {
            for (u64 y = 0; y < WORLD_SIZE; ++y)
            {
                for (u64 x = 0; x < WORLD_SIZE; ++x)
                {
                    auto & renderable_chunk = renderable_chunks[x + y * WORLD_SIZE + z * WORLD_SIZE * WORLD_SIZE];
                    renderable_chunk.chunk->pos = f32vec3{static_cast<f32>(x * CHUNK_SIZE), static_cast<f32>(y * CHUNK_SIZE), static_cast<f32>(z * CHUNK_SIZE)};
                    renderable_chunk.face_buffer = device.create_buffer(daxa::BufferInfo{
                        .size = CHUNK_MAX_SIZE,
                        .debug_name = APPNAME_PREFIX("face_buffer"),
                    });
                    renderable_chunk.water_face_buffer = device.create_buffer(daxa::BufferInfo{
                        .size = CHUNK_MAX_SIZE,
                        .debug_name = APPNAME_PREFIX("water_face_buffer"),
                    });
                    renderable_chunk.chunk->generate();
                }
            }
        }

        atlas_texture_array = device.create_image({
            .format = daxa::Format::R8G8B8A8_SRGB,
            .size = {16, 16, 1},
            .mip_level_count = 4,
            .array_layer_count = static_cast<u32>(texture_names.size()),
            .usage = daxa::ImageUsageFlagBits::SHADER_READ_ONLY | daxa::ImageUsageFlagBits::TRANSFER_SRC | daxa::ImageUsageFlagBits::TRANSFER_DST,
            .debug_name = APPNAME_PREFIX("atlas_texture_array"),
        });

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
        for (auto & renderable_chunk : renderable_chunks)
        {
            device.destroy_buffer(renderable_chunk.face_buffer);
            device.destroy_buffer(renderable_chunk.water_face_buffer);
        }
        device.destroy_image(atlas_texture_array);
        device.destroy_sampler(atlas_sampler);
    }

    Voxel get_voxel(i32vec3 p)
    {
        i32 x = p.x / static_cast<i32>(CHUNK_SIZE);
        i32 y = p.y / static_cast<i32>(CHUNK_SIZE);
        i32 z = p.z / static_cast<i32>(CHUNK_SIZE);

        if (p.x < 0 || x >= static_cast<i32>(WORLD_SIZE))
            return {.id = BlockID_Air};
        if (p.y < 0 || y >= static_cast<i32>(WORLD_SIZE))
            return {.id = BlockID_Air};
        if (p.z < 0 || z >= static_cast<i32>(WORLD_SIZE))
            return {.id = BlockID_Air};

        auto & chunk = renderable_chunks[static_cast<u64>(x) + static_cast<u64>(y) * WORLD_SIZE + static_cast<u64>(z) * WORLD_SIZE * WORLD_SIZE];

        u64 xi = static_cast<u64>(p.x) % CHUNK_SIZE;
        u64 yi = static_cast<u64>(p.y) % CHUNK_SIZE;
        u64 zi = static_cast<u64>(p.z) % CHUNK_SIZE;
        u64 i = xi + yi * CHUNK_SIZE + zi * CHUNK_SIZE * CHUNK_SIZE;
        return chunk.chunk->voxels[i];
    }
    void construct_chunk_meshes(RenderableChunk & renderable_chunk, VoxelFace * buffer_ptr, VoxelFace * water_buffer_ptr)
    {
        auto & chunk_pos = renderable_chunk.chunk->pos;
        auto & face_n = renderable_chunk.face_n;
        auto & water_face_n = renderable_chunk.water_face_n;

        face_n = 0;
        for (i32 zi = 0; zi < 32; ++zi)
        {
            for (i32 yi = 0; yi < 32; ++yi)
            {
                for (i32 xi = 0; xi < 32; ++xi)
                {
                    i32vec3 voxel_pos = i32vec3{xi, yi, zi} + i32vec3{static_cast<i32>(chunk_pos.x), static_cast<i32>(chunk_pos.y), static_cast<i32>(chunk_pos.z)};
                    Voxel current_voxel = get_voxel(voxel_pos);
                    if (current_voxel.is_occluding())
                    {
                        if (!get_voxel(voxel_pos + i32vec3{+1, 0, 0}).is_occluding_nx())
                            *buffer_ptr = VoxelFace(xi, yi, zi, 0, current_voxel.id), ++buffer_ptr, ++face_n;
                        if (!get_voxel(voxel_pos + i32vec3{-1, 0, 0}).is_occluding_px())
                            *buffer_ptr = VoxelFace(xi, yi, zi, 1, current_voxel.id), ++buffer_ptr, ++face_n;
                        if (!get_voxel(voxel_pos + i32vec3{0, +1, 0}).is_occluding_ny())
                            *buffer_ptr = VoxelFace(xi, yi, zi, 2, current_voxel.id), ++buffer_ptr, ++face_n;
                        if (!get_voxel(voxel_pos + i32vec3{0, -1, 0}).is_occluding_py())
                            *buffer_ptr = VoxelFace(xi, yi, zi, 3, current_voxel.id), ++buffer_ptr, ++face_n;
                        if (!get_voxel(voxel_pos + i32vec3{0, 0, +1}).is_occluding_nz())
                            *buffer_ptr = VoxelFace(xi, yi, zi, 4, current_voxel.id), ++buffer_ptr, ++face_n;
                        if (!get_voxel(voxel_pos + i32vec3{0, 0, -1}).is_occluding_pz())
                            *buffer_ptr = VoxelFace(xi, yi, zi, 5, current_voxel.id), ++buffer_ptr, ++face_n;
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

        water_face_n = 0;
        for (i32 zi = 0; zi < static_cast<i32>(CHUNK_SIZE); ++zi)
        {
            for (i32 yi = 0; yi < static_cast<i32>(CHUNK_SIZE); ++yi)
            {
                for (i32 xi = 0; xi < static_cast<i32>(CHUNK_SIZE); ++xi)
                {
                    i32vec3 voxel_pos = i32vec3{xi, yi, zi} + i32vec3{static_cast<i32>(chunk_pos.x), static_cast<i32>(chunk_pos.y), static_cast<i32>(chunk_pos.z)};
                    Voxel current_voxel = get_voxel(voxel_pos);
                    if (current_voxel.id == BlockID_Water)
                    {
                        if (get_voxel(voxel_pos + i32vec3{+1, 0, 0}).id == BlockID_Air)
                            *water_buffer_ptr = VoxelFace(xi, yi, zi, 0, current_voxel.id), ++water_buffer_ptr, ++water_face_n;
                        if (get_voxel(voxel_pos + i32vec3{-1, 0, 0}).id == BlockID_Air)
                            *water_buffer_ptr = VoxelFace(xi, yi, zi, 1, current_voxel.id), ++water_buffer_ptr, ++water_face_n;
                        if (get_voxel(voxel_pos + i32vec3{0, +1, 0}).id == BlockID_Air)
                            *water_buffer_ptr = VoxelFace(xi, yi, zi, 2, current_voxel.id), ++water_buffer_ptr, ++water_face_n;
                        if (get_voxel(voxel_pos + i32vec3{0, -1, 0}).id == BlockID_Air)
                            *water_buffer_ptr = VoxelFace(xi, yi, zi, 3, current_voxel.id), ++water_buffer_ptr, ++water_face_n;
                        if (get_voxel(voxel_pos + i32vec3{0, 0, +1}).id == BlockID_Air)
                            *water_buffer_ptr = VoxelFace(xi, yi, zi, 4, current_voxel.id), ++water_buffer_ptr, ++water_face_n;
                        if (get_voxel(voxel_pos + i32vec3{0, 0, -1}).id == BlockID_Air)
                            *water_buffer_ptr = VoxelFace(xi, yi, zi, 5, current_voxel.id), ++water_buffer_ptr, ++water_face_n;
                    }
                }
            }
        }
    }

    void record_load_textures_tasks(daxa::TaskList & new_task_list)
    {
        task_atlas_texture_array = new_task_list.create_task_image({.image = &atlas_texture_array, .debug_name = APPNAME_PREFIX("task_atlas_texture_array")});

        new_task_list.add_task({
            .used_images = {
                {task_atlas_texture_array, daxa::TaskImageAccess::TRANSFER_WRITE, daxa::ImageMipArraySlice{.base_array_layer = 0, .layer_count = static_cast<u32>(texture_names.size())}},
            },
            .task = [this](daxa::TaskRuntime runtime)
            {
                if (!textures_valid)
                {
                    auto cmd_list = runtime.get_command_list();
                    usize image_size = 16 * 16 * sizeof(u8) * 4;
                    auto texture_staging_buffer = device.create_buffer({
                        .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                        .size = static_cast<u32>(image_size * texture_names.size()),
                        .debug_name = APPNAME_PREFIX("texture_staging_buffer"),
                    });
                    cmd_list.destroy_buffer_deferred(texture_staging_buffer);
                    u8 * staging_buffer_data = device.map_memory_as<u8>(texture_staging_buffer);
                    for (usize i = 0; i < texture_names.size(); ++i)
                    {
                        stbi_set_flip_vertically_on_load(true);
                        auto path = textures_filepath / texture_names[i];
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
                    device.unmap_memory(texture_staging_buffer);
                }
            },
            .debug_name = APPNAME_PREFIX("Upload Textures"),
        });

        auto image_info = device.info_image(atlas_texture_array);
        std::array<i32, 3> mip_size = {static_cast<i32>(image_info.size[0]), static_cast<i32>(image_info.size[1]), static_cast<i32>(image_info.size[2])};
        for (u32 i = 0; i < image_info.mip_level_count - 1; ++i)
        {
            std::array<i32, 3> next_mip_size = {std::max<i32>(1, mip_size[0] / 2), std::max<i32>(1, mip_size[1] / 2), std::max<i32>(1, mip_size[2] / 2)};
            new_task_list.add_task({
                .used_images = {
                    {task_atlas_texture_array, daxa::TaskImageAccess::TRANSFER_WRITE, daxa::ImageMipArraySlice{.base_mip_level = i + 0, .base_array_layer = 0, .layer_count = static_cast<u32>(texture_names.size())}},
                    {task_atlas_texture_array, daxa::TaskImageAccess::TRANSFER_WRITE, daxa::ImageMipArraySlice{.base_mip_level = i + 1, .base_array_layer = 0, .layer_count = static_cast<u32>(texture_names.size())}},
                },
                .task = [=, this](daxa::TaskRuntime const & runtime)
                {
                    auto cmd_list = runtime.get_command_list();
                    auto image_id = runtime.get_image(task_atlas_texture_array);
                    cmd_list.blit_image_to_image({
                        .src_image = image_id,
                        .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL, // TODO: get from TaskRuntime
                        .dst_image = image_id,
                        .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                        .src_slice = {
                            .image_aspect = image_info.aspect,
                            .mip_level = i,
                            .base_array_layer = 0,
                            .layer_count = 1,
                        },
                        .src_offsets = {{{0, 0, 0}, {mip_size[0], mip_size[1], mip_size[2]}}},
                        .dst_slice = {
                            .image_aspect = image_info.aspect,
                            .mip_level = i + 1,
                            .base_array_layer = 0,
                            .layer_count = 1,
                        },
                        .dst_offsets = {{{0, 0, 0}, {next_mip_size[0], next_mip_size[1], next_mip_size[2]}}},
                        .filter = daxa::Filter::LINEAR,
                    });
                },
                .debug_name = APPNAME_PREFIX("Generate Texture Mips ") + std::to_string(i),
            });
            mip_size = next_mip_size;
        }
    }
};

struct App : BaseApp<App>
{
    // clang-format off
    daxa::RasterPipeline raster_pipeline = pipeline_compiler.create_raster_pipeline({
        .vertex_shader_info = {.source = daxa::ShaderFile{"draw.glsl"}, .compile_options = {.defines = {daxa::ShaderDefine{"DRAW_VERT"}}}},
        .fragment_shader_info = {.source = daxa::ShaderFile{"draw.glsl"}, .compile_options = {.defines = {daxa::ShaderDefine{"DRAW_FRAG"}}}},
        .color_attachments = {{.format = swapchain.get_format()}},
        .depth_test = {
            .depth_attachment_format = daxa::Format::D24_UNORM_S8_UINT,
            .enable_depth_test = true,
            .enable_depth_write = true,
        },
        .raster = {
            .polygon_mode = daxa::PolygonMode::FILL,
            .face_culling = daxa::FaceCullFlagBits::BACK_BIT,
        },
        .push_constant_size = sizeof(DrawPush),
        .debug_name = APPNAME_PREFIX("raster_pipeline"),
    }).value();
    // clang-format on

    daxa::ImageId depth_image = device.create_image({
        .format = daxa::Format::D24_UNORM_S8_UINT,
        .aspect = daxa::ImageAspectFlagBits::DEPTH | daxa::ImageAspectFlagBits::STENCIL,
        .size = {size_x, size_y, 1},
        .usage = daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT,
    });
    daxa::TaskImageId task_depth_image;

    RenderableVoxelWorld renderable_world = RenderableVoxelWorld(device);

    Player3D player = {
        .pos = {10.0f, 10.0f, 10.0f},
        .rot = {0.75f * std::numbers::pi_v<f32>, -0.5f, 0.0f},
    };
    bool paused = true;

    daxa::TaskList loop_task_list = record_loop_task_list();

    ~App()
    {
        device.wait_idle();
        device.collect_garbage();
        device.destroy_image(depth_image);
    }

    void ui_update()
    {
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        if (paused)
        {
            ImGui::Begin("Debug");
            ImGui::End();
            ImGui::ShowDemoWindow();
        }
        ImGui::Render();
    }
    void on_update()
    {
        reload_pipeline(raster_pipeline);
        ui_update();

        player.camera.resize(static_cast<i32>(size_x), static_cast<i32>(size_y));
        player.camera.set_pos(player.pos);
        player.camera.set_rot(player.rot.x, player.rot.y);
        player.update(delta_time);

        swapchain_image = swapchain.acquire_next_image();
        if (swapchain_image.is_empty())
            return;
        loop_task_list.execute();
    }

    void on_mouse_move(f32 x, f32 y)
    {
        if (!paused)
        {
            f32 center_x = static_cast<f32>(size_x / 2);
            f32 center_y = static_cast<f32>(size_y / 2);
            auto offset = f32vec2{x - center_x, center_y - y};
            player.on_mouse_move(offset.x, offset.y);
            set_mouse_pos(center_x, center_y);
        }
    }
    void on_mouse_button(i32, i32) {}
    void on_key(i32 key_id, i32 action)
    {
        auto & io = ImGui::GetIO();
        if (io.WantCaptureKeyboard)
            return;

        if (key_id == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            toggle_pause();

        if (!paused)
        {
            player.on_key(key_id, action);
        }
    }
    void on_resize(u32 sx, u32 sy)
    {
        minimized = (sx == 0 || sy == 0);
        if (!minimized)
        {
            swapchain.resize();
            size_x = swapchain.info().width;
            size_y = swapchain.info().height;
            device.destroy_image(depth_image);
            depth_image = device.create_image({
                .format = daxa::Format::D24_UNORM_S8_UINT,
                .aspect = daxa::ImageAspectFlagBits::DEPTH | daxa::ImageAspectFlagBits::STENCIL,
                .size = {size_x, size_y, 1},
                .usage = daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT,
            });
            base_on_update();
        }
    }

    void toggle_pause()
    {
        set_mouse_capture(paused);
        paused = !paused;
    }

    void record_tasks(daxa::TaskList & new_task_list)
    {
        task_depth_image = new_task_list.create_task_image({.image = &depth_image, .debug_name = APPNAME_PREFIX("task_depth_image")});
        renderable_world.record_load_textures_tasks(new_task_list);
        std::vector<daxa::TaskBufferUse> upload_used_buffers;
        std::vector<daxa::TaskBufferUse> draw_used_buffers;
        for (auto & chunk : renderable_world.renderable_chunks)
        {
            chunk.task_face_buffer = new_task_list.create_task_buffer({.buffer = &chunk.face_buffer, .debug_name = APPNAME_PREFIX("chunk.task_face_buffer")});
            chunk.task_water_face_buffer = new_task_list.create_task_buffer({.buffer = &chunk.water_face_buffer, .debug_name = APPNAME_PREFIX("chunk.task_water_face_buffer")});
            upload_used_buffers.push_back({chunk.task_face_buffer, daxa::TaskBufferAccess::TRANSFER_WRITE});
            upload_used_buffers.push_back({chunk.task_water_face_buffer, daxa::TaskBufferAccess::TRANSFER_WRITE});
            draw_used_buffers.push_back({chunk.task_face_buffer, daxa::TaskBufferAccess::VERTEX_SHADER_READ_ONLY});
            draw_used_buffers.push_back({chunk.task_water_face_buffer, daxa::TaskBufferAccess::VERTEX_SHADER_READ_ONLY});
        }
        new_task_list.add_task({
            .used_buffers = upload_used_buffers,
            .task = [this](daxa::TaskRuntime runtime)
            {
                auto cmd_list = runtime.get_command_list();
                auto face_staging_buffer = device.create_buffer({
                    .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                    .size = static_cast<u32>(CHUNK_MAX_SIZE * renderable_world.renderable_chunks.size()),
                    .debug_name = APPNAME_PREFIX("face_staging_buffer"),
                });
                auto water_face_staging_buffer = device.create_buffer({
                    .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                    .size = static_cast<u32>(CHUNK_MAX_SIZE * renderable_world.renderable_chunks.size()),
                    .debug_name = APPNAME_PREFIX("water_face_staging_buffer"),
                });
                u32 i = 0;
                cmd_list.destroy_buffer_deferred(face_staging_buffer);
                cmd_list.destroy_buffer_deferred(water_face_staging_buffer);
                auto face_buffer_ptr = device.map_memory_as<VoxelFace>(face_staging_buffer);
                auto water_face_buffer_ptr = device.map_memory_as<VoxelFace>(water_face_staging_buffer);
                for (auto & chunk : renderable_world.renderable_chunks)
                {
                    if (chunk.invalid)
                    {
                        renderable_world.construct_chunk_meshes(chunk, face_buffer_ptr + CHUNK_MAX_VERTS * i, water_face_buffer_ptr + CHUNK_MAX_VERTS * i);
                        cmd_list.copy_buffer_to_buffer({
                            .src_buffer = face_staging_buffer,
                            .src_offset = CHUNK_MAX_SIZE * i,
                            .dst_buffer = chunk.face_buffer,
                            .size = sizeof(VoxelFace) * chunk.face_n,
                        });
                        cmd_list.copy_buffer_to_buffer({
                            .src_buffer = water_face_staging_buffer,
                            .src_offset = CHUNK_MAX_SIZE * i,
                            .dst_buffer = chunk.face_buffer,
                            .size = sizeof(VoxelFace) * chunk.water_face_n,
                        });
                        chunk.invalid = false;
                    }
                    ++i;
                }
                device.unmap_memory(face_staging_buffer);
                device.unmap_memory(water_face_staging_buffer);
            },
            .debug_name = APPNAME_PREFIX("Upload Chunks"),
        });
        auto atlas_image_info = device.info_image(renderable_world.atlas_texture_array);
        new_task_list.add_task({
            .used_buffers = draw_used_buffers,
            .used_images = {
                {task_swapchain_image, daxa::TaskImageAccess::COLOR_ATTACHMENT, daxa::ImageMipArraySlice{}},
                {task_depth_image, daxa::TaskImageAccess::DEPTH_ATTACHMENT, daxa::ImageMipArraySlice{}},
                {renderable_world.task_atlas_texture_array, daxa::TaskImageAccess::TRANSFER_WRITE, daxa::ImageMipArraySlice{.base_mip_level = 0, .level_count = atlas_image_info.mip_level_count, .base_array_layer = 0, .layer_count = static_cast<u32>(texture_names.size())}},
            },
            .task = [this](daxa::TaskRuntime runtime)
            {
                auto cmd_list = runtime.get_command_list();
                cmd_list.begin_renderpass({
                    .color_attachments = {{
                        .image_view = swapchain_image.default_view(),
                        .load_op = daxa::AttachmentLoadOp::CLEAR,
                        .clear_value = std::array<f32, 4>{0.2f, 0.4f, 1.0f, 1.0f},
                    }},
                    .depth_attachment = {{
                        .image_view = depth_image.default_view(),
                        .load_op = daxa::AttachmentLoadOp::CLEAR,
                        .clear_value = daxa::DepthValue{1.0f, 0},
                    }},
                    .render_area = {.x = 0, .y = 0, .width = size_x, .height = size_y},
                });
                auto mat = player.camera.get_vp();
                cmd_list.set_pipeline(raster_pipeline);
                for (auto & chunk : renderable_world.renderable_chunks)
                {
                    if (!chunk.invalid)
                    {
                        cmd_list.push_constant(DrawPush{
                            .vp_mat = *reinterpret_cast<f32mat4x4 *>(&mat),
                            .chunk_pos = chunk.chunk->pos,
                            .face_buffer = this->device.buffer_reference(chunk.face_buffer),
                            .atlas_texture = this->renderable_world.atlas_texture_array.default_view(),
                            .atlas_sampler = this->renderable_world.atlas_sampler,
                        });
                        cmd_list.draw({.vertex_count = chunk.face_n * 6});
                    }
                }
                cmd_list.end_renderpass();
            },
            .debug_name = APPNAME_PREFIX("Draw to swapchain"),
        });
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
