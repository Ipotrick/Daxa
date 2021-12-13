#include <iostream>

#define GLM_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Daxa.hpp"

class GimbalLockedCameraController {
public:
	glm::mat4 update(daxa::Window& window, f32 dt) {
		if (bCursorCaptured) {
			if (window.keyJustPressed(daxa::Scancode::ESCAPE)) {
				printf("release mouse\n");
				window.releaseCursor();
				bCursorCaptured = false;
			}
		} else {
			if (window.buttonJustPressed(daxa::MouseButton::Left)) {
				printf("capture mouse\n");
				window.captureCursor();
				bCursorCaptured = true;
			}
		}

		auto yawRotaAroundUp = glm::rotate(glm::mat4(1.0f), yaw, {0.f,0.f,1.f});
		glm::vec4 translation = {};
		if (window.keyPressed(daxa::Scancode::W)) {
			glm::vec4 direction = { 0.0f, 1.0f, 0.0f, 0.0f };
			translation += yawRotaAroundUp * direction * dt * translationSpeed;
		}
		if (window.keyPressed(daxa::Scancode::S)) {
			glm::vec4 direction = { 0.0f, -1.0f, 0.0f, 0.0f };
			translation += yawRotaAroundUp * direction * dt * translationSpeed;
		}
		if (window.keyPressed(daxa::Scancode::A)) {
			glm::vec4 direction = { -1.0f, 0.0f, 0.0f, 0.0f };
			translation += yawRotaAroundUp * direction * dt * translationSpeed;
		}
		if (window.keyPressed(daxa::Scancode::D)) {
			glm::vec4 direction = { 1.0f, 0.0f, 0.0f, 0.0f };
			translation += yawRotaAroundUp * direction * dt * translationSpeed;
		}
		if (window.keyPressed(daxa::Scancode::SPACE)) {
			translation += glm::vec4{ 0.f, 0.f, 1.f, 0.f } * dt * translationSpeed;
		}
		if (window.keyPressed(daxa::Scancode::LSHIFT)) {
			translation += glm::vec4{ 0.f, 0.f, -1.f, 0.f } * dt * translationSpeed;
		}
		pitch += window.getCursorPositionChange()[0] * cameraSwaySpeed * dt;
		pitch = std::clamp(pitch, 0.0f, glm::pi<f32>()*2);
		yaw += window.getCursorPositionChange()[1] * cameraSwaySpeed * dt;

		//printf("pitch: %f, yaw: %f\n");
		//printf("position: {%f,%f,%f}\n", position.x, position.y, position.z);

		auto prespective = glm::perspective(fov, (f32)window.getSize()[0]/(f32)window.getSize()[1], near, far);
		auto rota = yawRotaAroundUp * glm::rotate(glm::mat4(), pitch, glm::vec3{1.f,0.f,0.f});
		auto cameraModelMat = glm::translate(rota, {translation.x, translation.y, translation.z});
		auto view = glm::inverse(cameraModelMat);
		return prespective * view;
	}
private:
	bool bCursorCaptured = false;
	f32 fov = 74.0f;
	f32 near = 0.1f;
	f32 far = 1'000.0f;
	f32 cameraSwaySpeed = 0.01f;
	f32 translationSpeed = 1.0f;
	glm::vec4 up = { 0.f, 0.f, 1.0f, 0.f };
	glm::vec4 position = { 0.f, 0.f, 0.f, 1.f };
	f32 yaw = glm::pi<f32>();
	f32 pitch = glm::pi<f32>();
};

class MyUser {
public:
	MyUser(daxa::AppState& app) 
		: device{ daxa::gpu::Device::create() }
		, queue{ this->device->createQueue() }
		, swapchain{ this->device->createSwapchain(app.window->getSurface(), app.window->getSize()[0], app.window->getSize()[1], VK_PRESENT_MODE_IMMEDIATE_KHR)}
		, swapchainImage{ this->swapchain->aquireNextImage() }
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

		daxa::gpu::ShaderModuleHandle vertexShader = device->tryCreateShderModuleFromGLSL(
			vertexShaderGLSL,
			VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT
		).value();

		daxa::gpu::ShaderModuleHandle fragmenstShader = device->tryCreateShderModuleFromGLSL(
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
			.addColorAttachment(swapchain->getVkFormat());

		this->pipeline = device->createGraphicsPipeline(pipelineBuilder);

		this->globalsUniformAllocator = device->createBindingSetAllocator(pipeline->getSetDescription(0));

		// use the explicit create info structs:
		daxa::gpu::BufferCreateInfo bufferCI{
			.size = sizeof(float) * 3 * 3 /* positions */ + sizeof(float) * 4 * 3 /* colors */,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
		};
		this->vertexBuffer = device->createBuffer(bufferCI);

		// or use them embedded, like a named parameter list:
		this->globalUniformBuffer = device->createBuffer({
			.size = sizeof(float) * 4,
			.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
		});

		for (int i = 0; i < 3; i++) {
			frames.push_back(PerFrameData{ .presentSignal = device->createSignal(), .timeline = device->createTimelineSemaphore(), .timelineCounter = 0 });
		}
	}

