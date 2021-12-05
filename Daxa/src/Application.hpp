#include "DaxaCore.hpp"

#include <chrono>

#include "rendering/Renderer.hpp"
#include "platform/Window.hpp"

namespace daxa {

	struct AppState {
		bool continueRunning = true;
		std::chrono::microseconds deltaTimeLastFrame = {};
		std::shared_ptr<Window> window;

		float getDeltaTimeSeconds() const { return deltaTimeLastFrame.count() * 0.00'0001f; }
	};

	class User {
	public:
		virtual ~User() = default;
		virtual void init(std::shared_ptr<AppState> appstate)	= 0;
		virtual void update(std::shared_ptr<AppState> appstate)	= 0;
		virtual void deinit(std::shared_ptr<AppState> appstate)	= 0;
	};

	class Application {
	public:
		Application(u32 winWidth, u32 winHeight, std::string const& winName, std::unique_ptr<User> user);

		void run();
	private:
		void update();

		// the reason behind the exessive use of shared ptr here is for later multithreadding use.
		// the lifetimes in multithreadding can get very complex so shared_ptrs are appropriate for and ONLY FOR these "low frequency" types wich are accessed and copied rarely.
		std::shared_ptr<AppState> appstate;
		std::shared_ptr<Window> window;
		std::unique_ptr<User> user;
	};

}
