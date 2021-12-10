#include <iostream>

#include "Daxa.hpp"

class MyUser {
public:
	MyUser(daxa::AppState& app)
		: device{ daxa::gpu::Device::create() }
		, queue{ this->device.createQueue() }
		, swapchain{ this->device.createSwapchain(app.window->getSurface(), app.window->getSize()[0], app.window->getSize()[1]) }
		, swapchainImage{ this->swapchain.aquireNextImage() }
	{

		char const* vertexShaderGLSL = R"(
			#version 450
			#extension GL_KHR_vulkan_glsl : enable

			layout(location = 0) in vec3 position;
			layout(location = 1) in vec4 color;

			layout(location = 10) out vec4 v_color;

			layout(set = 0, binding = 0) uniform Globals {
				vec4 data;
			} globals;

			void main()
			{
				v_color = color;
				gl_Position = vec4(position, 1.0f);
			}
		)";

		char const* fragmentShaderGLSL = R"(
			#version 450
			#extension GL_KHR_vulkan_glsl : enable

			layout(location = 10) in vec4 v_color;

			layout (location = 0) out vec4 outFragColor;

			void main()
			{
				vec4 color = v_color;
				outFragColor = color;
			}
		)";

		daxa::gpu::ShaderModuleHandle vertexShader = device.tryCreateShderModuleFromGLSL(
			vertexShaderGLSL,
			VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT
		).value();

		daxa::gpu::ShaderModuleHandle fragmenstShader = device.tryCreateShderModuleFromGLSL(
			fragmentShaderGLSL,
			VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT
		).value();

		daxa::gpu::GraphicsPipelineBuilder pipelineBuilder;
		pipelineBuilder
			.addShaderStage(vertexShader)
			.addShaderStage(fragmenstShader)
			// adding a vertex input attribute binding:
			.beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)
			// all added vertex input attributes are added to the previously added vertex input attribute binding
			.addVertexInputAttribute(VK_FORMAT_R32G32B32_SFLOAT)			// positions
			.addVertexInputAttribute(VK_FORMAT_R32G32B32A32_SFLOAT)			// colors
			// location of attachments in a shader are implied by the order they are added in the pipeline builder:
			.addColorAttachment(swapchain.getVkFormat());

		pipeline = device.createGraphicsPipeline(pipelineBuilder);

		globalsUniformAllocator = device.createBindingSetAllocator(pipeline->getSetDescription(0));

		// use the explicit create info structs:
		daxa::gpu::BufferCreateInfo bufferCI{
			.size = sizeof(float) * 3 * 3 /* positions */ + sizeof(float) * 4 * 3 /* colors */,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU,
		};
		vertexBuffer = device.createBuffer(bufferCI);

		std::array vertecies = {
			 1.f, 1.f, 0.0f,	1.f, 0.f, 0.f, 1.f,
			-1.f, 1.f, 0.0f,	0.f, 1.f, 0.f, 1.f,
			 0.f,-1.f, 0.0f,	0.f, 0.f, 1.f, 1.f,
		};
		vertexBuffer->upload(vertecies.data(), sizeof(decltype(vertecies)));

		// or use them embedded, like a named parameter list:
		globalUniformBuffer = device.createBuffer({
			.size = sizeof(float) * 4,
			.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
		});

		globalUnifromsSet = globalsUniformAllocator.getSet();
		globalUnifromsSet->bindBuffer(0, globalUniformBuffer);

		for (int i = 0; i < 3; i++) {
			frames.push_back(PerFrameData{ .presentSignal = device.createSignal(), .timeline = device.createTimelineSemaphore(), .timelineCounter = 0 });
		}
	}

	void update(daxa::AppState& app) {
		printf("update, dt: %f\n", app.getDeltaTimeSeconds());

		if (app.window->getSize()[0] != swapchain.getSize().width || app.window->getSize()[1] != swapchain.getSize().height) {
			device.waitIdle();
			swapchain.resize(VkExtent2D{ .width = app.window->getSize()[0], .height = app.window->getSize()[1] });
			swapchainImage = swapchain.aquireNextImage();
		}

		auto* currentFrame = &frames.front();

		auto cmdList = device.getEmptyCommandList();

		cmdList->begin();


		/// ------------ Begin Data Uploading ---------------------



		std::array someBufferdata = { 1.0f , 1.0f , 1.0f ,1.0f };
		cmdList->copyHostToBuffer(daxa::gpu::HostToBufferCopyInfo{
			.src = someBufferdata.data(),
			.dst = globalUniformBuffer,
			.size = someBufferdata.size() * sizeof(float)
			});

		// array because we can allways pass multiple barriers at once for driver efficiency
		std::array imgBarrier0 = { daxa::gpu::ImageBarrier{
			.waitingStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,	// as we write to the image in the frag shader we need to make sure its finished transitioning the layout
			.image = swapchainImage.getImageHandle(),
			.layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,						// dont care about previous layout
			.layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,		// set new layout to color attachment optimal
		} };
		// array because we can allways pass multiple barriers at once for driver efficiency
		std::array memBarrier0 = { daxa::gpu::MemoryBarrier{
			.awaitedAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,				// wait for writing the vertex buffer
			.waitingStages = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR,		// the vertex creating must wait
		} };
		cmdList->insertBarriers(memBarrier0, {}, imgBarrier0);


		/// ------------ End Data Uploading ---------------------

		std::array framebuffer{ 
			daxa::gpu::RenderAttachmentInfo{
				.image = swapchainImage.getImageHandle(),
				.clearValue = VkClearValue{.color = VkClearColorValue{.float32 = { 0.9f, 0.9f, 0.9f, 1.0f } } },
			}
		};
		cmdList->beginRendering(daxa::gpu::BeginRenderingInfo{
			.colorAttachments = framebuffer,
		});

		cmdList->bindPipeline(pipeline);

		cmdList->bindSet(0, globalUnifromsSet);

		cmdList->bindVertexBuffer(0, vertexBuffer);

		cmdList->draw(3, 1, 0, 0);

		cmdList->endRendering();

		std::array imgBarrier1 = { daxa::gpu::ImageBarrier{
			.image = swapchainImage.getImageHandle(),
			.layoutBefore = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.layoutAfter = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		} };
		cmdList->insertBarriers({}, {}, imgBarrier1);

		cmdList->end();

		std::array signalTimelines = { std::tuple{ &currentFrame->timeline, ++currentFrame->timelineCounter } };
		queue.submit(daxa::gpu::SubmitInfo{
			.commandLists = { std::move(cmdList) },
			.signalTimelines = signalTimelines,
			.signalOnCompletion = { &currentFrame->presentSignal, 1 },
		});

		queue.present(std::move(swapchainImage), currentFrame->presentSignal);
		swapchainImage = swapchain.aquireNextImage();

		auto frameContext = std::move(frames.back());
		frames.pop_back();
		frames.push_front(std::move(frameContext));
		currentFrame = &frames.front();
		queue.checkForFinishedSubmits();

		currentFrame->timeline.wait(currentFrame->timelineCounter);
	}

	void cleanup(daxa::AppState& app) {
		device.waitIdle();
		frames.clear();
	}

	~MyUser() {

	}
private:
	daxa::gpu::Device device;
	daxa::gpu::Queue queue;
	daxa::gpu::Swapchain swapchain;
	daxa::gpu::SwapchainImage swapchainImage;
	daxa::gpu::GraphicsPipelineHandle pipeline;
	daxa::gpu::BindingSetAllocator globalsUniformAllocator;
	daxa::gpu::BufferHandle vertexBuffer;
	daxa::gpu::BufferHandle globalUniformBuffer;
	daxa::gpu::BindingSetHandle globalUnifromsSet;
	double totalElapsedTime = 0.0f;
	struct PerFrameData {
		daxa::gpu::SignalHandle presentSignal;
		daxa::gpu::TimelineSemaphore timeline;
		u64 timelineCounter = 0;
	};
	std::deque<PerFrameData> frames;
};

int main()
{
	daxa::initialize();

	{
		daxa::Application<MyUser> app{ 1000, 1000, "Daxa Triangle Sample" };
		app.run();
	}

	daxa::cleanup();
}