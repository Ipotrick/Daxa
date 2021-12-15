#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define GLM_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Daxa.hpp"

class GimbalLockedCameraController {
public:
	glm::mat4 update(daxa::Window& window, f32 dt) {
		f32 speed = window.keyPressed(daxa::Scancode::LSHIFT) ? translationSpeed * 4.0f : translationSpeed;
		if (window.isCursorCaptured()) {
			if (window.keyJustPressed(daxa::Scancode::ESCAPE)) {
				window.releaseCursor();
			}
		} else {
			if (window.buttonJustPressed(daxa::MouseButton::Left) && window.isCursorOverWindow()) {
				window.captureCursor();
			}
		}

		auto fov = this->fov;
		auto cameraSwaySpeed = this->cameraSwaySpeed;
		if (window.keyPressed(daxa::Scancode::C)) {
			fov *= 0.25f;
			cameraSwaySpeed *= 0.25;
		}

		auto yawRotaAroundUp = glm::rotate(glm::mat4(1.0f), yaw, {0.f,0.f,1.f});
		auto pitchRotation = glm::rotate(glm::mat4(1.0f), pitch, glm::vec3{1.f,0.f,0.f});
		glm::vec4 translation = {};
		if (window.keyPressed(daxa::Scancode::W)) {
			glm::vec4 direction = { 0.0f, 0.0f, -1.0f, 0.0f };
			translation += yawRotaAroundUp * pitchRotation * direction * dt * speed;
		}
		if (window.keyPressed(daxa::Scancode::S)) {
			glm::vec4 direction = { 0.0f, 0.0f, 1.0f, 0.0f };
			translation += yawRotaAroundUp * pitchRotation * direction * dt * speed;
		}
		if (window.keyPressed(daxa::Scancode::A)) {
			glm::vec4 direction = { 1.0f, 0.0f, 0.0f, 0.0f };
			translation += yawRotaAroundUp * direction * dt * speed;
		}
		if (window.keyPressed(daxa::Scancode::D)) {
			glm::vec4 direction = { -1.0f, 0.0f, 0.0f, 0.0f };
			translation += yawRotaAroundUp * direction * dt * speed;
		}
		if (window.keyPressed(daxa::Scancode::SPACE)) {
			translation += yawRotaAroundUp * pitchRotation * glm::vec4{ 0.f,  1.f, 0.f, 0.f } * dt * speed;
		}
		if (window.keyPressed(daxa::Scancode::LCTRL)) {
			translation += yawRotaAroundUp * pitchRotation * glm::vec4{ 0.f, -1.f,  0.f, 0.f } * dt * speed;
		}
		if (window.isCursorCaptured()) {
			pitch -= window.getCursorPositionChange()[1] * cameraSwaySpeed * dt;
			pitch = std::clamp(pitch, 0.0f, glm::pi<f32>());
			yaw += window.getCursorPositionChange()[0] * cameraSwaySpeed * dt;
		}
		position += translation;

		auto prespective = glm::perspective(fov, (f32)window.getSize()[0]/(f32)window.getSize()[1], near, far);
		auto rota = yawRotaAroundUp * pitchRotation;
		auto cameraModelMat = glm::translate(glm::mat4(1.0f), {position.x, position.y, position.z}) * rota;
		auto view = glm::inverse(cameraModelMat);
		return prespective * view;
	}
private:
	f32 fov = 74.0f;
	f32 near = 0.4f;
	f32 far = 1'000.0f;
	f32 cameraSwaySpeed = 0.1f;
	f32 translationSpeed = 5.0f;
	glm::vec4 up = { 0.f, 0.f, 1.0f, 0.f };
	glm::vec4 position = { 0.f, -2.f, 0.f, 1.f };
	f32 yaw = 0.0f;
	f32 pitch = glm::pi<f32>() * 0.5f;
};

class MyUser {
public:
	MyUser(daxa::AppState& app) 
		: device{ daxa::gpu::Device::create() }
		, queue{ this->device->createQueue() }
		, swapchain{ this->device->createSwapchain({
			.surface = app.window->getSurface(),
			.width = app.window->getSize()[0],
			.height = app.window->getSize()[1],
		})}
		, swapchainImage{ this->swapchain->aquireNextImage() }
	{ 
		char const* vertexShaderGLSL = R"(
			#version 450
			#extension GL_KHR_vulkan_glsl : enable

			layout(location = 0) in vec3 position;
			layout(location = 1) in vec2 uv;

			layout(location = 10) out vec2 vtf_uv;

			layout(set = 0, binding = 0) uniform Globals {
				mat4 vp;
			} globals;

			void main()
			{
				vtf_uv = uv;
				gl_Position = globals.vp * vec4(position, 1.0f);
			}
		)";

