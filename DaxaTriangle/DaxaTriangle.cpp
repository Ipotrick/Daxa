#include "Daxa.hpp"

#include <iostream>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main(int argc, char* args[])
{
    daxa::Mat3x3 mat{ std::array{1.0f,2.0f,1.0f}, std::array{2.0f,2.0f,2.0f}, std::array{ 2.0f,3.0f,6.0f } };

    auto inv = daxa::inverse(mat);
    std::cout << "inverse: " << inv << std::endl;

    daxa::Mat3x3 mat3{ std::array{2.0f,0.0f,0.0f}, std::array{0.0f,2.0f,0.0f}, {1.0f, 5.0f, -4.0f} };
    auto mat4 = daxa::reform<4, 3>(mat3);
    daxa::Vec3 vec3{ 1.0f,2.0f,3.0f };

    std::cout << mat4 << " * " << vec3 << " = ";
    auto res = mat4 * vec3;
    std::cout << res << std::endl;

    //The window we'll be rendering to
    SDL_Window* window = NULL;

    //The surface contained by the window
    SDL_Surface* screenSurface = NULL;

    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    }
    else {
        //Create window
        window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (window == NULL) {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        }
        else {
            //Get window surface
            screenSurface = SDL_GetWindowSurface(window);

            //Fill the surface white
            SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0x00, 0xFF));

            //Update the surface
            SDL_UpdateWindowSurface(window);

            //Wait two seconds
            SDL_Delay(2000);
        }
    }

    //Destroy window
    SDL_DestroyWindow(window);

    //Quit SDL subsystems
    SDL_Quit();

    return 0;
}