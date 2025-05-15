//
// Created by bobmo on 5/15/2025.
//

#include <stdlib.h>
#include <string.h>
#include "chip8.h"

#include <stdio.h>
#include <time.h>

static unsigned long mix(unsigned long a, unsigned long b, unsigned long c);

uint8_t chip8_font_set[FONT_SET_SIZE] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void chip8_clear_graphics(chip8_t *chip8) {
    memset(chip8->gfx, 0, sizeof(chip8->gfx));
}

void chip8_cycle(chip8_t* chip8) {
    /* Obtain the opcode stored over 2 bytes */
    chip8->opcode = chip8->memory[chip8->pc] << 8 | chip8->memory[chip8->pc + 1];

    const uint16_t first = chip8->opcode >> 12;

    switch (first) {
        case 0x0: {
            switch (chip8->opcode) {
                /* CLS: Clear the display */
                case 0x00E0:
                    chip8_clear_graphics(chip8);
                    break;

                /* RET: Return from a subroutine */
                case 0x00EE:
                    chip8->sp--;
                    chip8->pc = chip8->stack[chip8->sp];
                    break;

                default:
                    printf("Unknown opcode 0x%04X\n", chip8->opcode);
                    chip8_increment_pc(chip8);
                    break;
            }

            chip8_increment_pc(chip8);
            break;
        }

        /* JP addr: jump to location NNN */
        case 0x1:
            chip8->pc = chip8->opcode & 0x0FFF;
            break;

        case 0x2:
            chip8->stack[chip8->sp] = chip8->pc;
            chip8->sp++;
            chip8->pc = chip8->opcode & 0x0FFF;
            break;

        case 0x3: {
            const uint8_t x = (chip8->opcode & 0x0F00) >> 8;

            if (chip8->V[x] == (chip8->opcode & 0x00FF)) {
                chip8_increment_pc(chip8);
            }

            chip8_increment_pc(chip8);
            break;
        }

        case 0x4: {
            const uint8_t x = (chip8->opcode & 0x0F00) >> 8;

            if (chip8->V[x] != (chip8->opcode & 0x00FF)) {
                chip8_increment_pc(chip8);
            }

            chip8_increment_pc(chip8);
            break;
        }

        case 0x5: {
            const uint8_t x = (chip8->opcode & 0x0F00) >> 8;
            const uint8_t y = (chip8->opcode & 0x00F0) >> 4;

            if (chip8->V[x] == chip8->V[y]) {
                chip8_increment_pc(chip8);
            }

            chip8_increment_pc(chip8);
            break;
        }

        case 0x6: {
            const uint8_t x = (chip8->opcode & 0x0F00) >> 8;
            chip8->V[x] = chip8->opcode & 0x00FF;
            chip8_increment_pc(chip8);
            break;
        }

        case 0x7: {
            const uint8_t x = (chip8->opcode & 0x0F00) >> 8;
            chip8->V[x] += (chip8->opcode & 0x00FF);
            chip8_increment_pc(chip8);
            break;
        }

        case 0x8: {
            break;
        }

        case 0x9: {
            const uint8_t x = (chip8->opcode & 0x0F00) >> 8;
            const uint8_t y = (chip8->opcode & 0x00F0) >> 4;

            if (chip8->V[x] != chip8->V[y]) {
                chip8_increment_pc(chip8);
            }

            chip8_increment_pc(chip8);
            break;
        }

        default: {
            printf("Unknown opcode 0x%04X\n", chip8->opcode);
            chip8_increment_pc(chip8);
            break;
        }
    }
}

void chip8_increment_pc(chip8_t *chip8) {
    chip8->pc += 2;
}

void chip8_init(chip8_t *chip8) {
    srand((unsigned int) mix(time(nullptr), time(nullptr), time(nullptr)));

    memset(chip8, 0, sizeof(*chip8));

    chip8->pc = 0x200;

    /* Set font set */
    for (int i = 0; i < FONT_SET_SIZE; i++) {
        chip8->memory[i] = chip8_font_set[i];
    }
}

void chip8_load_rom(chip8_t* chip8, const char* filename) {

}

static unsigned long mix(unsigned long a, unsigned long b, unsigned long c)
{
    a=a-b;  a=a-c;  a=a^(c >> 13);
    b=b-c;  b=b-a;  b=b^(a << 8);
    c=c-a;  c=c-b;  c=c^(b >> 13);
    a=a-b;  a=a-c;  a=a^(c >> 12);
    b=b-c;  b=b-a;  b=b^(a << 16);
    c=c-a;  c=c-b;  c=c^(b >> 5);
    a=a-b;  a=a-c;  a=a^(c >> 3);
    b=b-c;  b=b-a;  b=b^(a << 10);
    c=c-a;  c=c-b;  c=c^(b >> 15);
    return c;
}