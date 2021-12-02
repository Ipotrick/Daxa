#pragma once

#include <unordered_map> 
#include <memory> 

#include "../platform/Window.hpp"

#include "gpu.hpp"

#include "Camera.hpp"
#include "StagingBufferPool.hpp"

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
		gpu::TimelineSemaphore timeline;
		u64 finishCounter = 0;
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
		void nextFrameContext();

		gpu::GraphicsPipelineHandle testPipeline;
		std::shared_ptr<Window> window{ nullptr };
		gpu::RenderWindow renderWindow;
		StagingBufferPool stagingBufferPool;
	};
}
