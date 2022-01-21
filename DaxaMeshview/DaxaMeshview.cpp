#include <iostream>

#include <stb_image.h>

#include "Daxa.hpp"
#include <glm/gtx/matrix_decompose.hpp>

#include "World.hpp"
#include "renderer/MeshRenderer.hpp"
#include "renderer/FrameBufferDebugRenderer.hpp"
#include "MeshLoading.hpp"
#include "cgltf.h"
#include "Components.hpp"

struct UIState {
	char loadFileTextBuf[256] = {};
	bool convertYtoZup = false;
};

class MyUser {
public:
	MyUser(daxa::AppState& app) 
		: renderCTX{ *app.window }
		, imageCache{ std::make_shared<daxa::ImageCache>(renderCTX.device) }
		, sceneLoader{ renderCTX.device, std::vector<std::filesystem::path>{"./", "./DaxaMeshview/assets/", "./assets/"}, this->imageCache }
	{ 
		ImGui::CreateContext();
		ImGui_ImplGlfw_InitForVulkan(app.window->getGLFWWindow(), true);
		imguiRenderer.emplace(renderCTX.device, renderCTX.queue);

		meshRender.init(renderCTX);
		frameBufferDebugRenderer.init(renderCTX, app.window->getWidth(), app.window->getHeight());

		auto lightEnt = ecm.createEntity();
		auto view = ecm.view<daxa::TransformComp, LightComp>();
		view.addComp(lightEnt, LightComp{ .color = { 0.8f, 0.8, 0.8f, 1.0f }, .strength = 6.25 });
		view.addComp(lightEnt, daxa::TransformComp{ .mat = glm::translate(glm::mat4{1.0f}, glm::vec3(0,0,3)) });
	}

	void update(daxa::AppState& app) {
		auto cmdList = renderCTX.device->getCommandList();

		ImGui_ImplGlfw_NewFrame(); 

		ImGui::NewFrame();

		ImGui::Begin("file import");
		ImGui::InputText("file path", uiState.loadFileTextBuf, sizeof(uiState.loadFileTextBuf));
		ImGui::Checkbox("convert y-up to z-up", &uiState.convertYtoZup);
		if (ImGui::Button("load")) {
			printf("try to load model with path: %s\n", uiState.loadFileTextBuf);

			auto ret = sceneLoader.loadScene(cmdList, uiState.loadFileTextBuf, ecm, uiState.convertYtoZup);
			if (ret.isErr()) {
				printf("failed to load scene, due to error: %s\n", ret.message().c_str());
			}
			std::memset(uiState.loadFileTextBuf, '\0', sizeof(uiState.loadFileTextBuf));
		}
		ImGui::End();

		ImGui::Begin("texture view");
		for (auto [key, tex] : imageCache->cache) {
			auto id = imguiRenderer->getImGuiTextureId(tex);
			ImGui::Text("pointer = %i", id);
			ImGui::Text("size = %d x %d", tex->getVkExtent().width, tex->getVkExtent().height);
			ImGui::Image((void*)id, ImVec2(400,400));
		}
		ImGui::End();

		frameBufferDebugRenderer.doGui(*imguiRenderer);

		if (!ImGui::GetIO().WantCaptureMouse) {
			cameraController.processInput(*app.window, app.getDeltaTimeSeconds());
		}

		ImGui::Render();

		if (app.window->getWidth() != renderCTX.swapchain->getSize().width || app.window->getHeight() != renderCTX.swapchain->getSize().height) {
			renderCTX.resize(cmdList, app.window->getWidth(), app.window->getHeight());
			frameBufferDebugRenderer.recreateImages(renderCTX, cmdList, app.window->getWidth(), app.window->getHeight());
		}

		cameraController.updateMatrices(*app.window);
		meshRender.setCamera(cmdList, cameraController.vp, cameraController.view);
		
		cmdList->insertImageBarrier({
			.barrier = {
				.dstStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
				.dstAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
			},
			.image = renderCTX.swapchainImage.getImageHandle(),
			.layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		});

		{
			auto lightDraws = std::vector<MeshRenderer::DrawLight>();
			auto view = ecm.view<LightComp, daxa::TransformComp>();

			for (auto [ent, light, trans] : view) {
				glm::vec3 scale;
				glm::quat rotation;
				glm::vec3 translation;
				glm::vec3 skew;
				glm::vec4 perspective;
				glm::decompose(trans.mat, scale, rotation, translation, skew, perspective);
				lightDraws.push_back(MeshRenderer::DrawLight{
					.position = translation,
					.strength = light.strength,
					.color = light.color,
				});
			}
			meshRender.setLights(lightDraws);
		}
		
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
						.transform = translation,
						.prim = &prim,
					});
				}
			}
			meshRender.render(renderCTX, cmdList, draws);
		}

		auto upload = FrameBufferDebugRenderer::CameraData{
			.far = cameraController.far,
			.near = cameraController.near,
			.inverseView = glm::inverse(cameraController.view),
			.inverseTransposeVP = glm::inverse(glm::transpose(cameraController.view)),
		};
		frameBufferDebugRenderer.renderDebugViews(renderCTX, cmdList, upload);

		cmdList->insertMemoryBarrier({
			.srcStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
			.dstStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
		});

		imguiRenderer->recordCommands(ImGui::GetDrawData(), cmdList, renderCTX.swapchainImage.getImageHandle());

		// array because we can allways pass multiple barriers at once for driver efficiency
		std::array imgBarrier1 = { daxa::gpu::ImageBarrier{
			.barrier = {
				.srcStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
				.srcAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
			},
			.image = renderCTX.swapchainImage.getImageHandle(),
			.layoutBefore = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.layoutAfter = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		} };
		cmdList->insertBarriers({}, imgBarrier1);

		cmdList->finalize();

		daxa::gpu::SubmitInfo submitInfo;
		submitInfo.commandLists.push_back(std::move(cmdList));
		submitInfo.signalOnCompletion = { &renderCTX.presentSignal, 1 };
		renderCTX.queue->submit(submitInfo);

		renderCTX.present();
		//printf("frame\n");
	}

	void cleanup(daxa::AppState& app) {
		renderCTX.waitIdle();
		imguiRenderer.reset();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	~MyUser() {

	}

private:
	std::vector<glm::mat4> transformBuffer;
	RenderContext renderCTX;
	MeshRenderer meshRender = {};
	FrameBufferDebugRenderer frameBufferDebugRenderer = {};
	daxa::GimbalLockedCameraController cameraController{};
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