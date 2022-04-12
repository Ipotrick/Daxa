#include <iostream>

// #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define GLM_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Daxa.hpp"

class GimbalLockedCameraController {
public:
	void processInput(daxa::Window& window, f32 dt) {
		f32 speed = window.keyPressed(GLFW_KEY_LEFT_SHIFT) ? translationSpeed * 4.0f : translationSpeed;
		if (window.isCursorCaptured()) {
			if (window.keyJustPressed(GLFW_KEY_ESCAPE)) {
				window.releaseCursor();
			}
		} else {
			if (window.buttonJustPressed(GLFW_MOUSE_BUTTON_LEFT) && window.isCursorOverWindow()) {
				window.captureCursor();
			}
		}

		auto cameraSwaySpeed = this->cameraSwaySpeed;
		if (window.keyPressed(GLFW_KEY_C)) {
			cameraSwaySpeed *= 0.25;
			bZoom = true;
		} else {
			bZoom = false;
		}

		auto yawRotaAroundUp = glm::rotate(glm::mat4(1.0f), yaw, {0.f,0.f,1.f});
		auto pitchRotation = glm::rotate(glm::mat4(1.0f), pitch, glm::vec3{1.f,0.f,0.f});
		glm::vec4 translation = {};
		if (window.keyPressed(GLFW_KEY_W)) {
			glm::vec4 direction = { 0.0f, 0.0f, -1.0f, 0.0f };
			translation += yawRotaAroundUp * pitchRotation * direction * dt * speed;
		}
		if (window.keyPressed(GLFW_KEY_S)) {
			glm::vec4 direction = { 0.0f, 0.0f, 1.0f, 0.0f };
			translation += yawRotaAroundUp * pitchRotation * direction * dt * speed;
		}
		if (window.keyPressed(GLFW_KEY_A)) {
			glm::vec4 direction = { 1.0f, 0.0f, 0.0f, 0.0f };
			translation += yawRotaAroundUp * direction * dt * speed;
		}
		if (window.keyPressed(GLFW_KEY_D)) {
			glm::vec4 direction = { -1.0f, 0.0f, 0.0f, 0.0f };
			translation += yawRotaAroundUp * direction * dt * speed;
		}
		if (window.keyPressed(GLFW_KEY_SPACE)) {
			translation += yawRotaAroundUp * pitchRotation * glm::vec4{ 0.f,  1.f, 0.f, 0.f } * dt * speed;
		}
		if (window.keyPressed(GLFW_KEY_LEFT_CONTROL)) {
			translation += yawRotaAroundUp * pitchRotation * glm::vec4{ 0.f, -1.f,  0.f, 0.f } * dt * speed;
		}
		if (window.isCursorCaptured()) {
			pitch -= window.getCursorPosChangeY() * cameraSwaySpeed * dt;
			pitch = std::clamp(pitch, 0.0f, glm::pi<f32>());
			yaw += window.getCursorPosChangeX() * cameraSwaySpeed * dt;
		}
		position += translation;
	}

	glm::mat4 getVP(daxa::Window& window) const {
		auto fov = this->fov;
		if (bZoom) {
			fov *= 0.25f;
		}
		auto yawRotaAroundUp = glm::rotate(glm::mat4(1.0f), yaw, {0.f,0.f,1.f});
		auto pitchRotation = glm::rotate(glm::mat4(1.0f), pitch, glm::vec3{1.f,0.f,0.f});
		auto prespective = glm::perspective(fov, (f32)window.getWidth()/(f32)window.getHeight(), near, far);
		auto rota = yawRotaAroundUp * pitchRotation;
		auto cameraModelMat = glm::translate(glm::mat4(1.0f), {position.x, position.y, position.z}) * rota;
		auto view = glm::inverse(cameraModelMat);
		return prespective * view;
	}
private:
	bool bZoom = false;
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
		: device{ daxa::Device::create() }
		, queue{ this->device->createQueue({.batchCount = 2 })}
		, swapchain{ this->device->createSwapchain({
			.surface = app.window->getSurface(),
			.width = app.window->getWidth(),
			.height = app.window->getHeight(),
		})}
		, swapchainImage{ this->swapchain->aquireNextImage() }
		, presentSignal{ this->device->createSignal({}) }
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

		daxa::ShaderModuleHandle vertexShader = device->createShaderModule({
			.sourceToGLSL = vertexShaderGLSL,
			.stage = VK_SHADER_STAGE_VERTEX_BIT
		}).value();

		daxa::ShaderModuleHandle fragmenstShader = device->createShaderModule({
			.sourceToGLSL = fragmentShaderGLSL,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT
		}).value();

		daxa::GraphicsPipelineBuilder pipelineBuilder;
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

		this->bindingSetAllocator = device->createBindingSetAllocator({ .setDescription = pipeline->getSetDescription(0) });

		// Begin texture creation

		stbi_set_flip_vertically_on_load(true);
		i32 sizeX, sizeY, numChannels;
		char const* const filepath = "DaxaCube/atlas.png";
		std::uint8_t * textureAtlasHostData = stbi_load(filepath, &sizeX, &sizeY, &numChannels, 0);
		VkFormat dataFormat;
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