		char const* fragmentShaderGLSL = R"(
			#version 450
			#extension GL_KHR_vulkan_glsl : enable

			layout(location = 10) in vec2 vtf_uv;

			layout (location = 0) out vec4 outFragColor;

			layout(set = 0, binding = 1) uniform sampler2D tex;

			void main()
			{
				vec4 color = texture(tex, vtf_uv);
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
			.configurateDepthTest({.enableDepthTest = true, .enableDepthWrite = true, .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT})
			// adding a vertex input attribute binding:
			.beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)
			// all added vertex input attributes are added to the previously added vertex input attribute binding
			.addVertexInputAttribute(VK_FORMAT_R32G32B32_SFLOAT)			// positions
			.addVertexInputAttribute(VK_FORMAT_R32G32_SFLOAT)				// uvs
			// location of attachments in a shader are implied by the order they are added in the pipeline builder:
			.addColorAttachment(swapchain->getVkFormat());

		this->pipeline = device->createGraphicsPipeline(pipelineBuilder);

		this->bindingSetAllocator = device->createBindingSetAllocator(pipeline->getSetDescription(0));

		// Begin texture creation

		stbi_set_flip_vertically_on_load(true);
		i32 sizeX, sizeY, numChannels;
		char const* const filepath = "DaxaCube/atlas.png";
		std::uint8_t * textureAtlasHostData = stbi_load(filepath, &sizeX, &sizeY, &numChannels, 0);
		VkFormat       dataFormat;
		switch (numChannels) {
			case 1: dataFormat = VK_FORMAT_R8_SRGB; break;
			case 3: dataFormat = VK_FORMAT_R8G8B8_SRGB; break;
			case 4:
			default: dataFormat = VK_FORMAT_R8G8B8A8_SRGB; break;
		}

		textureAtlas = device->createImage2d({
			.width = (u32)sizeX,
			.height = (u32)sizeY,
			.format = dataFormat,
			.imageAspekt = VK_IMAGE_ASPECT_COLOR_BIT,
			.imageUsage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			.sampler = device->createSampler({
				.minFilter = VK_FILTER_NEAREST,
				.magFilter = VK_FILTER_NEAREST,
			}),
		});

		// end texture creation

		std::array cubeVertecies{
			/*positions*/			/*tex uv*/ 
			0.5f, -0.5f, -0.5f,    0.0f, 0.0f,
			-0.5f, -0.5f, -0.5f,    1.0f, 0.0f,
			0.5f,  0.5f, -0.5f,    0.0f, 1.0f,
			-0.5f,  0.5f, -0.5f,    1.0f, 1.0f,

			-0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
			0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,    0.0f, 1.0f,
			0.5f,  0.5f,  0.5f,    1.0f, 1.0f,

			-0.5f, -0.5f, -0.5f,    0.0f, 0.0f,
			-0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,    0.0f, 1.0f,
			-0.5f,  0.5f,  0.5f,    1.0f, 1.0f,

			0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
			0.5f, -0.5f, -0.5f,    1.0f, 0.0f,
			0.5f,  0.5f,  0.5f,    0.0f, 1.0f,
			0.5f,  0.5f, -0.5f,    1.0f, 1.0f,

			-0.5f, -0.5f, -0.5f,    0.0f, 0.0f,
			0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
			0.5f, -0.5f,  0.5f,    1.0f, 1.0f,

			-0.5f,  0.5f, -0.5f,    0.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
			0.5f,  0.5f, -0.5f,    0.0f, 1.0f,
			0.5f,  0.5f,  0.5f,    1.0f, 1.0f,
		};
		this->vertexBuffer = device->createBuffer({
			.size = sizeof(decltype(cubeVertecies)),
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
		});

		std::array cubeIndices{
			0, 1, 2, 1, 2, 3,
			4, 5, 6, 5, 6, 7,
			8, 9, 10, 9, 10, 11,
			12, 13, 14, 13, 14, 15,
			16, 17, 18, 17, 18, 19,
			20, 21, 22, 21, 22, 23,
		};
		this->indexBuffer = device->createBuffer({
			.size = sizeof(decltype(cubeIndices)), 
			.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
		});

