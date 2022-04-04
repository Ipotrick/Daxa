#define GLM_DEPTH_ZERO_TO_ONE

#include <game/game.hpp>

int main() {
    daxa::initialize();
    {
        Game game;
        while (true) {
            game.update();
            if (game.window.should_close())
                break;
        }
    }
    daxa::cleanup();
}
