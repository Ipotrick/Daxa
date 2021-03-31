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

		void cleanup();

		u32 _frameNumber{ 0 };

		vk::CommandPool cmdPool{};

		vk::CommandBuffer cmd{ cmdPool };

		VkRenderPass _renderPass;

		std::vector<VkFramebuffer> _framebuffers;

		vk::Semaphore _presentSemaphore, _renderSemaphore;
		vk::Fence _renderFence;

		VkPipelineLayout _trianglePipelineLayout;

		VkPipeline _trianglePipeline;
		VkPipeline _redTrianglePipeline;
	};
}
