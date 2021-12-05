#include <SDL2/SDL.h>

#include <Daxa.hpp>

#include <iostream>
#include <iomanip>

using namespace std::chrono;

struct AppState {
	bool continueRunning = true;
	microseconds deltaTimeLastFrame{ 0 };
	float getDeltaTimeSeconds() const { return deltaTimeLastFrame.count() * 0.00'0001f; }
};

class Application {
public:
	Application(u32 winWidth, u32 winHeight, std::string const& winName);
	void run();
	void update();
private:
	std::shared_ptr<AppState> appstate;
	std::shared_ptr<daxa::Window> window;
	std::shared_ptr<daxa::Renderer> renderer;

	std::chrono::system_clock::time_point lastFrameStartTimePoint;
};

Application::Application(u32 winWidth, u32 winHeight, std::string const& winName) {
	this->window = std::make_shared<daxa::Window>(winName, std::array{ winWidth, winHeight });
	this->renderer = std::make_shared<daxa::Renderer>( this->window );
	this->appstate = std::make_shared<AppState>();
	renderer->init();
	
    SDL_AddEventWatch([](void *user_data, SDL_Event *event) -> int {
        auto *app_ptr = static_cast<Application*>(user_data);
		if (app_ptr && app_ptr->window) {
			if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
				SDL_Window* win = SDL_GetWindowFromID(event->window.windowID);
				if (app_ptr->appstate && app_ptr->appstate->continueRunning){
					uint32_t sx = event->window.data1;
					uint32_t sy = event->window.data2;
					app_ptr->window->setSize({sx, sy});
					app_ptr->update();
				}
			}
		}
		return 0;
    }, this);
}

void Application::run() {
	renderer->init();
	lastFrameStartTimePoint = system_clock::now();
	while (this->appstate->continueRunning) update();
	renderer->deinit();
}

void Application::update() {
	auto now = system_clock::now();
	this->appstate->deltaTimeLastFrame = duration_cast<microseconds>(now - lastFrameStartTimePoint);
	lastFrameStartTimePoint = now;
	if (window->update(0.01f)) {
		appstate->continueRunning = false;
		return;
	}
	renderer->draw(appstate->getDeltaTimeSeconds());
}

int main(int argc, char *args[])
{
    daxa::initialize();
    {
        Application app{1024, 720, std::string("Test")};
        app.run();
    }
    daxa::cleanup();
    return 0;
}