		auto cmdList = device->getEmptyCommandList();
		cmdList->begin();
		cmdList->copyHostToBuffer({
			.src = cubeVertecies.data(),
			.dst = vertexBuffer,
			.size = sizeof(decltype(cubeVertecies)),
		});
		cmdList->copyHostToBuffer({
			.src = cubeIndices.data(),
			.dst = indexBuffer,
			.size = sizeof(decltype(cubeIndices)),
		});
		cmdList->copyHostToImageSynced({
			.dst = textureAtlas,
			.size = sizeX * sizeY * numChannels * sizeof(u8),
			.src = textureAtlasHostData,
		}, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		cmdList->end();
		queue->submitBlocking({
			.commandLists = {cmdList},
		});
		queue->checkForFinishedSubmits();

		this->uniformBuffer = device->createBuffer({
			.size = sizeof(glm::mat4),
			.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
		});

		depthImage = device->createImage2d({
			.width = app.window->getSize()[0],
			.height = app.window->getSize()[1],
			.format = VK_FORMAT_D32_SFLOAT,
			.imageAspekt = VK_IMAGE_ASPECT_DEPTH_BIT,
			.imageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
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
			depthImage = device->createImage2d({
				.width = app.window->getSize()[0],
				.height = app.window->getSize()[1],
				.format = VK_FORMAT_D32_SFLOAT,
				.imageAspekt = VK_IMAGE_ASPECT_DEPTH_BIT,
				.imageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				.memoryPropertyFlags = VMA_MEMORY_USAGE_GPU_ONLY,
			});
		}

		auto* currentFrame = &frames.front();

		auto cmdList = device->getEmptyCommandList();

		cmdList->begin();


		/// ------------ Begin Data Uploading ---------------------


		auto vp = cameraController.update(*app.window, app.getDeltaTimeSeconds());
		cmdList->copyHostToBuffer(daxa::gpu::HostToBufferCopyInfo{
			.src = &vp,
			.dst = uniformBuffer,
			.size = sizeof(decltype(vp)),
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
			.awaitedAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,				// wait for writing the uniform buffer
			.waitingStages = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT_KHR,		// the vertex shader must wait until uniform is written
		} };
		cmdList->insertBarriers(memBarrier0, imgBarrier0);
		
		
		/// ------------ End Data Uploading ---------------------

		std::array framebuffer{
			daxa::gpu::RenderAttachmentInfo{
				.image = swapchainImage.getImageHandle(),
				.clearValue = { .color = VkClearColorValue{.float32 = { 1.f, 1.f, 1.f, 1.f } } },
			}
		};
		daxa::gpu::RenderAttachmentInfo depthAttachment{
			.image = depthImage,
			.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.clearValue = { .depthStencil = VkClearDepthStencilValue{ .depth = 1.0f } },
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		};
		cmdList->beginRendering(daxa::gpu::BeginRenderingInfo{
			.colorAttachments = framebuffer,
			.depthAttachment = &depthAttachment,
		});
		
		cmdList->bindPipeline(pipeline);
		
		auto set = bindingSetAllocator->getSet();
		set->bindBuffer(0, uniformBuffer);
		set->bindImage(1, textureAtlas, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		cmdList->bindSet(0, set);
		
		cmdList->bindIndexBuffer(indexBuffer);

		cmdList->bindVertexBuffer(0, vertexBuffer);
		
		cmdList->drawIndexed(indexBuffer->getSize() / sizeof(u32), 1, 0, 0, 0);
		
		cmdList->endRendering();

		// array because we can allways pass multiple barriers at once for driver efficiency
		std::array imgBarrier1 = { daxa::gpu::ImageBarrier{
			.image = swapchainImage.getImageHandle(),
			.layoutBefore = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.layoutAfter = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		} };
		cmdList->insertBarriers({}, imgBarrier1);

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
	daxa::gpu::ImageHandle depthImage;
	daxa::gpu::PipelineHandle pipeline;
	daxa::gpu::BindingSetAllocatorHandle bindingSetAllocator;
	daxa::gpu::BufferHandle vertexBuffer;
	daxa::gpu::BufferHandle indexBuffer;
	daxa::gpu::BufferHandle uniformBuffer;
	daxa::gpu::ImageHandle textureAtlas;
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