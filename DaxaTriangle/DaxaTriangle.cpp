#include <iostream>

#include "Daxa.hpp"

class MyUser {
public:
	MyUser(daxa::AppState& app) 
		: device{ daxa::gpu::Device::create() }
		, queue{ this->device.createQueue() }
		, renderWindow{ this->device.createRenderWindow(app.window->getSurface(), app.window->getSize()[0], app.window->getSize()[1])}
		, swapchainImage{ this->renderWindow.aquireNextImage() }
	{ 

		char const* vertexShaderGLSL = R"(
			#version 450
			#extension GL_KHR_vulkan_glsl : enable

			layout(location = 0) in vec3 position;
			layout(location = 1) in vec4 color;

			layout(location = 10) out vec4 v_color;

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

			layout(set = 0, binding = 0) uniform SomeBuffer {
				vec4 data;
			} someBuffer;

			void main()
			{
				vec4 color = v_color;
				color *= someBuffer.data.b;
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
			.beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)	// add vertex attributes:
			.addVertexInputAttribute(VK_FORMAT_R32G32B32_SFLOAT)			// positions
			.addVertexInputAttribute(VK_FORMAT_R32G32B32A32_SFLOAT)			// colors
			.addColorAttachment(renderWindow.getVkFormat());

		this->pipeline = device.createGraphicsPipeline(pipelineBuilder);

		this->setAllocator = device.createBindingSetAllocator(pipeline->getSetDescription(0));

		constexpr size_t vertexBufferSize = sizeof(float) * 3 * 3 /* positions */ + sizeof(float) * 4 * 3 /* colors */;
		daxa::gpu::BufferCreateInfo bufferCI{
			.size = vertexBufferSize,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
		};
		vertexBuffer = device.createBuffer(bufferCI);

		uniformBuffer = device.createBuffer({
			.size = sizeof(float) * 4,
			.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
		});

		for (int i = 0; i < 3; i++) {
			frames.push_back(PerFrameData{ .presentSignal = device.createSignal(), .timeline = device.createTimelineSemaphore(), .timelineCounter = 0 });
		}
	}

	void update(daxa::AppState& app) {
		printf("update, dt: %f\n", app.getDeltaTimeSeconds());

		if (app.window->getSize()[0] != renderWindow.getSize().width || app.window->getSize()[1] != renderWindow.getSize().height) {
			device.waitIdle();
			renderWindow.resize(VkExtent2D{ .width = app.window->getSize()[0], .height = app.window->getSize()[1] });
			swapchainImage = renderWindow.aquireNextImage();
		}

		auto* currentFrame = &frames.front();

		auto cmdList = device.getEmptyCommandList();

		cmdList.begin();


		/// ------------ Begin Data Uploading ---------------------


		std::array vertecies = {
			 1.f, 1.f, 0.0f,	1.f, 0.f, 0.f, 1.f,
			-1.f, 1.f, 0.0f,	0.f, 1.f, 0.f, 1.f,
			 0.f,-1.f, 0.0f,	0.f, 0.f, 1.f, 1.f,
		};
		cmdList.uploadToBuffer(vertecies, vertexBuffer);

		std::array someBufferdata = { 1.0f , 1.0f , 1.0f ,1.0f };
		cmdList.uploadToBuffer(someBufferdata, uniformBuffer);

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
		cmdList.insertBarriers(memBarrier0, {}, imgBarrier0);
		
		
		/// ------------ End Data Uploading ---------------------


		totalElapsedTime += app.getDeltaTimeSeconds();
		float r = std::cos(totalElapsedTime * 0.21313) * 0.3f + 0.5f;
		float g = std::cos(totalElapsedTime * 0.75454634) * 0.3f + 0.5f;
		float b = std::cos(totalElapsedTime) * 0.3f + 0.5f;

		VkClearValue clear{ .color = VkClearColorValue{.float32 = { r, g, b, 1.0f } } };

		std::array framebuffer{
			daxa::gpu::RenderAttachmentInfo{
				.image = swapchainImage.getImageHandle(),
				.clearValue = clear,
			}
		};
		cmdList.beginRendering(daxa::gpu::BeginRenderingInfo{
			.colorAttachments = framebuffer,
		});
		
		cmdList.bindPipeline(pipeline);
		
		auto set = setAllocator.getSet();
		set->bindBuffer(0, uniformBuffer);
		cmdList.bindSet(0, set);
		
		cmdList.bindVertexBuffer(0, vertexBuffer);
		
		cmdList.draw(3, 1, 0, 0);
		
		cmdList.endRendering();

		// array because we can allways pass multiple barriers at once for driver efficiency
		std::array imgBarrier1 = { daxa::gpu::ImageBarrier{
			.image = swapchainImage.getImageHandle(),
			.layoutBefore = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.layoutAfter = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		} };
		cmdList.insertBarriers({}, {}, imgBarrier1);

		cmdList.end();

		// "++currentFrame->finishCounter " is the value that will be set to the timeline when the execution is finished, basicly incrementing it 
		// the timeline is the counter we use to see if the frame is finished executing on the gpu later.
		std::array signalTimelines = { std::tuple{ &currentFrame->timeline, ++currentFrame->timelineCounter } };

		daxa::gpu::SubmitInfo submitInfo;
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

	void cleanup(daxa::AppState& app) {
		device.waitIdle();
		frames.clear();
	}

	~MyUser() {

	}
private:
	daxa::gpu::Device device;
	daxa::gpu::Queue queue;
	daxa::gpu::RenderWindow renderWindow;
	daxa::gpu::SwapchainImage swapchainImage;
	daxa::gpu::GraphicsPipelineHandle pipeline;
	daxa::gpu::BindingSetAllocator setAllocator;
	daxa::gpu::BufferHandle vertexBuffer;
	daxa::gpu::BufferHandle uniformBuffer;
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
		daxa::Application<MyUser> app{ 1000, 1000, "Daxa Triangle Sample"};
		app.run();
	}

	daxa::cleanup();
}