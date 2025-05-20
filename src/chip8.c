// CHIP-8 Core Emulator (Corrected)
#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

uint8_t chip8_font_set[FONT_SET_SIZE] = {
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

void chip8_clear_graphics(chip8_t *chip8)
{
    memset(chip8->gfx, 0, sizeof(chip8->gfx));
    chip8->draw_flag = true;
}

void chip8_increment_pc(chip8_t *chip8)
{
    chip8->pc += 2;
}

void chip8_init(chip8_t *chip8)
{
    srand((unsigned int)time(NULL));
    memset(chip8, 0, sizeof(*chip8));
    chip8->pc = 0x200;
    for (int i = 0; i < FONT_SET_SIZE; i++)
    {
        chip8->memory[FONT_START_ADDRESS + i] = chip8_font_set[i];
    }
    chip8_clear_graphics(chip8);
}

bool chip8_load_rom(chip8_t *chip8, const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        printf("Failed to open file %s\n", filename);
        return false;
    }
    int i = 0;
    uint8_t byte;
    while (fread(&byte, 1, 1, fp) == 1)
    {
        if (i + 0x200 >= 4096)
        {
            printf("ROM too large\n");
            fclose(fp);
            return false;
        }
        chip8->memory[0x200 + i++] = byte;
    }
    fclose(fp);
    return true;
}

