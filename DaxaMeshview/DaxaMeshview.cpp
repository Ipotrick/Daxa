#include <iostream>

#include <stb_image.h>

#include "Daxa.hpp"
#include <glm/gtx/matrix_decompose.hpp>

#include "World.hpp"
#include "renderer/MeshRenderer.hpp"
#include "renderer/FrameBufferDebugRenderer.hpp"
#include "AssetServer.hpp"
#include "cgltf.h"
#include "Components.hpp"
#include "AssetServer.hpp"
#include "renderer/fft.hpp"

struct UIState {
	char loadFileTextBuf[256] = "sponza/Sponza.gltf";
	int xAmount = 5;
	int yAmount = 5;
	int xDistance = 30;
	int yDistance = 20;
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
		imguiRenderer.emplace(renderCTX.device, renderCTX.queue, renderCTX.pipelineCompiler);

		meshRender.init(renderCTX);
		frameBufferDebugRenderer.init(renderCTX, app.window->getWidth(), app.window->getHeight());

		f32 step = 20.0f;
		for (int x = 0; x < 0; x++) {
			for (int y = 0; y < 1; y++) {
				auto lightEnt = ecm.createEntity();
				auto view = ecm.view<daxa::TransformComp, LightComp>();
				view.addComp(lightEnt, LightComp{ .strength = 6.25, .color = { 0.8f, 0.8, 0.8f, 1.0f } });
				view.addComp(lightEnt, daxa::TransformComp{ .mat = glm::translate(glm::mat4{1.0f}, glm::vec3(step*x,1,step*y)) });
			}
		}

