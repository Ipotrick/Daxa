#pragma once

#include <unordered_map> 
#include <memory> 

#include "../platform/Window.hpp"

#include "gpu.hpp"

#include "Camera.hpp"

namespace daxa {

	struct PerFrameRessources {
		gpu::SignalHandle renderingFinishedSignal;
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
		gpu::Queue queue;
		std::unordered_map<std::string_view, gpu::GraphicsPipelineHandle> pipelines;
		std::unordered_map<std::string_view, gpu::ImageHandle> images;
		std::unordered_map<std::string_view, gpu::BufferHandle> buffers;
		std::deque<PerFrameRessources> frameResc;
		PerFrameRessources* currentFrame = {};
		double totalElapsedTime{ 0.0 };
	private:
		void nextFrameContext();

		gpu::SwapchainImage swapchainImage;

		inline static constexpr size_t FRAMES_IN_FLIGHT{ 3 };

		std::shared_ptr<Window> window{ nullptr };
		gpu::RenderWindow renderWindow;
	};
}
