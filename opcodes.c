#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opcodes.h"

void execute_opcode(Emu8* emu8, unsigned short opcode) {
    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x00FF) {
                case 0x00E0: // CLS
                    memset(emu8->display, 0, sizeof(emu8->display));
                    break;
                case 0x00EE: // RET
                    if (emu8->sp == 0) {
                        printf("Stack underflow at RET\n");
                        exit(1);
                    }
                    emu8->sp--;
                    emu8->pc = emu8->stack[emu8->sp];
                    break;
                default:
                    printf("Unknown opcode: 0x%X\n", opcode);
                    break;
            }
            break;

        case 0x1000: // JP addr
            emu8->pc = opcode & 0x0FFF;
            break;

        case 0x2000: // CALL addr
            if (emu8->sp >= STACK_SIZE) {
                printf("Stack overflow at CALL\n");
                exit(1);
            }
            emu8->stack[emu8->sp] = emu8->pc;
            emu8->sp++;
            emu8->pc = opcode & 0x0FFF;
            break;

        case 0x3000: // SE Vx, byte
            if (emu8->V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
                emu8->pc += 2;
            break;

        case 0x4000: // SNE Vx, byte
            if (emu8->V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
                emu8->pc += 2;
            break;

        case 0x6000: // LD Vx, byte
            emu8->V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            break;

        case 0x7000: // ADD Vx, byte
            emu8->V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            break;

        case 0xA000: // LD I, addr
            emu8->I = opcode & 0x0FFF;
            break;

        case 0xC000: // RND Vx, byte
            emu8->V[(opcode & 0x0F00) >> 8] = (rand() % 256) & (opcode & 0x00FF);
            break;

        case 0xD000: // DRW Vx, Vy, nibble
            {
                unsigned char x = emu8->V[(opcode & 0x0F00) >> 8];
                unsigned char y = emu8->V[(opcode & 0x00F0) >> 4];
                unsigned char height = opcode & 0x000F;
                emu8->V[0xF] = 0;

                for (int row = 0; row < height && (emu8->I + row) < MEMORY_SIZE; row++) {
                    unsigned char sprite_byte = emu8->memory[emu8->I + row];
                    for (int col = 0; col < 8; col++) {
                        if ((sprite_byte & (0x80 >> col)) != 0) {
                            int pixel_x = (x + col) % SCREEN_WIDTH;
                            int pixel_y = (y + row) % SCREEN_HEIGHT;
                            if (emu8->display[pixel_y][pixel_x] == 1) {
                                emu8->V[0xF] = 1;
                            }
                            emu8->display[pixel_y][pixel_x] ^= 1;
                        }
                    }
                }
            }
            break;

        case 0xE000: // Key-related opcodes
            switch (opcode & 0x00FF) {
                case 0x9E: // SKP Vx
                    if (emu8->keypad.keys[emu8->V[(opcode & 0x0F00) >> 8]])
                        emu8->pc += 2;
                    break;
                case 0xA1: // SKNP Vx
                    if (!emu8->keypad.keys[emu8->V[(opcode & 0x0F00) >> 8]])
                        emu8->pc += 2;
                    break;
                default:
                    printf("Unknown opcode: 0x%X\n", opcode);
                    break;
            }
            break;

        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x07: // LD Vx, DT
                    emu8->V[(opcode & 0x0F00) >> 8] = emu8->delay_timer;
                    break;

                case 0x0A: // LD Vx, K (Wait for keypress)
                    {
                        int key = keypad_get_pressed_key(&emu8->keypad);
                        if (key >= 0) {
                            emu8->V[(opcode & 0x0F00) >> 8] = (unsigned char)key;
                        } else {
                            emu8->pc -= 2; // Stay on this instruction until a key is pressed
                        }
                    }
                    break;

                case 0x15: // LD DT, Vx
                    emu8->delay_timer = emu8->V[(opcode & 0x0F00) >> 8];
                    break;

                case 0x18: // LD ST, Vx
                    emu8->sound_timer = emu8->V[(opcode & 0x0F00) >> 8];
                    break;

                case 0x1E: // ADD I, Vx
                    emu8->I += emu8->V[(opcode & 0x0F00) >> 8];
                    break;

                case 0x29: // LD F, Vx
                    emu8->I = emu8->V[(opcode & 0x0F00) >> 8] * 5;
                    break;

                case 0x33: // LD B, Vx
                    {
                        unsigned char vx = emu8->V[(opcode & 0x0F00) >> 8];
                        emu8->memory[emu8->I] = vx / 100;
                        emu8->memory[emu8->I + 1] = (vx / 10) % 10;
                        emu8->memory[emu8->I + 2] = vx % 10;
                    }
                    break;

                case 0x65: // LD Vx, [I]
                    {
                        unsigned char vx = (opcode & 0x0F00) >> 8;
                        for (int i = 0; i <= vx && (emu8->I + i) < MEMORY_SIZE; i++) {
                            emu8->V[i] = emu8->memory[emu8->I + i];
                        }
                    }
                    break;

                default:
                    printf("Unknown opcode: 0x%X\n", opcode);
                    break;
            }
            break;

        default:
            printf("Unknown opcode: 0x%X\n", opcode);
            break;
    }
}