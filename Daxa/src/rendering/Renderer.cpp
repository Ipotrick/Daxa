#include "Renderer.hpp"

namespace daxa {

	Renderer::Renderer(std::shared_ptr<Window> win)
		: window{ std::move(win) }
		, device{ gpu::Device::createNewDevice() }
		, renderWindow{ this->device, gpu::Device::getInstance(), window->getWindowHandleSDL(), window->getSize()[0], window->getSize()[1], VK_PRESENT_MODE_FIFO_KHR }
	{ 
		for (int i = 0; i < 3; i++) {
			this->frameResc.push_back(PerFrameRessources{});
		}
	}

	Renderer::~Renderer() {
		device->waitIdle();
		for (auto& frame : frameResc) {
			for (auto& [name, sema] : frame.semaphores) {
				vkDestroySemaphore(device->getVkDevice(), sema, nullptr);
			}
			for (auto& [name, fence] : frame.fences) {
				vkDestroyFence(device->getVkDevice(), fence, nullptr);
			}
		}
		frameResc.clear();
		persResc.reset();
	}

	void Renderer::init() {
		for (int i = 0; i < 3; i++) {
			frameResc[i].semaphores["render"] = device->getVkDevice().createSemaphore({});
			frameResc[i].semaphores["aquireSwapchainImage"] = device->getVkDevice().createSemaphore({});
		}

		gpu::ShaderModuleHandle vertexShader = device->tryCreateShderModuleFromGLSL(
			std::filesystem::path{ "daxa/shaders/test.vert" }, 
			(vk::ShaderStageFlagBits)VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT
		).value();

		gpu::ShaderModuleHandle fragmenstShader = device->tryCreateShderModuleFromGLSL(
			std::filesystem::path{ "daxa/shaders/test.frag" },
			(vk::ShaderStageFlagBits)VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT
		).value();

		printf("after frag creation\n");
		VkPipelineVertexInputStateCreateInfo d;
		gpu::GraphicsPipelineBuilder pipelineBuilder;
		pipelineBuilder.addShaderStage(vertexShader);
		pipelineBuilder.addShaderStage(fragmenstShader);
		pipelineBuilder.addColorAttachment((vk::Format)renderWindow.getVkFormat());
		testPipeline = device->createGraphicsPipeline(pipelineBuilder);
	}

	void Renderer::nextFrameContext() {
		auto frameContext = std::move(frameResc.back());
		frameResc.pop_back();
		frameResc.push_front(std::move(frameContext));
		device->nextFrameContext();
	}
	
	void Renderer::draw(float deltaTime) {
		auto& frame = frameResc.front();

		auto swapchainImage = renderWindow.getNextImage(frame.semaphores["aquireSwapchainImage"]);

		auto cmdList = device->getEmptyCommandList();

		cmdList.begin();

		cmdList.changeImageLayout(swapchainImage.getImageHandle(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		double intpart;
		totalElapsedTime += deltaTime;
		float r = std::cos(totalElapsedTime * 0.21313) * 0.3f + 0.5f;
		float g = std::cos(totalElapsedTime * 0.75454634) * 0.3f + 0.5f;
		float b = std::cos(totalElapsedTime) * 0.3f + 0.5f;
		printf("color: %f, %f, %f\n", r, g, b);

		VkClearValue clear{ .color = VkClearColorValue{.float32 = { r, g, b, 1.0f } } };

		std::array colorAttachments{
			gpu::RenderAttachmentInfo{
				.image = swapchainImage.getImageHandle(),
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.clearValue = clear,
			}
		};
		gpu::BeginRenderingInfo renderInfo;
		renderInfo.colorAttachments = colorAttachments; 
		cmdList.beginRendering(renderInfo);

		vkCmdBindPipeline(cmdList.getVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, testPipeline->getVkPipeline());

		VkViewport viewport{
			.x = 0,
			.y = 0,
			.width = (f32)swapchainImage.getImageHandle()->getExtent().width,
			.height = (f32)swapchainImage.getImageHandle()->getExtent().height,
			.minDepth = 0,
			.maxDepth = 1,
		};
		cmdList.setViewport(viewport);

		VkRect2D scissor{
			.offset = {0,0},
			.extent = { (u32)viewport.width, (u32)viewport.height },
		};
		cmdList.setScissor(scissor);

		cmdList.draw(3, 1, 0, 0);

		cmdList.endRendering();

		cmdList.changeImageLayout(swapchainImage.getImageHandle(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		cmdList.end();

		std::array waitOnSemasSubmit = { frame.semaphores["aquireSwapchainImage"] };
		std::array singalSemasSubmit = { frame.semaphores["render"] };
		device->submit(std::move(cmdList), {
			.waitOnSemaphores = waitOnSemasSubmit,
			.signalSemaphores = singalSemasSubmit,
			});

		std::array waitOnSemasPresent = { frame.semaphores["render"] };
		device->present(swapchainImage, waitOnSemasPresent);

		nextFrameContext();
	}

	void Renderer::deinit() {

	}
}
