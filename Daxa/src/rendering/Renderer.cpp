#include "Renderer.hpp"

namespace daxa {

	Renderer::Renderer(std::shared_ptr<Window> win)
		: window{ std::move(win) }
		, device{ gpu::Device::createNewDevice() }
		, renderWindow{ this->device, gpu::Device::getInstance(), window->getWindowHandleSDL(), window->getSize()[0], window->getSize()[1], vk::PresentModeKHR::eFifo }
	{ 
		for (int i = 0; i < 3; i++) {
			this->frameResc.push_back(PerFrameRessources{});
		}
	}

	void Renderer::nextFrameContext() {
		auto frameContext = std::move(frameResc.back());
		frameResc.pop_back();
		frameResc.push_front(std::move(frameContext));
		device->nextFrameContext();
	}
}
