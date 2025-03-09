#include <stdio.h>
#include <time.h>
#include <string.h>
#include "emu8.h"
#include "cpu.h"
#include "keyboard.h"

#define DEFAULT_SCALE 10
#define TARGET_FPS 60

void render_display(Emu8* emu8) {
    void* pixels;
    int pitch;
    if (SDL_LockTexture(emu8->texture, NULL, &pixels, &pitch) < 0) {
        fprintf(stderr, "[%s] Texture lock failed: %s\n", __TIME__, SDL_GetError());
        return;
    }

    Uint32* pixel_data = (Uint32*)pixels;
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            pixel_data[y * (pitch / sizeof(Uint32)) + x] = 
                emu8->display[y][x] ? 0xFFFFFFFF : 0xFF000000;
        }
    }

    SDL_UnlockTexture(emu8->texture);

    SDL_SetRenderDrawColor(emu8->renderer, 0, 0, 0, 255);
    SDL_RenderClear(emu8->renderer);
    SDL_RenderCopy(emu8->renderer, emu8->texture, NULL, NULL);
    SDL_RenderPresent(emu8->renderer);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <rom_file> [-s scale] [-f]\n", argv[0]);
        printf("  -s scale: Set window scale (default %d, e.g., -s 15 for 15x)\n", DEFAULT_SCALE);
        printf("  -f: Enable full-screen mode\n");
        return 1;
    }

    int scale = DEFAULT_SCALE;
    int fullscreen = 0;
    const char* rom_file = argv[1];

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            scale = atoi(argv[++i]);
            if (scale < 1) scale = DEFAULT_SCALE;
        } else if (strcmp(argv[i], "-f") == 0) {
            fullscreen = 1;
        }
    }

    srand(time(NULL));
    Emu8 emu8;
    init_emu8(&emu8);

    cleanup_emu8(&emu8); // Reset SDL state
    create_window_and_renderer(&emu8, scale, fullscreen);

    load_rom(&emu8, rom_file);

    int running = 1;
    SDL_Event event;

    const Uint32 frame_delay = 1000 / TARGET_FPS;
    Uint32 frame_start;

    while (running) {
        frame_start = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    keypad_handle_event(&emu8.keypad, &event, &running);
                    break;
                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        SDL_RenderSetLogicalSize(emu8.renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
                    }
                    break;
            }
        }

        emulate_cycle(&emu8);
        disassemble_log(&emu8);
        update_timers(&emu8);

        render_display(&emu8);

        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < frame_delay) {
            SDL_Delay(frame_delay - frame_time);
        }
    }

    cleanup_emu8(&emu8);
    return 0;
}