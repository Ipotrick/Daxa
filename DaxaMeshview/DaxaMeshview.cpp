#include <iostream>

#include <stb_image.h>

#include "Daxa.hpp"

#include "World.hpp"
#include "RenderContext.hpp"
#include "MeshRenderer.hpp"

struct UIState {
	char loadFileTextBuf[256] = {};
};

struct ModelComp {
	daxa::gpu::BufferHandle indiexBuffer = {};
	daxa::gpu::BufferHandle vertexBuffer = {};
	daxa::gpu::ImageHandle image = {};
	u32 indexCount = 0;
};

class MyUser {
public:
	MyUser(daxa::AppState& app) 
		: renderCTX{ *app.window }
		, imageCache{ renderCTX->device }
	{ 

		auto textureAtlas = imageCache.get({
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
		auto vertexBuffer = renderCTX->device->createBuffer({
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
		auto indexBuffer = renderCTX->device->createBuffer({
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

		auto uniformBuffer = renderCTX->device->createBuffer({
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

		auto ent = ecm.createEntity();

		auto view = ecm.view<daxa::TransformComp, ModelComp>();
		view.addComp(ent, daxa::TransformComp{.translation = glm::mat4{1.0f}});
		view.addComp(ent, ModelComp{ .image = textureAtlas, .vertexBuffer = vertexBuffer, .indiexBuffer = indexBuffer, .indexCount = cubeIndices.size()});
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
		meshRender.setCameraVP(cmdList, vp);

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

		{
			auto draws = std::vector<MeshRenderer::DrawMesh>();
			auto view = ecm.view<daxa::TransformComp, ModelComp>();

			for (auto [ent, trans, model] : view) {
				draws.push_back(MeshRenderer::DrawMesh{
					.albedo = model.image,
					.indexCount = model.indexCount,
					.indices = model.indiexBuffer,
					.vertices = model.vertexBuffer,
					.transform = trans.translation,
				});
			}
			meshRender.render(*renderCTX, cmdList, draws);
		}

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
	daxa::ImageCache imageCache;
	std::optional<daxa::ImGuiRenderer> imguiRenderer = std::nullopt;
	double totalElapsedTime = 0.0f;
	UIState uiState;
	daxa::EntityComponentManager ecm;
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