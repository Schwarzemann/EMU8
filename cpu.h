#ifndef CPU_H
#define CPU_H

#include "emu8.h"

void emulate_cycle(Emu8* emu8);
void update_timers(Emu8* emu8);
void disassemble_log(Emu8* emu8);

#endif