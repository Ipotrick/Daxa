#include "Renderer.hpp"

#include <chrono>
using namespace std::chrono;

namespace daxa {

	Renderer::Renderer(std::shared_ptr<Window> win)
		: window{ std::move(win) }
		, device{ gpu::Device::createNewDevice() }
		, renderWindow{ device->createRenderWindow(window->getWindowHandleSDL(), window->getSize()[0], window->getSize()[1], VK_PRESENT_MODE_IMMEDIATE_KHR) }
		, stagingBufferPool{ &*device, (size_t)100'000, (VkBufferUsageFlags)VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU }
	{ 
		for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
			this->frameResc.push_back(PerFrameRessources{ 
				.timeline = device->createTimelineSemaphore()
			});
		}
		currentFrame = &frameResc.front();
		swapchainImage = renderWindow.aquireNextImage();
	}

	Renderer::~Renderer() {
		device->waitIdle();
		for (auto& frame : frameResc) {
			for (auto& [name, sema] : frame.semaphores) {
				vkDestroySemaphore(device->getVkDevice(), sema, nullptr);
			}
		}
		frameResc.clear();
		persResc.reset();
	}

	void Renderer::init() {
		for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
			VkSemaphoreCreateInfo semaphoreCreateInfo{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				.pNext = nullptr,
			}; 
			vkCreateSemaphore(device->getVkDevice(), &semaphoreCreateInfo, nullptr, &frameResc[i].semaphores["render"]);
		}

		gpu::ShaderModuleHandle vertexShader = device->tryCreateShderModuleFromFile(
			"daxa/shaders/test.vert", 
			VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT
		).value();

		gpu::ShaderModuleHandle fragmenstShader = device->tryCreateShderModuleFromFile(
			"daxa/shaders/test.frag",
			VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT
		).value();

		VkPipelineVertexInputStateCreateInfo d;
		gpu::GraphicsPipelineBuilder pipelineBuilder;
		pipelineBuilder.addShaderStage(vertexShader);
		pipelineBuilder.addShaderStage(fragmenstShader);
		pipelineBuilder.addColorAttachment(renderWindow.getVkFormat());
		testPipeline = device->createGraphicsPipeline(pipelineBuilder);

		//auto setAllocator = device->createBindingSetAllocator(testPipeline->getSetDescription(/*set:*/0));
		//
		//auto set = setAllocator.getSet();

		// set will automaticly be recycled into the allocator
		// allocator dies and kills all its sets, it also checks if all sets are released as a debug messure
	}

	void Renderer::nextFrameContext() {
		auto frameContext = std::move(frameResc.back());
		frameResc.pop_back();
		frameResc.push_front(std::move(frameContext));
		currentFrame = &frameResc.front();
		device->recycle();
	}
	
	void Renderer::draw(float deltaTime) {
		auto cmdList = device->getEmptyCommandList();

		cmdList.begin();

		cmdList.changeImageLayout(swapchainImage.getImageHandle(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		double intpart;
		totalElapsedTime += deltaTime;
		float r = std::cos(totalElapsedTime * 0.21313) * 0.3f + 0.5f;
		float g = std::cos(totalElapsedTime * 0.75454634) * 0.3f + 0.5f;
		float b = std::cos(totalElapsedTime) * 0.3f + 0.5f;

		VkClearValue clear{ .color = VkClearColorValue{.float32 = { r, g, b, 1.0f } } };

		std::array colorAttachments{
			gpu::RenderAttachmentInfo{
				.image = swapchainImage.getImageHandle(),
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.clearValue = clear,
			}
		};
		cmdList.beginRendering(gpu::BeginRenderingInfo{
			.colorAttachments = colorAttachments,
		});

		cmdList.bindPipeline(testPipeline);

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
			.offset = { 0, 0 },
			.extent = { (u32)viewport.width, (u32)viewport.height },
		};
		cmdList.setScissor(scissor);

		cmdList.draw(3, 1, 0, 0);

		cmdList.endRendering(); 

		cmdList.changeImageLayout(swapchainImage.getImageHandle(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		cmdList.end();

		std::array signalTimelines = { std::tuple{ &currentFrame->timeline, ++currentFrame->finishCounter } };
		std::vector<gpu::CommandList> commandLists;
		commandLists.push_back(std::move(cmdList));
		device->submit({
			.commandLists = std::move(commandLists),
			.signalTimelines = signalTimelines,
			.signalSemaphores = { &currentFrame->semaphores["render"], 1 },
		});

		std::array waitOnSemasPresent = { currentFrame->semaphores["render"] };
		renderWindow.present(std::move(swapchainImage), { &currentFrame->semaphores["render"], 1 });

		if (window->getSize()[0] != renderWindow.getSize().width || window->getSize()[1] != renderWindow.getSize().height) {
			device->waitIdle();
			renderWindow.resize(VkExtent2D{ .width = window->getSize()[0], .height = window->getSize()[1] });
		}
		swapchainImage = renderWindow.aquireNextImage();

		nextFrameContext();

		currentFrame->timeline.wait(currentFrame->finishCounter);
	}

	void Renderer::deinit() {

	}
}
