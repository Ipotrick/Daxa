#pragma once

#include "threading/OwningMutex.hpp"
#include "threading/Jobs.hpp"
#include "platform/Window.hpp"
#include "rendering/Rendering.hpp"


namespace daxa {

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

		vkh::Pool<VkSemaphore> semaPool{
				[=]() { return vkh::makeSemaphore(vkh::mainDevice); },
				[=](VkSemaphore sem) { vkDestroySemaphore(vkh::mainDevice, sem, nullptr); },
				[=](VkSemaphore sem) { /* pool destruction deallocs anyway */ }
		};

		vkh::CommandPool cmdPool{};

		std::vector<vkh::UniqueHandle<VkFramebuffer>> _framebuffers;

		vkh::UniqueHandle<VkSemaphore> presentSem{ vkh::makeSemaphore() };
		vkh::UniqueHandle<VkFence> renderFence{ vkh::makeFence() };

		u32 _frameNumber{ 0 };

		vkh::UniqueHandle<VkRenderPass> _renderPass;

		VkPipelineLayout _trianglePipelineLayout;

		VkPipeline _trianglePipeline;
		VkPipeline _redTrianglePipeline;

		VkPipeline meshPipeline;
		SimpleMesh triangleMesh;
	};
}
