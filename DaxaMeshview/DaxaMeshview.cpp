#include <iostream>

#include <stb_image.h>

#include "Daxa.hpp"

#include "World.hpp"
#include "RenderContext.hpp"
#include "MeshRenderer.hpp"
#include "cgltf.h"

struct UIState {
	char loadFileTextBuf[256] = {};
};

struct ModelComp {
	daxa::gpu::BufferHandle indiexBuffer = {};
	daxa::gpu::BufferHandle vertexPositions = {};
	daxa::gpu::BufferHandle vertexUVs = {};
	daxa::gpu::ImageHandle image = {};
	u32 indexCount = 0;
};

struct ChildComp {
	daxa::EntityHandle parent = {};
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

		auto cmdList = renderCTX->device->getEmptyCommandList();
		cmdList->begin();
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

	daxa::EntityHandle createEntityFromModel(char const* const modelPath) {
		std::string fixedModelPath = modelPath;
		if (!(fixedModelPath.back() == '/' || fixedModelPath.back() == '\\')) {
			fixedModelPath.push_back('/');
		}
		std::string scenePath = fixedModelPath + "scene.gltf";
		cgltf_options options = {};
		cgltf_data* data = NULL;
		cgltf_result result = cgltf_parse_file(&options, scenePath.c_str(), &data);
		if (result == cgltf_result_success)
		{
			printf("loading success\n");
			for (int i = 0; i < data->textures_count; i++) {
				printf("texture nr %i\n", i);
				printf("texture uri: %s\n", data->textures[i].image->uri);
			}

			std::string texPath;
			std::vector<daxa::gpu::ImageHandle> textures;

			for (int i = 0; i < data->textures_count; i++) {
				texPath = fixedModelPath + data->textures[i].image->uri;
				printf("texture path %s\n",texPath.c_str());
				textures.push_back(imageCache.get({
					.path = texPath.c_str(), 
					.samplerInfo = daxa::gpu::SamplerCreateInfo{
						.minFilter = VK_FILTER_LINEAR,
						.magFilter = VK_FILTER_LINEAR,
					}
				}));
			}

			std::string bufferPath = fixedModelPath + data->buffers[0].uri;
			printf("buffer data path: %s\n", bufferPath.c_str());

			cgltf_options buffer_load_options = {};
			cgltf_load_buffers(&buffer_load_options, data, scenePath.c_str());

			daxa::gpu::BufferHandle indices;
			daxa::gpu::BufferHandle positions;
			daxa::gpu::BufferHandle uvs;

			auto cmd = renderCTX->device->getEmptyCommandList();
			cmd->begin();

			auto* prim = data->meshes[0].primitives;
			auto count = data->meshes[0].primitives->indices->count;
			auto size = count * sizeof(u32);
			u8* indicesPtr = (u8*)data->meshes[0].primitives->indices->buffer_view->buffer->data;
			indicesPtr += data->meshes[0].primitives->indices->buffer_view->offset;
			indicesPtr += data->meshes[0].primitives->indices->offset;

			indices = renderCTX->device->createBuffer({
				.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				.size = size,
			});

			cmd->copyHostToBuffer({
				.dst = indices,
				.size = size,
				.src = indicesPtr,
			});

			for (auto i = 0; i < prim->attributes_count; i++) {
				if (strcmp("POSITION", prim->attributes[i].name) == 0) {
					printf("found position vertex attribute\n");

					auto count = prim->attributes[i].data->count;
					auto size = prim->attributes[i].data->count * sizeof(glm::vec3);
					auto stride = prim->attributes[i].data->stride;

					positions = renderCTX->device->createBuffer({
						.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
						.size = size,
					});

					auto mm = cmd->mapMemoryStaged(positions, size, 0);
					u8* dataPtr = (u8*)prim->attributes[i].data->buffer_view->buffer->data;
					dataPtr += prim->attributes[i].data->buffer_view->offset;
					dataPtr += prim->attributes[i].data->offset;
					
					std::memcpy(mm.hostPtr, dataPtr, size);
					
					cmd->unmapMemoryStaged(mm);
				}
				if (strcmp("TEXCOORD_0", prim->attributes[i].name) == 0) {
					printf("found texture coord vertex attribute\n");

					auto count = prim->attributes[i].data->count;
					auto size = prim->attributes[i].data->count * sizeof(glm::vec2);
					auto stride = prim->attributes[i].data->stride;

					uvs = renderCTX->device->createBuffer({
						.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
						.size = size,
					});

					auto mm = cmd->mapMemoryStaged(uvs, size, 0);
					u8* dataPtr = (u8*)prim->attributes[i].data->buffer_view->buffer->data;
					dataPtr += prim->attributes[i].data->buffer_view->offset;
					dataPtr += prim->attributes[i].data->offset;
					
					std::memcpy(mm.hostPtr, dataPtr, size);
					
					cmd->unmapMemoryStaged(mm);
				}
			}
			
			cmd->end();
			renderCTX->queue->submitBlocking({
				.commandLists = {cmd}
			});

			if (textures.size() > 0) {
				auto cmd = imageCache.getUploadCommands();
				renderCTX->queue->submitBlocking({.commandLists = {cmd}});
			}

			auto ent = ecm.createEntity();

			auto view = ecm.view<daxa::TransformComp, ModelComp>();

			printf("count: %i\n", count);

			auto trans = glm::mat4{1.0f};
			trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3{1,0,0});

			view.addComp(ent, daxa::TransformComp{.translation = trans});
			view.addComp(ent, ModelComp{
				.image = textures[0],
				.indexCount = (u32)count,
				.indiexBuffer = indices,
				.vertexPositions = positions,
				.vertexUVs = uvs,
			});

			return ent;

			cgltf_free(data);
		}
		else {
			printf("loading failed\n");
		}
		return {};
	}

	void update(daxa::AppState& app) {

		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		ImGui::Begin("file import");
		ImGui::InputText("file path", uiState.loadFileTextBuf, sizeof(uiState.loadFileTextBuf));
		if (ImGui::Button("load")) {
			printf("try to load model with path: %s\n", uiState.loadFileTextBuf);

			auto entity = createEntityFromModel(uiState.loadFileTextBuf);
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
			auto childComps = ecm.view<ChildComp>();

			for (auto [ent, trans, model] : view) {
				transformBuffer.clear();
				transformBuffer.push_back(trans.translation);
				auto iterEnt = ent;
				while (ChildComp* childComp = childComps.getCompIf<ChildComp>(iterEnt)) {
					transformBuffer.push_back(view.getComp<daxa::TransformComp>(childComp->parent).translation);
					iterEnt = childComp->parent;
				}

				auto translation = glm::mat4{1.0f};
				for (int i = 0; i < transformBuffer.size(); i++) {
					translation =  transformBuffer[i] * translation;
				}

				//if (!childComps.hasComp<ChildComp>(ent)) {
				//	trans.translation = glm::rotate(trans.translation, 1.f * (f32)app.getDeltaTimeSeconds(), glm::vec3{1,1,1});
				//}
				draws.push_back(MeshRenderer::DrawMesh{
					.albedo = model.image,
					.indexCount = model.indexCount,
					.indices = model.indiexBuffer,
					.positions = model.vertexPositions,
					.uvs = model.vertexUVs,
					.transform = translation,
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
	std::vector<glm::mat4> transformBuffer;
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