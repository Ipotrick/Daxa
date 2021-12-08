#include "Renderer.hpp"

#include <chrono>
using namespace std::chrono;

namespace daxa {

	Renderer::Renderer(std::shared_ptr<Window> win)
		: window{ std::move(win) }
		, device{ gpu::Device::create() }
		, renderWindow{ device.createRenderWindow(window->getWindowHandleSDL(), window->getSize()[0], window->getSize()[1], VK_PRESENT_MODE_IMMEDIATE_KHR) }
		, queue{ device.createQueue() }
	{ 
	}

	Renderer::~Renderer() {
		device.waitIdle();
		frameResc.clear();
	}

	void Renderer::init() {
		for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
			this->frameResc.push_back(PerFrameRessources{
				.renderingFinishedSignal = device.createSignal(),
				.timeline = device.createTimelineSemaphore(),
			});
		}

		currentFrame = &frameResc.front();
		swapchainImage = renderWindow.aquireNextImage();

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
		pipelines["triangle"] = device.createGraphicsPipeline(pipelineBuilder);

		constexpr size_t vertexBufferSize = sizeof(float) * 3 * 3 /* positions */ + sizeof(float) * 4 * 3 /* colors */;
		gpu::BufferCreateInfo bufferCI{
			.size = vertexBufferSize,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,	// these usages are hints to the driver, you the validation layer will allways tell you wich ones are needed
			.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
		};
		buffers["vertex"] = device.createBuffer(bufferCI);

		setAllocators["someBuffer"] = device.createBindingSetAllocator(pipelines["triangle"]->getSetDescription(0));

		gpu::BufferCreateInfo someBufferCI{
			.size = sizeof(float)*4,
			.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
		};
		buffers["someBuffer"] = device.createBuffer(someBufferCI);
	}

	void Renderer::nextFrameContext() {
		auto frameContext = std::move(frameResc.back());
		frameResc.pop_back();
		frameResc.push_front(std::move(frameContext));
		currentFrame = &frameResc.front();
		queue.checkForFinishedSubmits();
	}
	
	void Renderer::draw(float deltaTime) {
		if (window->getSize()[0] != renderWindow.getSize().width || window->getSize()[1] != renderWindow.getSize().height) {
			device.waitIdle();
			renderWindow.resize(VkExtent2D{ .width = window->getSize()[0], .height = window->getSize()[1] });
			swapchainImage = renderWindow.aquireNextImage();
		}

		auto cmdList = device.getEmptyCommandList();

		cmdList.begin();

		std::array vertecies = {
			 1.f, 1.f, 0.0f,		1.f, 0.f, 0.f, 1.f, 
			-1.f, 1.f, 0.0f,		0.f, 1.f, 0.f, 1.f,
			 0.f,-1.f, 0.0f,		0.f, 0.f, 1.f, 1.f,
		};
		cmdList.uploadToBuffer(vertecies, buffers["vertex"]);
		std::array someBufferdata = { 1.0f , 1.0f , 1.0f ,1.0f };
		cmdList.uploadToBuffer(someBufferdata, buffers["someBuffer"]);

		std::array imgBarrier0 = { gpu::ImageBarrier{
			.waitingStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,	// as we write to the image in the frag shader we need to make sure its finished transitioning the layout
			.image = swapchainImage.getImageHandle(),
			.layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,						// dont care about previous layout
			.layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,		// set new layout to color attachment optimal
		}};
		std::array memBarrier0 = { gpu::MemoryBarrier{
			.awaitedAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,				// wait for writing the vertex buffer
			.waitingStages = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR,		// the vertex creating must wait
		} };
		cmdList.insertBarriers(memBarrier0, {}, imgBarrier0);

		double intpart;
		totalElapsedTime += deltaTime;
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

		cmdList.bindPipeline(pipelines["triangle"]);

		VkViewport viewport{
			.x = 0,
			.y = 0,
			.width = (f32)swapchainImage.getImageHandle()->getVkExtent().width,
			.height = (f32)swapchainImage.getImageHandle()->getVkExtent().height,
			.minDepth = 0,
			.maxDepth = 1,
		};
		cmdList.setViewport(viewport);

		auto set = setAllocators["someBuffer"].getSet();
		cmdList.updateSetBuffer(set, 0, buffers["someBuffer"]);
		cmdList.bindSet(0, set);

		cmdList.bindVertexBuffer(0, buffers["vertex"]);

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
		std::array signalTimelines = { std::tuple{ &currentFrame->timeline, ++currentFrame->finishCounter }};

		gpu::SubmitInfo submitInfo;
		submitInfo.commandLists.push_back(std::move(cmdList));
		submitInfo.signalOnCompletion = { &currentFrame->renderingFinishedSignal, 1 };
		submitInfo.signalTimelines = signalTimelines;
		queue.submit(std::move(submitInfo));

		queue.present(std::move(swapchainImage), currentFrame->renderingFinishedSignal);
		swapchainImage = renderWindow.aquireNextImage();

		// we get the next frame context
		nextFrameContext();					

		// we wait on the gpu to finish executing the frame
		// as we have two frame contexts we are actually waiting on the previous frame to complete.
		// if you only have one frame in flight you can just wait on the frame to finish here too.
		currentFrame->timeline.wait(currentFrame->finishCounter);	
	}

	void Renderer::deinit() {

	}
}