void chip8_cycle(chip8_t *chip8)
{
    chip8->opcode = chip8->memory[chip8->pc] << 8 | chip8->memory[chip8->pc + 1];
    uint16_t x = (chip8->opcode & 0x0F00) >> 8;
    uint16_t y = (chip8->opcode & 0x00F0) >> 4;
    uint16_t nnn = chip8->opcode & 0x0FFF;
    uint8_t kk = chip8->opcode & 0x00FF;
    uint8_t n = chip8->opcode & 0x000F;

    switch (chip8->opcode & 0xF000)
    {
    case 0x0000:
        switch (chip8->opcode)
        {
        case 0x00E0:
            chip8_clear_graphics(chip8);
            chip8_increment_pc(chip8);
            break;
        case 0x00EE:
            if (chip8->sp == 0)
            {
                printf("Stack underflow\n");
                return;
            }
            chip8->sp--;
            chip8->pc = chip8->stack[chip8->sp];
            break;
        default:
            printf("Unknown opcode 0x%04X\n", chip8->opcode);
            chip8_increment_pc(chip8);
            break;
        }
        break;

    case 0x1000:
        chip8->pc = nnn;
        break;
    case 0x2000:
        if (chip8->sp >= 16)
        {
            printf("Stack overflow\n");
            return;
        }
        chip8->stack[chip8->sp++] = chip8->pc + 2;
        chip8->pc = nnn;
        break;
    case 0x3000:
        chip8->pc += (chip8->V[x] == kk) ? 4 : 2;
        break;
    case 0x4000:
        chip8->pc += (chip8->V[x] != kk) ? 4 : 2;
        break;
    case 0x5000:
        chip8->pc += (chip8->V[x] == chip8->V[y]) ? 4 : 2;
        break;
    case 0x6000:
        chip8->V[x] = kk;
        chip8_increment_pc(chip8);
        break;
    case 0x7000:
        chip8->V[x] += kk;
        chip8_increment_pc(chip8);
        break;
    case 0x8000:
        switch (chip8->opcode & 0x000F)
        {
        case 0x0:
            chip8->V[x] = chip8->V[y];
            break;
        case 0x1:
            chip8->V[x] |= chip8->V[y];
            break;
        case 0x2:
            chip8->V[x] &= chip8->V[y];
            break;
        case 0x3:
            chip8->V[x] ^= chip8->V[y];
            break;
        case 0x4: {
            uint16_t sum = chip8->V[x] + chip8->V[y];
            chip8->V[0xF] = sum > 0xFF;
            chip8->V[x] = sum & 0xFF;
            break;
        }
        case 0x5:
            chip8->V[0xF] = chip8->V[x] > chip8->V[y];
            chip8->V[x] -= chip8->V[y];
            break;
        case 0x6:
            chip8->V[0xF] = chip8->V[x] & 0x1;
            chip8->V[x] >>= 1;
            break;
        case 0x7:
            chip8->V[0xF] = chip8->V[y] > chip8->V[x];
            chip8->V[x] = chip8->V[y] - chip8->V[x];
            break;
        case 0xE:
            chip8->V[0xF] = (chip8->V[x] & 0x80) >> 7;
            chip8->V[x] <<= 1;
            break;
        default:
            printf("Unknown 8xy* opcode 0x%04X\n", chip8->opcode);
            break;
        }
        chip8_increment_pc(chip8);
        break;

    case 0x9000:
        chip8->pc += (chip8->V[x] != chip8->V[y]) ? 4 : 2;
        break;
    case 0xA000:
        chip8->I = nnn;
        chip8_increment_pc(chip8);
        break;
    case 0xB000:
        chip8->pc = nnn + chip8->V[0];
        break;
    case 0xC000:
        chip8->V[x] = (rand() % 256) & kk;
        chip8_increment_pc(chip8);
        break;
    case 0xD000: {
        uint8_t vx = chip8->V[x] % 64;
        uint8_t vy = chip8->V[y] % 32;
        chip8->V[0xF] = 0;
        chip8->draw_flag = true;

        for (int row = 0; row < n; row++)
        {
            uint8_t sprite = chip8->memory[chip8->I + row];
            for (int col = 0; col < 8; col++)
            {
                if ((sprite & (0x80 >> col)) != 0)
                {
                    uint16_t index = (vx + col) + ((vy + row) * 64);
                    if (chip8->gfx[index])
                        chip8->V[0xF] = 1;
                    chip8->gfx[index] ^= 1;
                }
            }
        }
        chip8_increment_pc(chip8);
        break;
    }

    case 0xE000:
        switch (kk)
        {
        case 0x9E:
            if (chip8->key[chip8->V[x]])
                chip8_increment_pc(chip8);
            chip8_increment_pc(chip8);
            break;
        case 0xA1:
            if (!chip8->key[chip8->V[x]])
                chip8_increment_pc(chip8);
            chip8_increment_pc(chip8);
            break;
        default:
            printf("Unknown Ex** opcode: 0x%04X\n", chip8->opcode);
            chip8_increment_pc(chip8);
            break;
        }
        break;

    case 0xF000:
        switch (kk)
        {
        case 0x07:
            chip8->V[x] = chip8->timer_delay;
            break;
        case 0x0A: {
            for (int i = 0; i < 16; i++)
            {
                if (chip8->key[i])
                {
                    chip8->V[x] = i;
                    chip8_increment_pc(chip8);
                    return;
                }
            }
            return; // block until keypress
        }
        case 0x15:
            chip8->timer_delay = chip8->V[x];
            break;
        case 0x18:
            chip8->timer_sound = chip8->V[x];
            break;
        case 0x1E:
            chip8->I += chip8->V[x];
            break;
        case 0x29:
            chip8->I = FONT_START_ADDRESS + chip8->V[x] * 5;
            break;
        case 0x33:
            chip8->memory[chip8->I] = chip8->V[x] / 100;
            chip8->memory[chip8->I + 1] = (chip8->V[x] / 10) % 10;
            chip8->memory[chip8->I + 2] = chip8->V[x] % 10;
            break;
        case 0x55:
            for (int i = 0; i <= x; i++)
                chip8->memory[chip8->I + i] = chip8->V[i];
            break;
        case 0x65:
            for (int i = 0; i <= x; i++)
                chip8->V[i] = chip8->memory[chip8->I + i];
            break;
        default:
            printf("Unknown Fx** opcode: 0x%04X\n", chip8->opcode);
            break;
        }
        chip8_increment_pc(chip8);
        break;

    default:
        printf("Unknown opcode: 0x%04X\n", chip8->opcode);
        chip8_increment_pc(chip8);
        break;
    }
}