		auto cmd = renderCTX.queue->getCommandList({});
		cmd.finalize();
		renderCTX.queue->submit({.commandLists = { cmd }});
	}

	void update(daxa::AppState& app) {

		if (app.window->getWidth() == 0 || app.window->getHeight() == 0) {
			return;
		}
		std::string cmdName = "cmd list nr. ";
		cmdName += std::to_string(frame);
		auto cmdList = renderCTX.queue->getCommandList({.debugName = cmdName.c_str()});
		frame++;
		ImGui_ImplGlfw_NewFrame(); 

		ImGui::NewFrame();

		ImGui::Begin("file import");
		ImGui::InputText("file path", uiState.loadFileTextBuf, sizeof(uiState.loadFileTextBuf));
		ImGui::SliderInt("x copy", &uiState.xAmount, 1, 100);
		ImGui::SliderInt("y copy", &uiState.yAmount, 1, 100);
		ImGui::SliderInt("x distance", &uiState.xDistance, 1, 100);
		ImGui::SliderInt("y distance", &uiState.yDistance, 1, 100);
		if (ImGui::Button("load")) {
			printf("try to load model with path: %s\n", uiState.loadFileTextBuf);
			for (int x = 0; x < uiState.xAmount; x++) {
				for (int y = 0; y < uiState.yAmount; y++) {
					auto ret = sceneLoader.getScene(cmdList, uiState.loadFileTextBuf, ecm);
					if (ret.isErr()) {
						printf("failed to load scene, due to error: %s\n", ret.message().c_str());
						break;
					}
					auto view = ecm.view<daxa::TransformComp>();
					view.getComp<daxa::TransformComp>(ret.value()).mat = glm::translate(glm::mat4{1.0f}, glm::vec3{x*uiState.xDistance,0,y*uiState.yDistance});
				}
			}

			std::memset(uiState.loadFileTextBuf, '\0', sizeof(uiState.loadFileTextBuf));
		}
		ImGui::End();

		ImGui::Begin("texture view");
		for (auto [key, tex] : imageCache->cache) {
			auto id = imguiRenderer->getImGuiTextureId(tex);
			ImGui::Text("pointer = %llu", id);
			ImGui::Text("size = %d x %d", tex->getImageHandle()->getVkExtent3D().width, tex->getImageHandle()->getVkExtent3D().height);
			ImGui::Image(reinterpret_cast<void*>(id), ImVec2(400,400));
		}
		ImGui::End();
		ImGui::Begin("fft debug");
		{
			ImGui::Text("fftImage");
			auto id = imguiRenderer->getImGuiTextureId(meshRender.fft.fftImage);
			ImGui::Image(reinterpret_cast<void*>(id), ImVec2(512,512));
			ImGui::Text("horFreqImageRG");
			id = imguiRenderer->getImGuiTextureId(meshRender.fft.horFreqImageRG);
			ImGui::Image(reinterpret_cast<void*>(id), ImVec2(512,512));
			ImGui::Text("horFreqImageBA");
			id = imguiRenderer->getImGuiTextureId(meshRender.fft.horFreqImageBA);
			ImGui::Image(reinterpret_cast<void*>(id), ImVec2(512,512));
			ImGui::Text("fullFreqImageRG");
			id = imguiRenderer->getImGuiTextureId(meshRender.fft.fullFreqImageRG);
			ImGui::Image(reinterpret_cast<void*>(id), ImVec2(512,512));
			ImGui::Text("fullFreqImageBA");
			id = imguiRenderer->getImGuiTextureId(meshRender.fft.fullFreqImageBA);
			ImGui::Image(reinterpret_cast<void*>(id), ImVec2(512,512));
			ImGui::Text("aperatureImage");
			id = imguiRenderer->getImGuiTextureId(meshRender.fft.aperatureImage);
			ImGui::Image(reinterpret_cast<void*>(id), ImVec2(512,512));
			ImGui::Text("kernelImage");
			id = imguiRenderer->getImGuiTextureId(meshRender.fft.kernelImage);
			ImGui::Image(reinterpret_cast<void*>(id), ImVec2(512,512));
			ImGui::Text("kernel");
			id = imguiRenderer->getImGuiTextureId(meshRender.fft.kernel);
			ImGui::Image(reinterpret_cast<void*>(id), ImVec2(512,512));
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
		meshRender.setCamera(cmdList, cameraController.proj, cameraController.view, cameraController.fov, cameraController.position);
		
		cmdList.queueImageBarrier({
			.barrier = {
				.dstStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
				.dstAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
			},
			.image = renderCTX.swapchainImage.getImageViewHandle(),
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
			meshDrawCommands.clear();
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
				for (size_t i = 0; i < transformBuffer.size(); i++) {
					translation =  transformBuffer[i] * translation;
				}

				//if (!childComps.hasComp<ChildComp>(ent)) {F
				//	trans.translation = glm::rotate(trans.translation, 1.f * (f32)app.getDeltaTimeSeconds(), glm::vec3{1,1,1});
				//}
				for (auto& prim : model.meshes) {
					meshDrawCommands.push_back(DrawPrimCmd{
						.transform = translation,
						.prim = &prim,
					});
				}
			}
			meshRender.render(renderCTX, cmdList, meshDrawCommands);
		}

		auto upload = FrameBufferDebugRenderer::CameraData{
			.inverseView = glm::inverse(cameraController.view),
			.inverseTransposeVP = glm::inverse(glm::transpose(cameraController.view)),
			.near = cameraController.near,
			.far = cameraController.far,
		};
		frameBufferDebugRenderer.renderDebugViews(renderCTX, cmdList, upload);
		cmdList.queueMemoryBarrier({
			.srcStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
			.dstStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
		});

		imguiRenderer->recordCommands(ImGui::GetDrawData(), cmdList, renderCTX.swapchainImage.getImageViewHandle());
		cmdList.queueImageBarrier({
			.barrier = {
				.srcStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
				.srcAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
			},
			.image = renderCTX.swapchainImage.getImageViewHandle(),
			.layoutBefore = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.layoutAfter = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		});

		cmdList.finalize();

		daxa::SubmitInfo submitInfo;
		submitInfo.commandLists.push_back(std::move(cmdList));
		submitInfo.signalOnCompletion = { &renderCTX.presentSignal, 1 };
		renderCTX.queue->submit(submitInfo);

		renderCTX.present();
	}

	void cleanup(daxa::AppState&) {
		renderCTX.waitIdle();
		imguiRenderer.reset();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	~MyUser() {

	}

private:
	size_t frame = 0;	
	std::vector<glm::mat4> transformBuffer;
	RenderContext renderCTX;
	MeshRenderer meshRender = {};
	FrameBufferDebugRenderer frameBufferDebugRenderer = {};
	daxa::GimbalLockedCameraController cameraController{};
	std::shared_ptr<daxa::ImageCache> imageCache;
	AssetCache sceneLoader;
	std::optional<daxa::ImGuiRenderer> imguiRenderer = std::nullopt;
	UIState uiState;
	daxa::EntityComponentManager ecm;
	std::vector<DrawPrimCmd> meshDrawCommands;
};

int main() {
	daxa::initialize();

	{
		daxa::Application<MyUser> app{ 1024, 1024, "Daxa Meshview"};
		app.run();
	}

	daxa::cleanup();
}