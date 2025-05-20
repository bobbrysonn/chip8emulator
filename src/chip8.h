//
// Created by bobmo on 5/14/2025.
//
#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

#define CHIP8_SCREEN_HEIGHT 32
#define CHIP8_SCREEN_WIDTH 64
#define FONT_SET_SIZE 80
#define FONT_START_ADDRESS 0x050

extern uint8_t chip8_font_set[FONT_SET_SIZE];

typedef struct
{
    bool draw_flag;
    uint8_t gfx[64 * 32];
    uint8_t key[16];
    uint8_t memory[4096];
    uint8_t V[16];
    uint8_t timer_delay;
    uint8_t timer_sound;
    uint16_t I;
    uint16_t opcode;
    uint16_t pc;
    uint16_t sp;
    uint16_t stack[16];
} chip8_t;

void chip8_clear_graphics(chip8_t *chip8);
void chip8_cycle(chip8_t *chip8);

/**
 * Jump to next instruction
 *
 * @param chip8 Cpu
 */
void chip8_increment_pc(chip8_t *chip8);
void chip8_init(chip8_t *chip8);
bool chip8_load_rom(chip8_t *chip8, const char *filename);

#endif // CHIP8_H
