#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>  // for usleep (optional, for timing control)
#include "cpu.h"

// Disassembler
const char* disassemble_opcode(unsigned short opcode) {
    static char buffer[64];
    unsigned char vx = (opcode & 0x0F00) >> 8;
    unsigned char vy = (opcode & 0x00F0) >> 4;
    unsigned char nibble = opcode & 0x000F;
    unsigned short addr = opcode & 0x0FFF;
    unsigned char byte = opcode & 0x00FF;

    switch (opcode & 0xF000) {
        case 0x0000:
            if (opcode == 0x00E0) return "CLS";   // CLS
            if (opcode == 0x00EE) return "RET";   // RET
            break;

        case 0x1000: snprintf(buffer, sizeof(buffer), "JP 0x%03X", addr); return buffer; // JP addr
        case 0x2000: snprintf(buffer, sizeof(buffer), "CALL 0x%03X", addr); return buffer; // CALL addr

        case 0x3000: snprintf(buffer, sizeof(buffer), "SE V%X, 0x%02X", vx, byte); return buffer; // SE Vx, byte
        case 0x4000: snprintf(buffer, sizeof(buffer), "SNE V%X, 0x%02X", vx, byte); return buffer; // SNE Vx, byte
        case 0x6000: snprintf(buffer, sizeof(buffer), "LD V%X, 0x%02X", vx, byte); return buffer; // LD Vx, byte
        case 0x7000: snprintf(buffer, sizeof(buffer), "ADD V%X, 0x%02X", vx, byte); return buffer; // ADD Vx, byte

        case 0xA000: snprintf(buffer, sizeof(buffer), "LD I, 0x%03X", addr); return buffer; // LD I, addr
        case 0xC000: snprintf(buffer, sizeof(buffer), "RND V%X, 0x%02X", vx, byte); return buffer; // RND Vx, byte

        case 0xD000: snprintf(buffer, sizeof(buffer), "DRW V%X, V%X, 0x%X", vx, vy, nibble); return buffer; // DRW Vx, Vy, nibble

        case 0xE000:
            if (opcode == 0xE09E) snprintf(buffer, sizeof(buffer), "SKP V%X", vx); return buffer; // SKP Vx
            if (opcode == 0xE0A1) snprintf(buffer, sizeof(buffer), "SKNP V%X", vx); return buffer; // SKNP Vx
            break;

        case 0xF000:
            switch (opcode) {
                case 0xF007: snprintf(buffer, sizeof(buffer), "LD V%X, DT", vx); return buffer; // LD Vx, DT
                case 0xF00A: snprintf(buffer, sizeof(buffer), "LD V%X, K", vx); return buffer; // LD Vx, K
                case 0xF015: snprintf(buffer, sizeof(buffer), "LD DT, V%X", vx); return buffer; // LD DT, Vx
                case 0xF018: snprintf(buffer, sizeof(buffer), "LD ST, V%X", vx); return buffer; // LD ST, Vx
                case 0xF01E: snprintf(buffer, sizeof(buffer), "ADD I, V%X", vx); return buffer; // ADD I, Vx
                case 0xF029: snprintf(buffer, sizeof(buffer), "LD F, V%X", vx); return buffer; // LD F, Vx
                case 0xF033: snprintf(buffer, sizeof(buffer), "LD B, V%X", vx); return buffer; // LD B, Vx
                case 0xF055: snprintf(buffer, sizeof(buffer), "LD [I], V%X", vx); return buffer; // LD [I], Vx
                case 0xF065: snprintf(buffer, sizeof(buffer), "LD V%X, [I]", vx); return buffer; // LD Vx, [I]
                default:
                    return "Unknown 0xF000 opcode";
            }

        case 0x9000: snprintf(buffer, sizeof(buffer), "SNE V%X, V%X", vx, vy); return buffer; // SE Vx, Vy

        case 0xB000: snprintf(buffer, sizeof(buffer), "JP V0, 0x%03X", addr); return buffer; // JP V0, addr

        default:
            return "Unknown opcode";
    }

    return "Unknown opcode";
}


// Function to disassemble the next given amount of opcodes
void disassemble_log(Emu8* emu8) {
    printf("Disassembled Instructions:\n");

    for (int i = 0; i < 35; i++) {
        unsigned short opcode = emu8->memory[emu8->pc] << 8 | emu8->memory[emu8->pc + 1];
        const char* disassembled_opcode = disassemble_opcode(opcode);
        printf("0x%04X: %s\n", emu8->pc, disassembled_opcode);
        emu8->pc += 2;  // Move PC for the next instruction
    }

    // Move the PC back to its original position
    emu8->pc -= 70;
    // usleep(100000);
}

void debug_logging(Emu8* emu8, unsigned short opcode) {
    // Clear the terminal to refresh the debug info
    printf("\033[H\033[J");

    printf("\r=== EMU8 State ===\n");
    printf("\rProgram Counter     : 0x%04X   Opcode: 0x%04X\n", emu8->pc, opcode);
    printf("\rIndex Register      : 0x%04X   Stack Pointer    : 0x%02X\n", emu8->I, emu8->sp);
    printf("\rDelay Timer         : 0x%02X     Sound Timer      : 0x%02X\n", emu8->delay_timer, emu8->sound_timer);
    printf("\r--------------------\n");

    // Print Registers (V0-VF)
    printf("\rRegisters (V0-VF):\n");
    for (int i = 0; i < REGISTER_COUNT; i += 4) {
        printf("\rV%02d   : 0x%02X   V%02d   : 0x%02X   V%02d   : 0x%02X   V%02d   : 0x%02X\n",
               i, emu8->V[i],
               i + 1, emu8->V[i + 1],
               i + 2, emu8->V[i + 2],
               i + 3, emu8->V[i + 3]);
    }
    printf("\r--------------------\n");

    // Print stack contents
    printf("\rStack (Top %d entries):\n", emu8->sp);
    if (emu8->sp == 0) {
        printf("\r  (empty)\n");
    } else {
        for (int i = 0; i < emu8->sp && i < STACK_SIZE; i++) {
            printf("\r  Stack[%02d]: 0x%04X\n", i, emu8->stack[i]);
        }
    }

    // Print memory dump of the opcode area
    printf("\r\nMemory (Opcode context):\n");
    for (int i = emu8->pc; i < emu8->pc + 8 && i < MEMORY_SIZE; i++) {
        printf("\r 0x%04X: 0x%02X\n", i, emu8->memory[i]);
    }

    printf("\r==================\n\n");
    // usleep(100000);
}

// Main cycle function that controls the CPU operations
// Main cycle function
void emulate_cycle(Emu8* emu8) {
    // Check PC bounds
    if (emu8->pc >= MEMORY_SIZE - 1) {
        printf("PC out of bounds: 0x%X\n", emu8->pc);
        exit(1);
    }

    unsigned short opcode = emu8->memory[emu8->pc] << 8 | emu8->memory[emu8->pc + 1];

    debug_logging(emu8, opcode);

    emu8->pc += 2;
    
    execute_opcode(emu8, opcode);
}

// Update timers for delay and sound
void update_timers(Emu8* emu8) {
    if (emu8->delay_timer > 0)
        emu8->delay_timer--;
    
    if (emu8->sound_timer > 0)
        emu8->sound_timer--;
}
