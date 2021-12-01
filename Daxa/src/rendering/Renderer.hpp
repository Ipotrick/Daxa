#pragma once

#include <unordered_map> 
#include <memory> 

#include "../platform/Window.hpp"

#include "Camera.hpp"

#include "api_abstration/Device.hpp"
#include "api_abstration/RenderWindow.hpp"

namespace daxa {

	struct PersistentRessources {
		std::unordered_map<std::string_view, gpu::GraphicsPipelineHandle> pipelines;
		std::unordered_map<std::string_view, gpu::ImageHandle> images;
		std::unordered_map<std::string_view, gpu::BufferHandle> buffers;
	};

	struct PerFrameRessources {
		std::unordered_map<std::string_view, gpu::ImageHandle> images;
		std::unordered_map<std::string_view, gpu::BufferHandle> buffers;
		std::unordered_map<std::string_view, VkSemaphore> semaphores;
		gpu::FenceHandle finishHandle;
	};

	class Renderer {
	public:
		Renderer(std::shared_ptr<Window> win);

		~Renderer();

		void init();

		void draw(float deltaTime);

		void deinit();

		FPSCamera camera;
		std::shared_ptr<gpu::Device> device;
		std::optional<PersistentRessources> persResc;
		std::deque<PerFrameRessources> frameResc;
		double totalElapsedTime{ 0.0 };
	private:
		gpu::GraphicsPipelineHandle testPipeline;
		std::shared_ptr<Window> window{ nullptr };
		gpu::RenderWindow renderWindow;
		void nextFrameContext();
	};
}
