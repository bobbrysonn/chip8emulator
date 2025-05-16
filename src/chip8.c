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
    // printf("Current opcode: 0x%04X\n", (chip8->memory[chip8->pc] << 8 | chip8->memory[chip8->pc + 1]));

    chip8->opcode = chip8->memory[chip8->pc] << 8 | chip8->memory[chip8->pc + 1];

    const uint16_t first = chip8->opcode >> 12;

    switch (first) {
        case 0x0: {
            switch (chip8->opcode) {
                case 0x00E0:
                    chip8_clear_graphics(chip8);
                    break;

                case 0x00EE:
                    if (chip8->sp == 0) {
                        printf("Stack underflow\n");
                        return;
                    }

                    chip8->sp--;
                    chip8->pc = chip8->stack[chip8->sp];
                    return;

                default:
                    printf("Unknown opcode 0x%04X\n", chip8->opcode);
                    break;
            }

            chip8_increment_pc(chip8);
            break;
        }

        /* JP addr: jump to location NNN */
        case 0x1: {
            chip8->pc = chip8->opcode & 0x0FFF;
            break;
        }

        case 0x2: {
            chip8->stack[chip8->sp] = chip8->pc;

            if (chip8->sp >= 15) {
                printf("Stack overflow\n");
                return;
            }

            chip8->sp++;
            chip8->pc = chip8->opcode & 0x0FFF;
            break;
        }

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
            const uint8_t x = (chip8->opcode & 0x0F00) >> 8;
            const uint8_t y = (chip8->opcode & 0x00F0) >> 4;
            const uint8_t k = (chip8->opcode & 0x000F);

            switch (k) {
                case 0x0: {
                    chip8->V[x] = chip8->V[y];
                    break;
                }

                case 0x1: {
                    chip8->V[x] |= chip8->V[y];
                    break;
                }

                case 0x2: {
                    chip8->V[x] &= chip8->V[y];
                    break;
                }

                case 0x3: {
                    chip8->V[x] ^= chip8->V[y];
                    break;
                }

                case 0x4: {
                    const uint16_t sum = chip8->V[x] + chip8->V[y];
                    chip8->V[0xF] = (sum > 0xFF) ? 1 : 0;
                    chip8->V[x] = sum & 0xFF;
                    break;
                }

                case 0x5: {
                    chip8->V[0xF] = (chip8->V[x] > chip8->V[y]) ? 1 : 0;
                    chip8->V[x] -= chip8->V[y];
                    break;
                }

                case 0x6: {
                    chip8->V[0xF] = chip8->V[x] & 0x1;
                    chip8->V[x] >>= 1;
                    break;
                }

                case 0x7: {
                    chip8->V[0xF] = (chip8->V[y] > chip8->V[x]) ? 1 : 0;
                    chip8->V[x] = chip8->V[y] - chip8->V[x];
                    break;
                }

                case 0xE: {
                    chip8->V[0xF] = (chip8->V[x] & 0x80) >> 7;
                    chip8->V[x] <<= 1;
                    break;
                }

                default: {
                    printf("Unknown opcode 0x%04X\n", chip8->opcode);
                    break;
                }
            }

            chip8_increment_pc(chip8);
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

        case 0xA: {
            chip8->I = chip8->opcode & 0x0FFF;
            chip8_increment_pc(chip8);
            break;
        }

        case 0xB: {
            chip8->pc = (chip8->opcode & 0x0FFF) + (uint16_t)chip8->V[0];
            break;
        }

        case 0xC: {
            const uint8_t x = (chip8->opcode & 0x0F00) >> 8;
            const uint8_t y = (uint8_t)rand() % 256; // NOLINT(cert-msc30-c, cert-msc50-cpp)

            chip8->V[x] = y & (chip8->opcode & 0x00FF);

            chip8_increment_pc(chip8);
            break;
        }

        case 0xD: {
            const uint8_t x = chip8->V[(chip8->opcode & 0x0F00) >> 8];
            const uint8_t y = chip8->V[(chip8->opcode & 0x00F0) >> 4];
            const uint8_t height = chip8->opcode & 0x000F;

            chip8->V[0xF] = 0;

            for (int row = 0; row < height; row++) {
                const uint8_t pixel = chip8->memory[chip8->I + row];

                for (int col = 0; col < 8; col++) {
                    if ((pixel & (0x80 >> col)) != 0) {
                        const uint16_t x_pos = (x + col) % 64;
                        const uint16_t y_pos = (y + row) % 32;
                        const uint16_t idx = x_pos + (y_pos * 64);

                        if (chip8->gfx[idx] == 1) {
                            chip8->V[0xF] = 1;
                        }

                        chip8->gfx[idx] ^= 1;
                    }
                }
            }

            chip8_increment_pc(chip8);
            break;
        }

        case 0xE: {
            const uint8_t x = (chip8->opcode & 0x0F00) >> 8;
            const uint16_t kk = chip8->opcode & 0x00FF;

            switch (kk) {
                case 0x9E: {
                    if (chip8->key[chip8->V[x] & 0xF] != 1) {
                        chip8_increment_pc(chip8);
                    }
                    break;
                }

                case 0xA1: {
                    if (chip8->key[chip8->V[x]] != 1) {
                        chip8_increment_pc(chip8);
                    }
                    break;
                }

                default: {
                    printf("Unknown opcode 0x%04X\n", chip8->opcode);
                    chip8_increment_pc(chip8);
                    break;
                }
            }

            chip8_increment_pc(chip8);
            break;
        }

        case 0xF: {
            const uint8_t x = (chip8->opcode & 0x0F00) >> 8;
            const uint16_t kk = chip8->opcode & 0x00FF;

            switch (kk) {
                case 0x07: {
                    chip8->V[x] = chip8->timer_delay;
                    break;
                }

                case 0x0A: {
                    for (int i = 0; i < 16; i++) {
                        if (chip8->key[i] != 0) {
                            chip8->V[x] = i;

                            chip8_increment_pc(chip8);

                            return;
                        }
                    }

                    return;
                }

                case 0x15: {
                    chip8->timer_delay = chip8->V[x];
                    break;
                }

                case 0x18: {
                    chip8->timer_sound = chip8->V[x];
                    break;
                }

                case 0x1E: {
                    chip8->I += chip8->V[x];
                    break;
                }

                case 0x29: {
                    if (chip8->V[x] < 16) {
                        chip8->I = FONT_START_ADDRESS + chip8->V[x] * 5;
                    }
                    break;
                }

                case 0x33: {
                    const uint8_t value = chip8->V[x];
                    chip8->memory[chip8->I]     = value / 100;
                    chip8->memory[chip8->I + 1] = (value / 10) % 10;
                    chip8->memory[chip8->I + 2] = value % 10;
                    break;
                }

                case 0x55: {
                    for (int i = 0; i <= x; i++) {
                        chip8->memory[chip8->I + i] = chip8->V[i];
                    }
                    break;
                }

                case 0x65: {
                    for (int i = 0; i <= x; i++) {
                        chip8->V[i] = chip8->memory[chip8->I + i];
                    }
                    break;
                }

                default: {
                    printf("Unknown opcode 0x%04X\n", chip8->opcode);
                    chip8_increment_pc(chip8);
                    break;
                }
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
        chip8->memory[FONT_START_ADDRESS + i] = chip8_font_set[i];
    }
}

bool chip8_load_rom(chip8_t* chip8, const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (fp == nullptr) {
        printf("Failed to open file %s\n", filename);
        return false;
    }

    int i = 0;
    uint8_t byte;
    while (fread(&byte, 1, 1, fp) == 1) {
        if (i + 0x200 >= 4096) {
            printf("Rom file is too large\n");
            fclose(fp);
            return false;
        }
        chip8->memory[i + 0x200] = byte;

        i += 1;
    }

    fclose(fp);

    return true;
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