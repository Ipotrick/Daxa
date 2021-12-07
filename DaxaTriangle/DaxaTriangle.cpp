#include "Daxa.hpp"

#include <iostream>

using namespace daxa;

struct PerFrameData {
    gpu::SignalHandle presentSignal;
    gpu::TimelineSemaphore timeline;
    u64 timelineCounter = 0;
};

class MyUser : public daxa::User {
    //std::unique_ptr<daxa::Renderer> defaultRenderer;
    gpu::Device device;
    gpu::Queue queue;
    std::deque<PerFrameData> frames;
    gpu::RenderWindow renderWindow;
    gpu::GraphicsPipelineHandle pipeline;
    gpu::BindingSetAllocator setAllocator;
    gpu::BufferHandle vertexBuffer;
    gpu::BufferHandle uniformBuffer;
    double totalElapsedTime = 0.0f;
    gpu::SwapchainImage swapchainImage;
public:
    MyUser() = default;
    MyUser(MyUser&&) noexcept = delete;
    MyUser& operator=(MyUser&&) noexcept = delete;

    virtual void init(std::shared_ptr<daxa::AppState> appstate) override {
        device = gpu::Device::create();
        queue = device.createQueue();

        for (int i = 0; i < 3; i++) {
            frames.push_back(PerFrameData{.presentSignal = device.createSignal(), .timeline = device.createTimelineSemaphore()});
        }
        renderWindow = device.createRenderWindow(appstate->window->getWindowHandleSDL(), appstate->window->getSize()[0], appstate->window->getSize()[1]);

        gpu::ShaderModuleHandle vertexShader = device.tryCreateShderModuleFromFile(
            "daxa/shaders/test.vert",
            VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT
        ).value();

        gpu::ShaderModuleHandle fragmenstShader = device.tryCreateShderModuleFromFile(
            "daxa/shaders/test.frag",
            VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT
        ).value();

        VkPipelineVertexInputStateCreateInfo d;
        gpu::GraphicsPipelineBuilder pipelineBuilder;
        pipelineBuilder.addShaderStage(vertexShader);
        pipelineBuilder.addShaderStage(fragmenstShader);
        pipelineBuilder.beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX);	// add vertex attributes:
        pipelineBuilder.addVertexInputAttribute(VK_FORMAT_R32G32B32_SFLOAT);			// positions
        pipelineBuilder.addVertexInputAttribute(VK_FORMAT_R32G32B32A32_SFLOAT);			// colors
        pipelineBuilder.addColorAttachment(renderWindow.getVkFormat());

        pipeline = device.createGraphicsPipeline(pipelineBuilder);

        setAllocator = device.createBindingSetAllocator(pipeline->getSetDescription(0));

        constexpr size_t vertexBufferSize = sizeof(float) * 3 * 3 /* positions */ + sizeof(float) * 4 * 3 /* colors */;
        gpu::BufferCreateInfo bufferCI{
            .size = vertexBufferSize,
            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,	
            .memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
        };
        vertexBuffer = device.createBuffer(bufferCI);

        gpu::BufferCreateInfo someBufferCI{
            .size = sizeof(float) * 4,
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
        };
        uniformBuffer = device.createBuffer(someBufferCI);

