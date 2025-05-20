#include "chip8.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>

bool init(const char *filename);
void deInit();

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;
chip8_t *cpu = NULL;
uint32_t last_time_tick = 0;

int main(int argc, char *argv[])
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
                switch (event.key.key)
                {
                case SDLK_1: cpu->key[0x1] = isDown; break;
                case SDLK_2: cpu->key[0x2] = isDown; break;
                case SDLK_3: cpu->key[0x3] = isDown; break;
                case SDLK_4: cpu->key[0xC] = isDown; break;
                case SDLK_Q: cpu->key[0x4] = isDown; break;
                case SDLK_W: cpu->key[0x5] = isDown; break;
                case SDLK_E: cpu->key[0x6] = isDown; break;
                case SDLK_R: cpu->key[0xD] = isDown; break;
                case SDLK_A: cpu->key[0x7] = isDown; break;
                case SDLK_S: cpu->key[0x8] = isDown; break;
                case SDLK_D: cpu->key[0x9] = isDown; break;
                case SDLK_F: cpu->key[0xE] = isDown; break;
                case SDLK_Z: cpu->key[0xA] = isDown; break;
                case SDLK_X: cpu->key[0x0] = isDown; break;
                case SDLK_C: cpu->key[0xB] = isDown; break;
                case SDLK_V: cpu->key[0xF] = isDown; break;
                case SDLK_ESCAPE: quit = true; break;
                default: break;
                }
                break;
            }

            default:
                break;
            }
        }

        // Run several chip8 cycles per frame (~500Hz)
        for (int i = 0; i < 10; i++) {
            chip8_cycle(cpu);
        }

        // Timer tick (60Hz)
        const uint32_t current_time_tick = SDL_GetTicks();
        if (current_time_tick - last_time_tick >= 16)
        {
            if (cpu->timer_delay > 0) cpu->timer_delay--;
            if (cpu->timer_sound > 0) cpu->timer_sound--;
            last_time_tick = current_time_tick;
        }

        if (cpu->draw_flag)
        {
            uint32_t pixels[64 * 32];
            for (int i = 0; i < 64 * 32; i++)
            {
                pixels[i] = cpu->gfx[i] ? 0xFFFFFFFF : 0xFF000000;
            }

            SDL_UpdateTexture(texture, NULL, pixels, 64 * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderTexture(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);

            cpu->draw_flag = false;
        }

        SDL_Delay(1);
    }

    deInit();
    return 0;
}

bool init(const char *filename)
{
    if (!(SDL_InitSubSystem(SDL_INIT_VIDEO)))
    {
        printf("SDL_InitSubSystem() failed: %s\n", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow("CHIP-8 Emulator", 1024, 512, 0);
    if (!window)
    {
        printf("SDL_CreateWindow() failed: %s\n", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer)
    {
        printf("SDL_CreateRenderer() failed: %s\n", SDL_GetError());
        return false;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    if (!texture)
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

    if (!chip8_load_rom(cpu, filename))
    {
        return false;
    }

    chip8_clear_graphics(cpu);

    return true;
}

void deInit()
{
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    free(cpu);
}