		auto cmdList = device->getCommandList();
		cmdList.copyHostToBuffer({
			.src = cubeVertecies.data(),
			.dst = vertexBuffer,
			.size = sizeof(decltype(cubeVertecies)),
		});
		cmdList.copyHostToBuffer({
			.src = cubeIndices.data(),
			.dst = indexBuffer,
			.size = sizeof(decltype(cubeIndices)),
		});
		cmdList.copyHostToImageSynced({
			.dst = textureAtlas,
			.size = sizeX * sizeY * numChannels * sizeof(u8),
			.src = textureAtlasHostData,
			.dstFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		});
		cmdList.finalize();
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
			.width = app.window->getWidth(),
			.height = app.window->getHeight(),
			.format = VK_FORMAT_D32_SFLOAT,
			.imageAspekt = VK_IMAGE_ASPECT_DEPTH_BIT,
			.imageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		});
	}

	void update(daxa::AppState& app) {
		if (app.window->getWidth() != swapchain->getSize().width || app.window->getHeight() != swapchain->getSize().height) {
			device->waitIdle();
			swapchain->resize(VkExtent2D{ .width = app.window->getWidth(), .height = app.window->getHeight() });
			swapchainImage = swapchain->aquireNextImage();
			depthImage = device->createImage2d({
				.width = app.window->getWidth(),
				.height = app.window->getHeight(),
				.format = VK_FORMAT_D32_SFLOAT,
				.imageAspekt = VK_IMAGE_ASPECT_DEPTH_BIT,
				.imageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				.memoryPropertyFlags = VMA_MEMORY_USAGE_GPU_ONLY,
			});
		}

		auto cmdList = device->getCommandList();

		/// ------------ Begin Data Uploading ---------------------

		cameraController.processInput(*app.window, app.getDeltaTimeSeconds());
		auto vp = cameraController.getVP(*app.window);
		cmdList.copyHostToBuffer(daxa::HostToBufferCopyInfo{
			.src = &vp,
			.dst = uniformBuffer,
			.size = sizeof(decltype(vp)),
		});

		// array because we can allways pass multiple barriers at once for driver efficiency
		std::array imgBarrier0 = { daxa::ImageBarrier{
			.dstStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,	// as we write to the image in the frag shader we need to make sure its finished transitioning the layout
			.image = swapchainImage.getImageHandle(),
			.layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,						// dont care about previous layout
			.layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,		// set new layout to color attachment optimal
		} };
		// array because we can allways pass multiple barriers at once for driver efficiency
		std::array memBarrier0 = { daxa::MemoryBarrier{
			.srcAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,				// wait for writing the uniform buffer
			.dstStages = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT_KHR,		// the vertex shader must wait until uniform is written
		} };
		cmdList.insertBarriers(memBarrier0, imgBarrier0);
		
		
		/// ------------ End Data Uploading ---------------------

		std::array framebuffer{
			daxa::RenderAttachmentInfo{
				.image = swapchainImage.getImageHandle(),
				.clearValue = { .color = VkClearColorValue{.float32 = { 1.f, 1.f, 1.f, 1.f } } },
			}
		};
		daxa::RenderAttachmentInfo depthAttachment{
			.image = depthImage,
			.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.clearValue = { .depthStencil = VkClearDepthStencilValue{ .depth = 1.0f } },
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		};
		cmdList.beginRendering(daxa::BeginRenderingInfo{
			.colorAttachments = framebuffer,
			.depthAttachment = &depthAttachment,
		});
		
		cmdList.bindPipeline(pipeline);
		
		auto set = bindingSetAllocator->getSet();
		set->bindBuffer(0, uniformBuffer);
		set->bindImage(1, textureAtlas, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		cmdList.bindSet(0, set);
		
		cmdList.bindIndexBuffer(indexBuffer);

		cmdList.bindVertexBuffer(0, vertexBuffer);
		
		cmdList.drawIndexed(indexBuffer.getSize() / sizeof(u32), 1, 0, 0, 0);
		
		cmdList.endRendering();

		// array because we can allways pass multiple barriers at once for driver efficiency
		std::array imgBarrier1 = { daxa::ImageBarrier{
			.image = swapchainImage.getImageHandle(),
			.layoutBefore = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.layoutAfter = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		} };
		cmdList.insertBarriers({}, imgBarrier1);

		cmdList.finalize();

		daxa::SubmitInfo submitInfo;
		submitInfo.commandLists.push_back(std::move(cmdList));
		submitInfo.signalOnCompletion = { &presentSignal, 1 };
		queue->submit(submitInfo);

		queue->present(std::move(swapchainImage), presentSignal);
		swapchainImage = swapchain->aquireNextImage();

		// switches to the next batch. If all available batches are still beeing executed, 
		// this function will block until there is a batch available again.
		// the batches are reclaimed in order.
		queue->nextBatch();

		// reclaim ressources and free ref count handles
		queue->checkForFinishedSubmits();
	}

	void cleanup(daxa::AppState& app) {
		queue->waitIdle();
		queue->checkForFinishedSubmits();
		device->waitIdle();
	}

	~MyUser() {

	}
private:
	GimbalLockedCameraController cameraController{};
	daxa::DeviceHandle device;
	daxa::QueueHandle queue;
	daxa::SwapchainHandle swapchain;
	daxa::SwapchainImage swapchainImage;
	daxa::ImageHandle depthImage;
	daxa::PipelineHandle pipeline;
	daxa::BindingSetAllocatorHandle bindingSetAllocator;
	daxa::BufferHandle vertexBuffer;
	daxa::BufferHandle indexBuffer;
	daxa::BufferHandle uniformBuffer;
	daxa::ImageHandle textureAtlas;
	daxa::SignalHandle presentSignal;
	double totalElapsedTime = 0.0f;
};

int main()
{
	daxa::initialize();

	{
		daxa::Application<MyUser> app{ 1000, 1000, "Daxa Cube Sample"};
		app.run();
	}

	daxa::cleanup();
}