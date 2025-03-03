#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <SDL2/SDL.h>

#define KEYPAD_SIZE 16

typedef struct {
    unsigned char keys[KEYPAD_SIZE]; // 0-F key states (0 = released, 1 = pressed)
} Keypad;

void keypad_init(Keypad* keypad);
void keypad_handle_event(Keypad* keypad, SDL_Event* event, int* running);

#endif // KEYBOARD_H