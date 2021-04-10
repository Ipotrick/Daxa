#pragma once

#include "DaxaCore.hpp"

#include "threading/OwningMutex.hpp"
#include "threading/Jobs.hpp"
#include "platform/Window.hpp"
#include "rendering/Rendering.hpp"

namespace daxa {

	struct MeshPushConstants {
		daxa::Mat4x4 renderMatrix;
		daxa::Vec4 data;
	};

	class Application {
	public:
		Application(std::string name, u32 width, u32 height);

		~Application();

		void draw();

		std::unique_ptr<OwningMutex<Window>> windowMutex; 
	private:

		void init_default_renderpass();

		void init_framebuffers();

		void init_pipelines();

		void init_mesh_pipeline();

		void cleanup();

		void loadMeshes();

		void uploadMesh(SimpleMesh& mesh);

		vkh::Pool<vk::Semaphore> semaPool{
				[]() { return vkh_old::device.createSemaphore({}); },
				[](vk::Semaphore sem) { vkh_old::device.destroySemaphore(sem,nullptr); },
				[](vk::Semaphore sem) { /* dont need to reset a semaphore */ }
		};

		vkh::CommandPool cmdPool{vkh_old::device, vkh_old::mainGraphicsQueueFamiltyIndex};

		std::vector<vk::UniqueFramebuffer> framebuffers;

		vk::UniqueFence renderFence{ vkh_old::device.createFenceUnique(vk::FenceCreateInfo{}) };

		vk::UniqueSemaphore presentSem;

		u32 _frameNumber{ 0 };

		vk::UniqueRenderPass mainRenderpass;

		SimpleMesh monkeyMesh;
		vkh::Pipeline meshPipeline;
	};
}
