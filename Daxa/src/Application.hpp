#pragma once

#include "DaxaCore.hpp"

#include <chrono>

#include "platform/Window.hpp"

namespace daxa {

	struct AppState {
		bool continueRunning = true;
		std::chrono::microseconds deltaTimeLastFrame = {};
		Window* window = nullptr;

		float getDeltaTimeSeconds() const { return std::chrono::duration<float>(deltaTimeLastFrame).count(); }
	};

	template<typename TUser>
	class Application {
	public:
		Application(u32 winWidth, u32 winHeight, std::string const& winName) 
			: window{ winName, winWidth, winHeight }
			, appstate{ .continueRunning = true, .window = &this->window }
		{ }

		void run() {
			using namespace std::chrono;
			user = std::make_unique<TUser>(appstate);

			auto lastFrameStartTimePoint = system_clock::now();
			while (this->appstate.continueRunning) {
				auto now = system_clock::now();
				this->appstate.deltaTimeLastFrame = duration_cast<microseconds>(now - lastFrameStartTimePoint);
				lastFrameStartTimePoint = now;

				if (window.update(appstate.getDeltaTimeSeconds())) break;

				user->update(appstate);
			}

			user->cleanup(appstate);
			user.reset();
		}
	private:

		Window window;
		AppState appstate;
		std::unique_ptr<TUser> user;
	};

}
