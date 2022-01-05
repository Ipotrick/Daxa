#include <iostream>

#include <stb_image.h>

#include "Daxa.hpp"

#include "World.hpp"
#include "RenderContext.hpp"
#include "MeshRenderer.hpp"

struct UIState {
	char loadFileTextBuf[256] = {};
};

class MyUser {
public:
	MyUser(daxa::AppState& app) 
		: renderCTX{ *app.window }
		, imageCache{ renderCTX->device }
	{ 
		auto mesh = daxa::Mesh::tryLoadFromGLTF2("DaxaMeshview/frog/scene.gltf").value();
		
		auto ecm = daxa::EntityComponentManager{};
		auto ent0 = ecm.createEntity();
		auto ent1 = ecm.createEntity();
		auto ent2 = ecm.createEntity();
		auto view = ecm.view<daxa::TransformComp>();
		view.addComp(ent0, daxa::TransformComp{.translation= {}});
		view.addComp(ent1, daxa::TransformComp{.translation= {}});
		view.addComp(ent2, daxa::TransformComp{.translation= {}});
		for (auto [entity, transform] : view) {
			printf("entity with index: %i and version: %i has a transform\n", entity.index, entity.version);
		}

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

		daxa::gpu::ShaderModuleHandle vertexShader = renderCTX->device->tryCreateShderModuleFromGLSL(
			vertexShaderGLSL,
			VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT
		).value();

		daxa::gpu::ShaderModuleHandle fragmenstShader = renderCTX->device->tryCreateShderModuleFromGLSL(
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
			.addColorAttachment(renderCTX->swapchain->getVkFormat());

		this->pipeline = renderCTX->device->createGraphicsPipeline(pipelineBuilder);

		this->bindingSetAllocator = renderCTX->device->createBindingSetAllocator(pipeline->getSetDescription(0));

		textureAtlas = imageCache.get({
			.path = "DaxaCube/atlas.png", 
			.samplerInfo = daxa::gpu::SamplerCreateInfo{
				.minFilter = VK_FILTER_NEAREST,
				.magFilter = VK_FILTER_NEAREST,
			}
		});
		renderCTX->queue->submitBlocking({.commandLists = {imageCache.getUploadCommands()}});

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
		this->vertexBuffer = renderCTX->device->createBuffer({
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
		this->indexBuffer = renderCTX->device->createBuffer({
			.size = sizeof(decltype(cubeIndices)), 
			.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
		});

		auto cmdList = renderCTX->device->getEmptyCommandList();
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
		cmdList->end();
		renderCTX->queue->submitBlocking({
			.commandLists = {cmdList},
		});
		renderCTX->queue->checkForFinishedSubmits();

		this->uniformBuffer = renderCTX->device->createBuffer({
			.size = sizeof(glm::mat4),
			.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
		});

		renderCTX->depthImage = renderCTX->device->createImage2d({
			.width = app.window->getWidth(),
			.height = app.window->getHeight(),
			.format = VK_FORMAT_D32_SFLOAT,
			.imageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			.imageAspekt = VK_IMAGE_ASPECT_DEPTH_BIT,
		});

		ImGui::CreateContext();
		ImGui_ImplGlfw_InitForVulkan(app.window->getGLFWWindow(), true);
		imguiRenderer.emplace(renderCTX->device, renderCTX->queue);

		meshRender.init(*renderCTX);
	}

	void update(daxa::AppState& app) {

		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		ImGui::Begin("file import");
		ImGui::InputText("file path", uiState.loadFileTextBuf, sizeof(uiState.loadFileTextBuf));
		if (ImGui::Button("load")) {
			printf("try to load model with path: %s\n", uiState.loadFileTextBuf);
			std::memset(uiState.loadFileTextBuf, '\0', sizeof(uiState.loadFileTextBuf));
		}
		ImGui::End();

		if (!ImGui::GetIO().WantCaptureMouse) {
			cameraController.processInput(*app.window, app.getDeltaTimeSeconds());
		}

		ImGui::Render();

		if (app.window->getWidth() != renderCTX->swapchain->getSize().width || app.window->getHeight() != renderCTX->swapchain->getSize().height) {
			renderCTX->resize(app.window->getWidth(),app.window->getHeight());
		}

		auto cmdList = renderCTX->device->getEmptyCommandList();

		cmdList->begin();


		/// ------------ Begin Data Uploading ---------------------


		auto vp = cameraController.getVP(*app.window);
		cmdList->copyHostToBuffer(daxa::gpu::HostToBufferCopyInfo{
			.src = &vp,
			.dst = uniformBuffer,
			.size = sizeof(decltype(vp)),
		});

		// array because we can allways pass multiple barriers at once for driver efficiency
		std::array imgBarrier0 = { daxa::gpu::ImageBarrier{
			.waitingStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,	// as we write to the image in the frag shader we need to make sure its finished transitioning the layout
			.image = renderCTX->swapchainImage.getImageHandle(),
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
				.image = renderCTX->swapchainImage.getImageHandle(),
				.clearValue = { .color = VkClearColorValue{.float32 = { 1.f, 1.f, 1.f, 1.f } } },
			}
		};
		daxa::gpu::RenderAttachmentInfo depthAttachment{
			.image = renderCTX->depthImage,
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

		cmdList->insertMemoryBarrier({
			.awaitedStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
			.waitingStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
		});

		imguiRenderer->recordCommands(ImGui::GetDrawData(), cmdList, renderCTX->swapchainImage.getImageHandle());

		// array because we can allways pass multiple barriers at once for driver efficiency
		std::array imgBarrier1 = { daxa::gpu::ImageBarrier{
			.awaitedStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
			.image = renderCTX->swapchainImage.getImageHandle(),
			.layoutBefore = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.layoutAfter = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		} };
		cmdList->insertBarriers({}, imgBarrier1);

		cmdList->end();

		daxa::gpu::SubmitInfo submitInfo;
		submitInfo.commandLists.push_back(std::move(cmdList));
		submitInfo.signalOnCompletion = { &renderCTX->presentSignal, 1 };
		renderCTX->queue->submit(submitInfo);

		renderCTX->present();
	}

	void cleanup(daxa::AppState& app) {
		renderCTX->waitIdle();
		imguiRenderer.reset();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	~MyUser() {

	}
private:
	std::optional<RenderContext> renderCTX;
	daxa::GimbalLockedCameraController cameraController{};
	MeshRenderer meshRender = {};
	daxa::gpu::PipelineHandle pipeline;
	daxa::gpu::BindingSetAllocatorHandle bindingSetAllocator;
	daxa::gpu::BufferHandle vertexBuffer;
	daxa::gpu::BufferHandle indexBuffer;
	daxa::gpu::BufferHandle uniformBuffer;
	daxa::gpu::ImageHandle textureAtlas;
	daxa::ImageCache imageCache;
	std::optional<daxa::ImGuiRenderer> imguiRenderer = std::nullopt;
	double totalElapsedTime = 0.0f;
	UIState uiState;
};

int main()
{
	daxa::initialize();

	{
		daxa::Application<MyUser> app{ 1000, 1000, "Daxa Dear ImGui Sample"};
		app.run();
	}

	daxa::cleanup();
}