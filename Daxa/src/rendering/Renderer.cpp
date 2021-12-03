#include "Renderer.hpp"

#include <chrono>
using namespace std::chrono;

namespace daxa {

	Renderer::Renderer(std::shared_ptr<Window> win)
		: window{ std::move(win) }
		, device{ gpu::Device::createNewDevice() }
		, renderWindow{ device->createRenderWindow(window->getWindowHandleSDL(), window->getSize()[0], window->getSize()[1]) }
		, stagingBufferPool{ &*device, (size_t)100'000, (VkBufferUsageFlags)VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU }
	{ 
		for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
			this->frameResc.push_back(PerFrameRessources{ 
				.timeline = device->createTimelineSemaphore()
			});
		}
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
			VkFenceCreateInfo fenceCreateInfo{
					.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
					.pNext = nullptr,
			};
			vkCreateSemaphore(device->getVkDevice(), &semaphoreCreateInfo, nullptr, &frameResc[i].semaphores["render"]);
		}

		gpu::ShaderModuleHandle vertexShader = device->tryCreateShderModuleFromGLSL(
			std::filesystem::path{ "daxa/shaders/test.vert" }, 
			VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT
		).value();

		gpu::ShaderModuleHandle fragmenstShader = device->tryCreateShderModuleFromGLSL(
			std::filesystem::path{ "daxa/shaders/test.frag" },
			VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT
		).value();

		printf("after frag creation\n");
		VkPipelineVertexInputStateCreateInfo d;
		gpu::GraphicsPipelineBuilder pipelineBuilder;
		pipelineBuilder.addShaderStage(vertexShader);
		pipelineBuilder.addShaderStage(fragmenstShader);
		pipelineBuilder.addColorAttachment(renderWindow.getVkFormat());
		testPipeline = device->createGraphicsPipeline(pipelineBuilder);
	}

	void Renderer::nextFrameContext() {
		auto frameContext = std::move(frameResc.back());
		frameResc.pop_back();
		frameResc.push_front(std::move(frameContext));
		device->recycle();
	}
	
	void Renderer::draw(float deltaTime) {
		auto& frame = frameResc.front();

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
		gpu::BeginRenderingInfo renderInfo;
		renderInfo.colorAttachments = colorAttachments; 
		cmdList.beginRendering(renderInfo);

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

		std::array signalTimelines = { std::tuple{ &frame.timeline, ++frame.finishCounter } };
		std::vector<gpu::CommandList> commandLists;
		commandLists.push_back(std::move(cmdList));
		device->submit({
			.commandLists = std::move(commandLists),
			.signalTimelines = signalTimelines,
			.signalSemaphores = { &frame.semaphores["render"], 1 },
		});

		std::array waitOnSemasPresent = { frame.semaphores["render"] };
		// present and aquire are combined as differencevendors implement the vsync waiting differently
		// nvidia waits in present, amd mostly waits on aquire
		// by grouping them here we allways get the same waiting behavior
		renderWindow.present(std::move(swapchainImage), { &frame.semaphores["render"], 1 });

		if (window->getSize()[0] != renderWindow.getSize().width || window->getSize()[1] != renderWindow.getSize().height) {
			device->waitIdle();
			renderWindow.resize(VkExtent2D{ .width = window->getSize()[0], .height = window->getSize()[1] });
		}
		swapchainImage = renderWindow.aquireNextImage();

		nextFrameContext();
	}

	void Renderer::deinit() {

	}
}
