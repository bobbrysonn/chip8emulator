#include "chip8.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>

bool init(const char *filename);
void deInit();

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;
chip8_t *cpu;
uint32_t last_time_tick = 0;

int main(const int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("No ROM given\n");
        printf("Usage: %s <rom file>\n", argv[0]);
        return 1;
    }

    if (!init(argv[1]))
    {
        printf("Failed to initialize!\n");
        return 1;
    }

    bool quit = false;
    while (!quit)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                quit = true;
                break;

            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP: {
                bool isDown = (event.type == SDL_EVENT_KEY_DOWN);

                switch (event.key.scancode)
                {
                // row 1
                case SDLK_1:
                    cpu->key[0x1] = isDown;
                    break;
                case SDLK_2:
                    cpu->key[0x2] = isDown;
                    break;
                case SDLK_3:
                    cpu->key[0x3] = isDown;
                    break;
                case SDLK_4:
                    cpu->key[0xC] = isDown;
                    break;

                    // row 2
                case SDLK_Q:
                    cpu->key[0x4] = isDown;
                    break;
                case SDLK_W:
                    cpu->key[0x5] = isDown;
                    break;
                case SDLK_E:
                    cpu->key[0x6] = isDown;
                    break;
                case SDLK_R:
                    cpu->key[0xD] = isDown;
                    break;

                    // row 3
                case SDLK_A:
                    cpu->key[0x7] = isDown;
                    break;
                case SDLK_S:
                    cpu->key[0x8] = isDown;
                    break;
                case SDLK_D:
                    cpu->key[0x9] = isDown;
                    break;
                case SDLK_F:
                    cpu->key[0xE] = isDown;
                    break;

                    // row 4
                case SDLK_Z:
                    cpu->key[0xA] = isDown;
                    break;
                case SDLK_X:
                    cpu->key[0x0] = isDown;
                    break;
                case SDLK_C:
                    cpu->key[0xB] = isDown;
                    break;
                case SDLK_V:
                    cpu->key[0xF] = isDown;
                    break;

                    // optional: exit on ESC
                case SDLK_ESCAPE:
                    quit = true;
                    break;

                default:
                    break;
                }
                break;
            }

            default:
                break;
            }
        }

        chip8_cycle(cpu);

        const uint32_t current_time_tick = SDL_GetTicks();
        if (current_time_tick - last_time_tick >= 16)
        {
            if (cpu->timer_delay > 0)
            {
                cpu->timer_delay--;
            }

            if (cpu->timer_sound > 0)
            {
                cpu->timer_sound--;
            }

            last_time_tick = current_time_tick;
        }

        uint32_t pixels[64 * 32];
        for (int i = 0; i < 64 * 32; i++)
        {
            pixels[i] = cpu->gfx[i] ? 0xFFFFFFFF : 0xFF000000;
        }

        SDL_UpdateTexture(texture, nullptr, pixels, 64 * sizeof(uint32_t));

        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        SDL_Delay(1);
    }

    deInit();

    return 0;
}

bool init(const char *filename)
{
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) == 0)
    {
        printf("SDL_InitSubSystem() failed: %s\n", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow("Chip8 Emulator", 1024, 512, 0);
    if (window == NULL)
    {
        printf("SDL_CreateWindow() failed: %s\n", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer == NULL)
    {
        printf("SDL_CreateRenderer() failed: %s\n", SDL_GetError());
        return false;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    if (texture == NULL)
    {
        printf("SDL_CreateTexture() failed: %s\n", SDL_GetError());
        return false;
    }

    cpu = malloc(sizeof(chip8_t));
    if (!cpu)
    {
        printf("Failed to allocate memory!\n");
        return false;
    }

    chip8_init(cpu);
    chip8_load_rom(cpu, filename);

    return true;
}

void deInit()
{
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(texture);
    SDL_Quit();

    free(cpu);
}