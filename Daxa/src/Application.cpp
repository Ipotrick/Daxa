#include "Application.hpp"
#include <iostream>
#include <iomanip>

using namespace std::chrono;

namespace daxa {

	Application::Application(u32 winWidth, u32 winHeight, std::string const& winName, std::unique_ptr<User> user) {
		this->window = std::make_unique<Window>(winName, std::array{ winWidth, winHeight });
		this->user = std::move(user);
		this->appstate = std::make_shared<AppState>();
	}

	void Application::run() {
		appstate->window = window;
		if (user) {
			user->init(this->appstate);
		}

		auto lastFrameStartTimePoint = system_clock::now();
		while (this->appstate->continueRunning) {
			auto now = system_clock::now();
			this->appstate->deltaTimeLastFrame = duration_cast<microseconds>(now - lastFrameStartTimePoint);
			lastFrameStartTimePoint = now;

			if (window->update(appstate->getDeltaTimeSeconds())) break;

			if (user) {
				user->update(this->appstate);
			}
		}

		if (user) {
			user->deinit(this->appstate);
		}
	}
}
