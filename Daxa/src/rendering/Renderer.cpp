#include "Renderer.hpp"

namespace daxa {

	Renderer::Renderer(std::shared_ptr<Window> win)
		: window{ std::move(win) }
		, device{ std::make_shared<gpu::Device>(std::move(gpu::Device::createNewDevice())) }
		, renderWindow{ this->device, gpu::Device::getInstance(), window->getWindowHandleSDL(), window->getSize()[0], window->getSize()[1], vk::PresentModeKHR::eFifo }
	{ }

	void Renderer::nextFrameContext() {
		auto frameContext = std::move(frameResc.back());
		frameResc.pop_back();
		frameResc.push_front(std::move(frameContext));
		device->nextFrameContext();
	}
}
