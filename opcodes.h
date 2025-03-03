#ifndef OPCODES_H
#define OPCODES_H

#include "emu8.h" // For Emu8 struct

// Function to handle a single opcode
void execute_opcode(Emu8* emu8, unsigned short opcode);

#endif // OPCODES_H