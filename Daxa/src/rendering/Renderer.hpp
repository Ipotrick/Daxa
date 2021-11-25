#pragma once

#include <unordered_map> 
#include <memory> 

#include "../platform/Window.hpp"

#include "Camera.hpp"
#include "Vulkan.hpp"
#include "ImageManager.hpp"
#include "Image.hpp"

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
	};

	class Renderer {
	public:
		Renderer(std::shared_ptr<Window> win);

		void init() {}

		void draw() {
			auto [image, view] = renderWindow.getNextImage();



			nextFrameContext();
		}

		void deinit() {}

		FPSCamera camera;
		std::shared_ptr<gpu::Device> device;
		PersistentRessources persResc;
		std::deque<PerFrameRessources> frameResc;
	private:
		std::shared_ptr<Window> window{ nullptr };
		gpu::RenderWindow renderWindow;
		void nextFrameContext();
	};
}