	void update(daxa::AppState& app) {
		//printf("update, dt: %f\n", app.getDeltaTimeSeconds());

		if (app.window->getSize()[0] != swapchain->getSize().width || app.window->getSize()[1] != swapchain->getSize().height) {
			device->waitIdle();
			swapchain->resize(VkExtent2D{ .width = app.window->getSize()[0], .height = app.window->getSize()[1] });
			swapchainImage = swapchain->aquireNextImage();
		}

		auto* currentFrame = &frames.front();

		auto cmdList = device->getEmptyCommandList();

		cmdList->begin();


		/// ------------ Begin Data Uploading ---------------------

		auto vp = cameraController.update(*app.window, app.getDeltaTimeSeconds());

		std::array vertecies = {
			 1.f, 1.f, 0.0f,	1.f, 0.f, 0.f, 1.f,
			-1.f, 1.f, 0.0f,	0.f, 1.f, 0.f, 1.f,
			 0.f,-1.f, 0.0f,	0.f, 0.f, 1.f, 1.f,
		};
		cmdList->copyHostToBuffer(daxa::gpu::HostToBufferCopyInfo{
			.src = vertecies.data(),
			.dst = vertexBuffer,
			.size = sizeof(decltype(vertecies))
		});

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
				.clearValue = { .color = VkClearColorValue{.float32 = { 1.f, 1.f, 1.f, 1.f } } },
			}
		};
		cmdList->beginRendering(daxa::gpu::BeginRenderingInfo{
			.colorAttachments = framebuffer,
		});
		
		cmdList->bindPipeline(pipeline);
		
		auto set = globalsUniformAllocator->getSet();
		set->bindBuffer(0, globalUniformBuffer);
		cmdList->bindSet(0, set);
		
		cmdList->bindVertexBuffer(0, vertexBuffer);
		
		cmdList->draw(3, 1, 0, 0);
		
		cmdList->endRendering();

		// array because we can allways pass multiple barriers at once for driver efficiency
		std::array imgBarrier1 = { daxa::gpu::ImageBarrier{
			.image = swapchainImage.getImageHandle(),
			.layoutBefore = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.layoutAfter = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		} };
		cmdList->insertBarriers({}, {}, imgBarrier1);

		cmdList->end();

		// "++currentFrame->finishCounter " is the value that will be set to the timeline when the execution is finished, basicly incrementing it 
		// the timeline is the counter we use to see if the frame is finished executing on the gpu later.
		std::array signalTimelines = { std::tuple{ currentFrame->timeline, ++currentFrame->timelineCounter } };

		daxa::gpu::SubmitInfo submitInfo;
		submitInfo.commandLists.push_back(std::move(cmdList));
		submitInfo.signalOnCompletion = { &currentFrame->presentSignal, 1 };
		submitInfo.signalTimelines = signalTimelines;
		queue->submit(submitInfo);

		queue->present(std::move(swapchainImage), currentFrame->presentSignal);
		swapchainImage = swapchain->aquireNextImage();

		// we get the next frame context
		auto frameContext = std::move(frames.back());
		frames.pop_back();
		frames.push_front(std::move(frameContext));
		currentFrame = &frames.front();
		queue->checkForFinishedSubmits();

		// we wait on the gpu to finish executing the frame
		// as we have two frame contexts we are actually waiting on the previous frame to complete. 
		// if you only have one frame in flight you can just wait on the frame to finish here too.
		currentFrame->timeline->wait(currentFrame->timelineCounter);
	}

	void cleanup(daxa::AppState& app) {
		queue->waitForFlush();
		device->waitIdle();
		frames.clear();
	}

	~MyUser() {

	}
private:
	GimbalLockedCameraController cameraController{};
	daxa::gpu::DeviceHandle device;
	daxa::gpu::QueueHandle queue;
	daxa::gpu::SwapchainHandle swapchain;
	daxa::gpu::SwapchainImage swapchainImage;
	daxa::gpu::GraphicsPipelineHandle pipeline;
	daxa::gpu::BindingSetAllocatorHandle globalsUniformAllocator;
	daxa::gpu::BufferHandle vertexBuffer;
	daxa::gpu::BufferHandle globalUniformBuffer;
	double totalElapsedTime = 0.0f;
	struct PerFrameData {
		daxa::gpu::SignalHandle presentSignal;
		daxa::gpu::TimelineSemaphoreHandle timeline;
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