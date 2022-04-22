#define GLM_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_VULKAN

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <fmt/format.h>
#include <Daxa.hpp>

#include <chrono>
#include <thread>

using namespace std::literals;

struct Window {
    GLFWwindow *window_ptr;
    glm::ivec2 frame_dim{1024, 1024};

    Window() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window_ptr = glfwCreateWindow(frame_dim.x, frame_dim.y, "title", nullptr, nullptr);
    }

    ~Window() {
        glfwDestroyWindow(window_ptr);
        glfwTerminate();
    }

    bool should_close() { return glfwWindowShouldClose(window_ptr); }
    void update() { glfwPollEvents(); }
    void swap_buffers() { glfwSwapBuffers(window_ptr); }
    VkSurfaceKHR get_vksurface(VkInstance vk_instance) {
        VkSurfaceKHR vulkan_surface;
        glfwCreateWindowSurface(vk_instance, window_ptr, nullptr, &vulkan_surface);
        return vulkan_surface;
    }

    template <typename T>
    void set_user_pointer(T *user_ptr) {
        glfwSetWindowUserPointer(window_ptr, user_ptr);
        glfwSetWindowSizeCallback(window_ptr, [](GLFWwindow *glfw_window_ptr, int size_x, int size_y) -> void {
            auto user_ptr = static_cast<T *>(glfwGetWindowUserPointer(glfw_window_ptr));
            if (user_ptr) {
                user_ptr->get_window().frame_dim = {size_x, size_y};
                user_ptr->on_resize();
            }
        });
        glfwSetCursorPosCallback(window_ptr, [](GLFWwindow *glfw_window_ptr, double mouse_x, double mouse_y) -> void {
            auto user_ptr = static_cast<T *>(glfwGetWindowUserPointer(glfw_window_ptr));
            if (user_ptr) {
                user_ptr->on_mouse_move(glm::dvec2{mouse_x, mouse_y});
            }
        });
        glfwSetScrollCallback(window_ptr, [](GLFWwindow *glfw_window_ptr, double offset_x, double offset_y) -> void {
            auto user_ptr = static_cast<T *>(glfwGetWindowUserPointer(glfw_window_ptr));
            if (user_ptr) {
                user_ptr->on_mouse_scroll(glm::dvec2{offset_x, offset_y});
            }
        });
        glfwSetMouseButtonCallback(window_ptr, [](GLFWwindow *glfw_window_ptr, int button, int action, int) -> void {
            auto user_ptr = static_cast<T *>(glfwGetWindowUserPointer(glfw_window_ptr));
            if (user_ptr) {
                user_ptr->on_mouse_button(button, action);
            }
        });
        glfwSetKeyCallback(window_ptr, [](GLFWwindow *glfw_window_ptr, int key, int, int action, int) -> void {
            auto user_ptr = static_cast<T *>(glfwGetWindowUserPointer(glfw_window_ptr));
            if (user_ptr) {
                user_ptr->on_key(key, action);
            }
        });
    }

    inline void set_mouse_capture(bool should_capture) {
        glfwSetCursorPos(window_ptr, static_cast<double>(frame_dim.x / 2), static_cast<double>(frame_dim.y / 2));
        glfwSetInputMode(window_ptr, GLFW_CURSOR, should_capture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        glfwSetInputMode(window_ptr, GLFW_RAW_MOUSE_MOTION, should_capture);
    }
    inline void set_mouse_pos(const glm::vec2 p) {
        glfwSetCursorPos(window_ptr, static_cast<f64>(p.x), static_cast<f64>(p.y));
    }
};

struct RenderContext {
    daxa::DeviceHandle device;
    daxa::PipelineCompilerHandle pipeline_compiler;
    daxa::CommandQueueHandle queue;

    daxa::SwapchainHandle swapchain;
    daxa::SwapchainImage swapchain_image;
    daxa::ImageViewHandle render_color_image, render_depth_image;

    struct PerFrameData {
        daxa::SignalHandle present_signal;
        daxa::TimelineSemaphoreHandle timeline;
        u64 timeline_counter = 0;
    };
    std::deque<PerFrameData> frames;

    auto create_color_image(glm::ivec2 dim) {
        auto result = device
                          ->createImageView(
                              {
                                  .image = device->createImage({
                                      .format = VK_FORMAT_R8G8B8A8_UNORM,
                                      .extent = {static_cast<u32>(dim.x), static_cast<u32>(dim.y), 1},
                                      .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
                                               VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                      .debugName = "Render Image",
                                  }),
                                  .format = VK_FORMAT_R8G8B8A8_UNORM,
                                  .subresourceRange =
                                      {
                                          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                          .baseMipLevel = 0,
                                          .levelCount = 1,
                                          .baseArrayLayer = 0,
                                          .layerCount = 1,
                                      },
                                  .debugName = "Render Image View",
                              });
        return result;
    }

