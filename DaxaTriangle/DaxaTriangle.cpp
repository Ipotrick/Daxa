#include "Daxa.hpp"

#include <iostream>

class MyUser : public daxa::User {
    std::unique_ptr<daxa::Renderer> defaultRenderer;
public:
    virtual void init(daxa::AppState* appstate) override {
        defaultRenderer = std::make_unique<daxa::Renderer>(appstate->window);
        defaultRenderer->init();
    }

    virtual void update(daxa::AppState* appstate) override {
        std::cout << "delta time: " << std::setw(8) << appstate->getDeltaTimeSeconds() * 1000.0f << "ms, fps: " << 1.0f / (appstate->getDeltaTimeSeconds()) << std::endl;
        defaultRenderer->draw(appstate->getDeltaTimeSeconds());
    }

    virtual void deinit(daxa::AppState* appstate) override {
        defaultRenderer->deinit();
    }
};

int main(int argc, char* args[])
{
    daxa::initialize();
    {
        daxa::Application app{ 1000, 1000, std::string("Test"), std::make_unique<MyUser>(MyUser{}) };

        app.run();
    }
    daxa::cleanup();

    return 0;
}