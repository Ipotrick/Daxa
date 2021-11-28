#include "Application.hpp"

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

		while (this->appstate->continueRunning) {
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
