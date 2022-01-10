#include <iostream>

#include <stb_image.h>

#include "Daxa.hpp"

#include "World.hpp"
#include "RenderContext.hpp"
#include "MeshRenderer.hpp"
#include "MeshLoading.hpp"
#include "cgltf.h"
#include "Components.hpp"

struct UIState {
	char loadFileTextBuf[256] = {};
};

class MyUser {
public:
	MyUser(daxa::AppState& app) 
		: renderCTX{ *app.window }
		, imageCache{ std::make_shared<daxa::ImageCache>(renderCTX->device) }
		, sceneLoader{ renderCTX->device, std::vector<std::filesystem::path>{"./", "./DaxaMeshview/"}, this->imageCache }
	{ 
		auto possiblePaths = std::vector<std::filesystem::path>{
			"./",
			"./DaxaMeshview/"
		};

		auto cmdList = renderCTX->device->getEmptyCommandList();
		cmdList->begin();

		imageCache->initDefaultTexture(cmdList);

		auto ret = sceneLoader.loadScene(cmdList, "deccerCube/SM_Deccer_Cubes_Textured.gltf", ecm);

		if (ret.isErr()) {
			std::cout << "loading error with message: " << ret.message() << std::endl;
		}

		auto textureAtlas = imageCache->get(
			{
				.path = "DaxaCube/atlas.png", 
				.samplerInfo = daxa::gpu::SamplerCreateInfo{
					.minFilter = VK_FILTER_NEAREST,
					.magFilter = VK_FILTER_NEAREST,
				}
			},
			cmdList
		);

		// end texture creation

		std::array cuveVertexPositions {
			/*positions*/		
			0.5f, -0.5f, -0.5f, 
			-0.5f, -0.5f, -0.5f,
			0.5f,  0.5f, -0.5f, 
			-0.5f,  0.5f, -0.5f,

			-0.5f, -0.5f,  0.5f,
			0.5f, -0.5f,  0.5f, 
			-0.5f,  0.5f,  0.5f,
			0.5f,  0.5f,  0.5f, 

			-0.5f, -0.5f, -0.5f,
			-0.5f, -0.5f,  0.5f,
			-0.5f,  0.5f, -0.5f,
			-0.5f,  0.5f,  0.5f,

			0.5f, -0.5f,  0.5f, 
			0.5f, -0.5f, -0.5f, 
			0.5f,  0.5f,  0.5f, 
			0.5f,  0.5f, -0.5f, 

			-0.5f, -0.5f, -0.5f,
			0.5f, -0.5f, -0.5f, 
			-0.5f, -0.5f,  0.5f,
			0.5f, -0.5f,  0.5f, 

			-0.5f,  0.5f, -0.5f,
			-0.5f,  0.5f,  0.5f,
			0.5f,  0.5f, -0.5f, 
			0.5f,  0.5f,  0.5f, 
		};
		auto positionVertexBuffer = renderCTX->device->createBuffer({
			.size = sizeof(decltype(cuveVertexPositions)),
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
		});

		std::array cubeVertexUVs {
			/*tex uv*/ 
			  0.0f, 0.0f,
			   1.0f, 0.0f,
			  0.0f, 1.0f,
			   1.0f, 1.0f,

			   0.0f, 0.0f,
			  1.0f, 0.0f,
			   0.0f, 1.0f,
			  1.0f, 1.0f,

			   0.0f, 0.0f,
			   1.0f, 0.0f,
			   0.0f, 1.0f,
			   1.0f, 1.0f,

			  0.0f, 0.0f,
			  1.0f, 0.0f,
			  0.0f, 1.0f,
			  1.0f, 1.0f,

			   0.0f, 0.0f,
			  0.0f, 1.0f,
			   1.0f, 0.0f,
			  1.0f, 1.0f,

			   0.0f, 0.0f,
			   1.0f, 0.0f,
			  0.0f, 1.0f,
			  1.0f, 1.0f,
		};
		auto uvVertexBuffer = renderCTX->device->createBuffer({
			.size = sizeof(decltype(cubeVertexUVs)),
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

		cmdList->copyHostToBuffer({
			.src = cuveVertexPositions.data(),
			.dst = positionVertexBuffer,
			.size = sizeof(decltype(cuveVertexPositions)),
		});
		cmdList->copyHostToBuffer({
			.src = cubeVertexUVs.data(),
			.dst = uvVertexBuffer,
			.size = sizeof(decltype(cubeVertexUVs)),
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

		ImGui::CreateContext();
		ImGui_ImplGlfw_InitForVulkan(app.window->getGLFWWindow(), true);
		imguiRenderer.emplace(renderCTX->device, renderCTX->queue);

		meshRender.init(*renderCTX);

		//auto ent = ecm.createEntity();
		//auto ent2 = ecm.createEntity();
//
		//auto modelMat = glm::mat4{1.0f};
		//modelMat = glm::translate(modelMat, glm::vec3{0,0,1});
		//modelMat = glm::rotate(modelMat, 45.f, glm::vec3{1,1,1});
//
		//auto view = ecm.view<daxa::TransformComp, ModelComp, ChildComp>();
		//view.addComp(ent, daxa::TransformComp{.translation = modelMat});
		//view.addComp(ent, ModelComp{ .image = textureAtlas, .vertexPositions = positionVertexBuffer, .vertexUVs = uvVertexBuffer, .indiexBuffer = indexBuffer, .indexCount = cubeIndices.size()});
//
		//view.addComp(ent2, daxa::TransformComp{.translation = modelMat});
		//view.addComp(ent2, ModelComp{ .image = textureAtlas, .vertexPositions = positionVertexBuffer, .vertexUVs = uvVertexBuffer, .indiexBuffer = indexBuffer, .indexCount = cubeIndices.size()});
		//view.addComp(ent2, ChildComp{ .parent = ent });
	}

	void update(daxa::AppState& app) {

		auto cmdList = renderCTX->device->getEmptyCommandList();

		cmdList->begin();

		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		ImGui::Begin("file import");
		ImGui::InputText("file path", uiState.loadFileTextBuf, sizeof(uiState.loadFileTextBuf));
		if (ImGui::Button("load")) {
			printf("try to load model with path: %s\n", uiState.loadFileTextBuf);

			sceneLoader.loadScene(cmdList, "frog/", ecm);
			std::memset(uiState.loadFileTextBuf, '\0', sizeof(uiState.loadFileTextBuf));
		}
		ImGui::End();

		ImGui::Begin("texture view");
		for (auto tex : sceneLoader.textures) {
			auto id = imguiRenderer->getImGuiTextureId(tex);
			ImGui::Text("pointer = %i", id);
			ImGui::Text("size = %d x %d", tex->getVkExtent().width, tex->getVkExtent().height);
			ImGui::Image((void*)id, ImVec2(400,400));
		}
		ImGui::End();

		if (!ImGui::GetIO().WantCaptureMouse) {
			cameraController.processInput(*app.window, app.getDeltaTimeSeconds());
		}

		ImGui::Render();

		if (app.window->getWidth() != renderCTX->swapchain->getSize().width || app.window->getHeight() != renderCTX->swapchain->getSize().height) {
			renderCTX->resize(app.window->getWidth(),app.window->getHeight());
		}

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
			auto childComps = ecm.view<ChildComp>();

			for (auto [ent, trans, model] : view) {
				transformBuffer.clear();
				transformBuffer.push_back(trans.mat);
				auto iterEnt = ent;
				while (ChildComp* childComp = childComps.getCompIf<ChildComp>(iterEnt)) {
					transformBuffer.push_back(view.getComp<daxa::TransformComp>(childComp->parent).mat);
					iterEnt = childComp->parent;
				}

				auto translation = glm::mat4{1.0f};
				for (int i = 0; i < transformBuffer.size(); i++) {
					translation =  transformBuffer[i] * translation;
				}

				//if (!childComps.hasComp<ChildComp>(ent)) {
				//	trans.translation = glm::rotate(trans.translation, 1.f * (f32)app.getDeltaTimeSeconds(), glm::vec3{1,1,1});
				//}
				for (auto& prim : model.meshes) {
					draws.push_back(MeshRenderer::DrawMesh{
						.albedo = prim.image,
						.indexCount = prim.indexCount,
						.indices = prim.indiexBuffer,
						.positions = prim.vertexPositions,
						.uvs = prim.vertexUVs,
						.transform = translation,
					});
				}
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
	std::vector<glm::mat4> transformBuffer;
	std::optional<RenderContext> renderCTX;
	daxa::GimbalLockedCameraController cameraController{};
	MeshRenderer meshRender = {};
	std::shared_ptr<daxa::ImageCache> imageCache;
	SceneLoader sceneLoader;
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