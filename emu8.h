#ifndef EMU8_H
#define EMU8_H

#define MEMORY_SIZE 4096
#define REGISTER_COUNT 16
#define STACK_SIZE 16
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

#include <SDL2/SDL.h>
#include "keyboard.h"

typedef struct {
    unsigned char memory[MEMORY_SIZE];
    unsigned char V[REGISTER_COUNT];    // Registers V0-VF
    unsigned short I;                   // Index register
    unsigned short pc;                  // Program counter
    unsigned short stack[STACK_SIZE];
    unsigned short sp;                  // Stack pointer
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned char display[SCREEN_HEIGHT][SCREEN_WIDTH];
    Keypad keypad;                      // Keyboard support.
    SDL_Window* window;                 // SDL window
    SDL_Renderer* renderer;             // SDL renderer
    SDL_Texture* texture;               // SDL texture for caching
    SDL_TimerID timer_id;               // Timer ID for interrupts
} Emu8;

void init_emu8(Emu8* emu8);
void load_rom(Emu8* emu8, const char* filename);
unsigned char* get_cached_memory(Emu8* emu8, unsigned short address, size_t size);
void cleanup_emu8(Emu8* emu8);
void create_window_and_renderer(Emu8* emu8, int scale, int fullscreen); // Added declaration

#endif