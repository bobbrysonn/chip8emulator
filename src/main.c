#include <stdio.h>
#include <SDL3/SDL.h>
#include "chip8.h"

bool init();
void deInit();

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;
chip8_t *cpu;

int main() {

    if (!init()) {
        printf("Failed to initialize!\n");
        return 1;
    }

    bool quit = false;
    while (!quit) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    quit = true;
                    break;

                default:
                    break;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    deInit();

    return 0;
}

bool init() {
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
        printf("SDL_InitSubSystem() failed: %s\n", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow("Chip8 Emulator", 1024, 512, 0);
    if (window == NULL) {
        printf("SDL_CreateWindow() failed: %s\n", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer == NULL) {
        printf("SDL_CreateRenderer() failed: %s\n", SDL_GetError());
        return false;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    if (texture == NULL) {
        printf("SDL_CreateTexture() failed: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

void deInit() {
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(texture);
    SDL_Quit();
}