    auto create_depth_image(glm::ivec2 dim) {
        auto result = device->createImageView({
            .image = device->createImage({
                .format = VK_FORMAT_D32_SFLOAT,
                .extent = {static_cast<u32>(dim.x), static_cast<u32>(dim.y), 1},
                .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                .debugName = "Depth Image",
            }),
            .format = VK_FORMAT_D32_SFLOAT,
            .subresourceRange =
                {
                    .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            .debugName = "Depth Image View",
        });
        return result;
    }

    RenderContext(VkSurfaceKHR surface, glm::ivec2 dim)
        : device(daxa::Device::create()),
          pipeline_compiler{this->device->createPipelineCompiler()},
          queue(device->createCommandQueue({.batchCount = 2})),
          swapchain(device->createSwapchain({
              .surface = surface,
              .width = static_cast<u32>(dim.x),
              .height = static_cast<u32>(dim.y),
              .presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR,
              .additionalUses = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
              .debugName = "Swapchain",
          })),
          swapchain_image(swapchain->aquireNextImage()),
          render_color_image(create_color_image(dim)),
          render_depth_image(create_depth_image(dim)) {
        for (int i = 0; i < 3; i++) {
            frames.push_back(PerFrameData{
                .present_signal = device->createSignal({}),
                .timeline = device->createTimelineSemaphore({}),
                .timeline_counter = 0,
            });
        }
        pipeline_compiler->addShaderSourceRootPath("Samples/Shadertoy/shaders");
        pipeline_compiler->addShaderSourceRootPath("DaxaMinecraft/assets/shaders");
    }

    ~RenderContext() {
        queue->waitIdle();
        queue->checkForFinishedSubmits();
        device->waitIdle();
        frames.clear();
    }

    auto begin_frame(glm::ivec2 dim) {
        resize(dim);
        auto cmd_list = queue->getCommandList({});
        cmd_list.queueImageBarrier(daxa::ImageBarrier{
            .barrier = daxa::FULL_MEMORY_BARRIER,
            .image = render_color_image,
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
        });
        cmd_list.queueImageBarrier(daxa::ImageBarrier{
            .barrier = daxa::FULL_MEMORY_BARRIER,
            .image = render_depth_image,
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        });
        return cmd_list;
    }

    void begin_rendering(daxa::CommandListHandle cmd_list) {
        std::array framebuffer{daxa::RenderAttachmentInfo{
            .image = swapchain_image.getImageViewHandle(),
            .clearValue = {.color = {.float32 = {1.0f, 0.0f, 1.0f, 1.0f}}},
        }};
        daxa::RenderAttachmentInfo depth_attachment{
            .image = render_depth_image,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .clearValue = {.depthStencil = {.depth = 1.0f}},
        };
        cmd_list.beginRendering(daxa::BeginRenderingInfo{
            .colorAttachments = framebuffer,
            .depthAttachment = &depth_attachment,
        });
    }

    void end_rendering(daxa::CommandListHandle cmd_list) {
        cmd_list.endRendering();
    }

    void end_frame(daxa::CommandListHandle cmd_list) {
        auto *current_frame = &frames.front();
        cmd_list.finalize();
        daxa::SubmitInfo submitInfo;
        submitInfo.commandLists.push_back(std::move(cmd_list));
        submitInfo.signalOnCompletion = {&current_frame->present_signal, 1};
        queue->submit(submitInfo);
        queue->present(std::move(swapchain_image), current_frame->present_signal);
        swapchain_image = swapchain->aquireNextImage();
        auto frameContext = std::move(frames.back());
        frames.pop_back();
        frames.push_front(std::move(frameContext));
        current_frame = &frames.front();
        queue->checkForFinishedSubmits();
        queue->nextBatch();
    }

    void resize(glm::ivec2 dim) {
        if (dim.x != static_cast<i32>(swapchain->getSize().width) ||
            dim.y != static_cast<i32>(swapchain->getSize().height)) {
            device->waitIdle();
            swapchain->resize(VkExtent2D{.width = static_cast<u32>(dim.x), .height = static_cast<u32>(dim.y)});
            swapchain_image = swapchain->aquireNextImage();
            render_color_image = create_color_image(dim);
            render_depth_image = create_depth_image(dim);
        }
    }

    void blit_to_swapchain(daxa::CommandListHandle cmd_list) {
        cmd_list.queueImageBarrier({
            .image = swapchain_image.getImageViewHandle(),
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        });
        cmd_list.queueImageBarrier({
            .image = render_color_image,
            .layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
            .layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        });

        auto render_extent = swapchain_image.getImageViewHandle()->getImageHandle()->getVkExtent3D();
        auto swap_extent = swapchain_image.getImageViewHandle()->getImageHandle()->getVkExtent3D();
        VkImageBlit blit{
            .srcSubresource =
                VkImageSubresourceLayers{
                    .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            .srcOffsets =
                {
                    VkOffset3D{0, 0, 0},
                    VkOffset3D{
                        static_cast<int32_t>(render_extent.width),
                        static_cast<int32_t>(render_extent.height),
                        static_cast<int32_t>(render_extent.depth),
                    },
                },
            .dstSubresource =
                VkImageSubresourceLayers{
                    .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            .dstOffsets =
                {
                    VkOffset3D{0, 0, 0},
                    VkOffset3D{
                        static_cast<int32_t>(swap_extent.width),
                        static_cast<int32_t>(swap_extent.height),
                        static_cast<int32_t>(swap_extent.depth),
                    },
                },
        };
        cmd_list.insertQueuedBarriers();
        vkCmdBlitImage(
            cmd_list.getVkCommandBuffer(),
            render_color_image->getImageHandle()->getVkImage(),
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            swapchain_image.getImageViewHandle()->getImageHandle()->getVkImage(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        cmd_list.queueImageBarrier({
            .image = swapchain_image.getImageViewHandle(),
            .layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .layoutAfter = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        });
        cmd_list.queueImageBarrier({
            .image = render_color_image,
            .layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
        });
    }
};

struct App {
    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point prev_frame_time;
    Clock::time_point start_time;

    Window window;

    VkSurfaceKHR vulkan_surface = window.get_vksurface(daxa::instance->getVkInstance());
    RenderContext render_context{vulkan_surface, window.frame_dim};

    std::array<float, 40> frametimes = {};
    u64 frametime_rotation_index = 0;

    struct ComputePipelineGlobals {
        glm::vec2 mouse_pos = {0, 0};
        glm::vec2 lmb_pos = {0, 0};
        glm::vec2 rmb_pos = {0, 0};
        glm::vec2 mmb_pos = {0, 0};
        glm::ivec2 frame_dim = {1, 1};
        float time = 0;
        bool lmb_pressed = false;
        bool rmb_pressed = false;
        bool mmb_pressed = false;

        void update_mb_positions() {
            if (lmb_pressed)
                lmb_pos = mouse_pos;
            if (rmb_pressed)
                rmb_pos = mouse_pos;
            if (mmb_pressed)
                mmb_pos = mouse_pos;
        }
    };
    daxa::PipelineHandle compute_pipeline;
    daxa::BufferHandle compute_pipeline_globals;

    daxa::PipelineHandle compute_buf0_pipeline;
    daxa::BufferHandle compute_buf0;

    ComputePipelineGlobals compute_globals_data{};

    void create_pipeline(daxa::PipelineHandle &pipe, const std::filesystem::path &path, const char *const entry) {
        auto result = render_context.pipeline_compiler->createComputePipeline({
            .shaderCI =
                {
                    .pathToSource = path,
                    .shaderLang = daxa::ShaderLang::HLSL,
                    .entryPoint = entry,
                    .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                },
            .overwriteSets = {daxa::BIND_ALL_SET_DESCRIPTION},
        });
        pipe = result.value();
    }

    void try_recompile(daxa::PipelineHandle &pipe) {
        if (render_context.pipeline_compiler->checkIfSourcesChanged(pipe)) {
            auto result = render_context.pipeline_compiler->recreatePipeline(pipe);
            std::cout << result << std::endl;
            if (result)
                pipe = result.value();
        }
    }

    App() {
        window.set_user_pointer<App>(this);

        create_pipeline(compute_pipeline, "Samples/Shadertoy/shaders/user_main.hlsl", "main");
        create_pipeline(compute_buf0_pipeline, "Samples/Shadertoy/shaders/user_buf0.hlsl", "buf0_main");

        compute_pipeline_globals = render_context.device->createBuffer({
            .size = sizeof(ComputePipelineGlobals),
            .memoryType = daxa::MemoryType::GPU_ONLY,
        });
        compute_buf0 = render_context.device->createBuffer({
            .size = sizeof(u32) * 512 * 256,
            .memoryType = daxa::MemoryType::GPU_ONLY,
        });

        start_time = Clock::now();

        // fnPtr_vkCmdSetCheckpointNV = (PFN_vkCmdSetCheckpointNV)vkGetDeviceProcAddr(render_context.device->getVkDevice(), "vkCmdSetCheckpointNV");
    }

    ~App() {
        render_context.queue->waitIdle();
        render_context.queue->checkForFinishedSubmits();
    }

    void update() {
        auto now = Clock::now();
        float dt = std::chrono::duration<float>(now - prev_frame_time).count();
        prev_frame_time = now;
        frametimes[frametime_rotation_index] = dt;
        frametime_rotation_index = (frametime_rotation_index + 1) % frametimes.size();
        float total_time_elapsed = std::chrono::duration<float>(now - start_time).count();

        window.update();

        if (window.frame_dim.x < 1 || window.frame_dim.y < 1) {
            std::this_thread::sleep_for(1ms);
            return;
        }

        try_recompile(compute_pipeline);
        try_recompile(compute_buf0_pipeline);

        auto extent = render_context.render_color_image->getImageHandle()->getVkExtent3D();

        auto cmd_list = render_context.begin_frame(window.frame_dim);

        compute_globals_data.frame_dim = {extent.width, extent.height};
        compute_globals_data.time = total_time_elapsed;

        auto compute_globals_id = compute_pipeline_globals.getDescriptorIndex();
        auto compute_buf0_id = compute_buf0.getDescriptorIndex();

        cmd_list.singleCopyHostToBuffer({
            .src = reinterpret_cast<u8 *>(&compute_globals_data),
            .dst = compute_pipeline_globals,
            .region = {.size = sizeof(decltype(compute_globals_data))},
        });
        cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);

        struct Push {
            u32 globals_id;
            u32 buf0_id;
            u32 output_image_id;
        };

        cmd_list.bindPipeline(compute_buf0_pipeline);
        cmd_list.bindAll();
        cmd_list.pushConstant(
            VK_SHADER_STAGE_COMPUTE_BIT,
            Push{
                .globals_id = compute_globals_id,
                .buf0_id = compute_buf0_id,
                .output_image_id = 0,
            });
        cmd_list.dispatch(32, 32);
        cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);
        cmd_list.pushConstant(
            VK_SHADER_STAGE_COMPUTE_BIT,
            Push{
                .globals_id = compute_globals_id,
                .buf0_id = compute_buf0_id,
                .output_image_id = 1,
            });
        cmd_list.dispatch(16, 16);
        cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);
        cmd_list.pushConstant(
            VK_SHADER_STAGE_COMPUTE_BIT,
            Push{
                .globals_id = compute_globals_id,
                .buf0_id = compute_buf0_id,
                .output_image_id = 2,
            });
        cmd_list.dispatch(8, 8);
        cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);
        cmd_list.pushConstant(
            VK_SHADER_STAGE_COMPUTE_BIT,
            Push{
                .globals_id = compute_globals_id,
                .buf0_id = compute_buf0_id,
                .output_image_id = 3,
            });
        cmd_list.dispatch(4, 4);
        cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);
        cmd_list.pushConstant(
            VK_SHADER_STAGE_COMPUTE_BIT,
            Push{
                .globals_id = compute_globals_id,
                .buf0_id = compute_buf0_id,
                .output_image_id = 4,
            });
        cmd_list.dispatch(2, 2);
        cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);
        cmd_list.pushConstant(
            VK_SHADER_STAGE_COMPUTE_BIT,
            Push{
                .globals_id = compute_globals_id,
                .buf0_id = compute_buf0_id,
                .output_image_id = 5,
            });
        cmd_list.dispatch(1, 1);
        cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);

        cmd_list.bindPipeline(compute_pipeline);
        cmd_list.bindAll();
        cmd_list.pushConstant(
            VK_SHADER_STAGE_COMPUTE_BIT,
            Push{
                .globals_id = compute_globals_id,
                .buf0_id = compute_buf0_id,
                .output_image_id = render_context.render_color_image->getDescriptorIndex(),
            });
        cmd_list.dispatch((extent.width + 7) / 8, (extent.height + 7) / 8);

        render_context.blit_to_swapchain(cmd_list);
        render_context.end_frame(cmd_list);
    }

    Window &get_window() { return window; }
    void on_mouse_move(const glm::dvec2 m) {
        compute_globals_data.mouse_pos = m;
        compute_globals_data.update_mb_positions();
    }
    void on_mouse_scroll(const glm::dvec2 /* offset */) {}
    void on_mouse_button(int button, int action) {
        switch (button) {
        case GLFW_MOUSE_BUTTON_1: compute_globals_data.lmb_pressed = action != GLFW_RELEASE; break;
        case GLFW_MOUSE_BUTTON_2: compute_globals_data.rmb_pressed = action != GLFW_RELEASE; break;
        case GLFW_MOUSE_BUTTON_3: compute_globals_data.mmb_pressed = action != GLFW_RELEASE; break;
        }
        compute_globals_data.update_mb_positions();
    }
    void on_key(int /* key */, int /* action */) {}
    void on_resize() { update(); }
};

int main() {
    daxa::initialize();
    {
        App app;
        while (true) {
            app.update();
            if (app.window.should_close())
                break;
        }
    }
    daxa::cleanup();
}