        swapchainImage = renderWindow.aquireNextImage();
    }

    virtual void update(std::shared_ptr<daxa::AppState> appstate) override {
        std::cout << "delta time: " << std::setw(8) << appstate->getDeltaTimeSeconds() * 1000.0f << "ms, fps: " << 1.0f / (appstate->getDeltaTimeSeconds()) << std::endl;

        if (appstate->window->getSize()[0] != renderWindow.getSize().width || appstate->window->getSize()[1] != renderWindow.getSize().height) {
            device.waitIdle();
            renderWindow.resize(VkExtent2D{ .width = appstate->window->getSize()[0], .height = appstate->window->getSize()[1] });
        }

        auto* currentFrame = &frames.front();

        auto cmdList = device.getEmptyCommandList();

        cmdList.begin();

        std::array vertecies = {
             1.f, 1.f, 0.0f,		1.f, 0.f, 0.f, 1.f,
            -1.f, 1.f, 0.0f,		0.f, 1.f, 0.f, 1.f,
             0.f,-1.f, 0.0f,		0.f, 0.f, 1.f, 1.f,
        };
        cmdList.uploadToBuffer(vertecies, vertexBuffer);
        std::array someBufferdata = { 1.0f , 1.0f , 1.0f ,1.0f };
        cmdList.uploadToBuffer(someBufferdata, uniformBuffer);

        std::array imgBarrier0 = { gpu::ImageBarrier{
            .waitingStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,	// as we write to the image in the frag shader we need to make sure its finished transitioning the layout
            .image = swapchainImage.getImageHandle(),
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,						// dont care about previous layout
            .layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,		// set new layout to color attachment optimal
        } };
        std::array memBarrier0 = { gpu::MemoryBarrier{
            .awaitedAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,				// wait for writing the vertex buffer
            .waitingStages = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR,		// the vertex creating must wait
        } };
        cmdList.insertBarriers(memBarrier0, {}, imgBarrier0);

        double intpart;
        totalElapsedTime += appstate->getDeltaTimeSeconds();
        float r = std::cos(totalElapsedTime * 0.21313) * 0.3f + 0.5f;
        float g = std::cos(totalElapsedTime * 0.75454634) * 0.3f + 0.5f;
        float b = std::cos(totalElapsedTime) * 0.3f + 0.5f;

        VkClearValue clear{ .color = VkClearColorValue{.float32 = { r, g, b, 1.0f } } };

        std::array colorAttachments{
            gpu::RenderAttachmentInfo{
                .image = swapchainImage.getImageHandle(),
                .clearValue = clear,
            }
        };
        cmdList.beginRendering(gpu::BeginRenderingInfo{
            .colorAttachments = colorAttachments,
            });

        cmdList.bindPipeline(pipeline);

        VkViewport viewport{
            .x = 0,
            .y = 0,
            .width = (f32)swapchainImage.getImageHandle()->getVkExtent().width,
            .height = (f32)swapchainImage.getImageHandle()->getVkExtent().height,
            .minDepth = 0,
            .maxDepth = 1,
        };
        cmdList.setViewport(viewport);

        auto set = setAllocator.getSet();
        cmdList.updateSetBuffer(set, 0, uniformBuffer);
        cmdList.bindSet(0, set);

        cmdList.bindVertexBuffer(0, vertexBuffer);

        cmdList.draw(3, 1, 0, 0);

        cmdList.endRendering();

        std::array imgBarrier1 = { gpu::ImageBarrier{
            .image = swapchainImage.getImageHandle(),
            .layoutBefore = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .layoutAfter = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        } };
        cmdList.insertBarriers({}, {}, imgBarrier1);

        cmdList.end();

        // "++currentFrame->finishCounter " is the value that will be set to the timeline when the execution is finished, basicly incrementing it 
        // the timeline is the counter we use to see if the frame is finished executing on the gpu later.
        std::array signalTimelines = { std::tuple{ &currentFrame->timeline, ++currentFrame->timelineCounter } };

        gpu::SubmitInfo submitInfo;
        submitInfo.commandLists.push_back(std::move(cmdList));
        submitInfo.signalOnCompletion = { &currentFrame->presentSignal, 1 };
        submitInfo.signalTimelines = signalTimelines;
        queue.submit(std::move(submitInfo));

        queue.present(std::move(swapchainImage), currentFrame->presentSignal);
        swapchainImage = renderWindow.aquireNextImage();
        
        // we get the next frame context
        auto frameContext = std::move(frames.back());
        frames.pop_back();
        frames.push_front(std::move(frameContext));
        currentFrame = &frames.front();
        queue.checkForFinishedSubmits();

        // we wait on the gpu to finish executing the frame
        // as we have two frame contexts we are actually waiting on the previous frame to complete.
        // if you only have one frame in flight you can just wait on the frame to finish here too.
        currentFrame->timeline.wait(currentFrame->timelineCounter);
    }

    virtual void deinit(std::shared_ptr<daxa::AppState> appstate) override {
        //defaultRenderer->deinit();
        device.waitIdle();
        frames.clear();
    }
};

int main(int argc, char* args[])
{
    daxa::initialize();
    {
        daxa::Application app{ 1000, 1000, std::string("Test"), std::make_unique<MyUser>() };

        app.run();
    }
    daxa::cleanup();

    return 0;
}