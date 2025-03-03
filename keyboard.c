#include <stdio.h>
#include "keyboard.h"

void keypad_init(Keypad* keypad) {
    // Initialize all keys to released state
    for (int i = 0; i < KEYPAD_SIZE; i++) {
        keypad->keys[i] = 0;
    }
}

void keypad_handle_event(Keypad* keypad, SDL_Event* event, int* running) {
    switch (event->type) {
        case SDL_KEYDOWN:
            switch (event->key.keysym.sym) {
                case SDLK_1: keypad->keys[0x1] = 1; break;
                case SDLK_2: keypad->keys[0x2] = 1; break;
                case SDLK_3: keypad->keys[0x3] = 1; break;
                case SDLK_4: keypad->keys[0xC] = 1; break;
                case SDLK_q: keypad->keys[0x4] = 1; break;
                case SDLK_w: keypad->keys[0x5] = 1; break;
                case SDLK_e: keypad->keys[0x6] = 1; break;
                case SDLK_r: keypad->keys[0xD] = 1; break;
                case SDLK_a: keypad->keys[0x7] = 1; break;
                case SDLK_s: keypad->keys[0x8] = 1; break;
                case SDLK_d: keypad->keys[0x9] = 1; break;
                case SDLK_f: keypad->keys[0xE] = 1; break;
                case SDLK_z: keypad->keys[0xA] = 1; break;
                case SDLK_x: keypad->keys[0x0] = 1; break;
                case SDLK_c: keypad->keys[0xB] = 1; break;
                case SDLK_v: keypad->keys[0xF] = 1; break;
                case SDLK_ESCAPE: *running = 0; break; // Optional: Quit with Escape
            }
            break;
        case SDL_KEYUP:
            switch (event->key.keysym.sym) {
                case SDLK_1: keypad->keys[0x1] = 0; break;
                case SDLK_2: keypad->keys[0x2] = 0; break;
                case SDLK_3: keypad->keys[0x3] = 0; break;
                case SDLK_4: keypad->keys[0xC] = 0; break;
                case SDLK_q: keypad->keys[0x4] = 0; break;
                case SDLK_w: keypad->keys[0x5] = 0; break;
                case SDLK_e: keypad->keys[0x6] = 0; break;
                case SDLK_r: keypad->keys[0xD] = 0; break;
                case SDLK_a: keypad->keys[0x7] = 0; break;
                case SDLK_s: keypad->keys[0x8] = 0; break;
                case SDLK_d: keypad->keys[0x9] = 0; break;
                case SDLK_f: keypad->keys[0xE] = 0; break;
                case SDLK_z: keypad->keys[0xA] = 0; break;
                case SDLK_x: keypad->keys[0x0] = 0; break;
                case SDLK_c: keypad->keys[0xB] = 0; break;
                case SDLK_v: keypad->keys[0xF] = 0; break;
            }
            break;
    }
}