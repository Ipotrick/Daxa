#include "Application.hpp"
#include <iostream>
#include <iomanip>

using namespace std::chrono;

namespace daxa {

	Application::Application(u32 winWidth, u32 winHeight, std::string const& winName, std::unique_ptr<User> user) {
		this->window = std::make_shared<Window>(winName, std::array{ winWidth, winHeight });
		this->renderer = std::make_shared<Renderer>( this->window );
		this->user = std::move(user);
		this->appstate = std::make_shared<AppState>();
		renderer->init();
	}

	void Application::run() {
		renderer->init();
		if (user) {
			user->init(this->appstate);
		}

		auto lastFrameStartTimePoint = system_clock::now();
		while (this->appstate->continueRunning) {
			auto now = system_clock::now();
			this->appstate->deltaTimeLastFrame = duration_cast<microseconds>(now - lastFrameStartTimePoint);
			lastFrameStartTimePoint = now;

			std::cout << "delta time: " << std::setw(8) << appstate->getDeltaTimeSeconds() * 1000.0f << "ms, fps: " << 1.0f / (appstate->getDeltaTimeSeconds()) << std::endl;

			if (window->update(0.01f)) break;

			if (user) {
				user->update(this->appstate);
			}

			renderer->draw();
		}

		if (user) {
			user->deinit(this->appstate);
		}
		renderer->deinit();
	}
}
