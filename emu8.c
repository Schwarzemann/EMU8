#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emu8.h"

#define FONTSET_START 0x000
#define FONTSET_SIZE 80
#define ROM_START 0x200
#define CACHE_SIZE 256  // Size of memory cache for sprite data
#define DEFAULT_SCALE 10
#define TARGET_FPS 60

static const unsigned char fontset[FONTSET_SIZE] = {
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

// Cache structure for frequently accessed memory regions
typedef struct {
    unsigned short address;
    unsigned char data[CACHE_SIZE];
    size_t size;
    time_t last_access;
} MemoryCache;

static MemoryCache sprite_cache;

static void initialize_sdl(Emu8* emu8) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        fprintf(stderr, "[%s] SDL initialization failed: %s\n", 
                __TIME__, SDL_GetError());
        exit(1);
    }
}

void create_window_and_renderer(Emu8* emu8, int scale, int fullscreen) {
    Uint32 window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    if (fullscreen) window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

    emu8->window = SDL_CreateWindow("EMU8",
                                    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                    SCREEN_WIDTH * scale, SCREEN_HEIGHT * scale,
                                    window_flags);
    if (!emu8->window) {
        fprintf(stderr, "[%s] Window creation failed: %s\n", 
                __TIME__, SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    emu8->renderer = SDL_CreateRenderer(emu8->window, -1, 
                                       SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
    if (!emu8->renderer) {
        fprintf(stderr, "[%s] Renderer creation failed: %s\n", 
                __TIME__, SDL_GetError());
        SDL_DestroyWindow(emu8->window);
        SDL_Quit();
        exit(1);
    }

    // Create a texture for double buffering and caching
    emu8->texture = SDL_CreateTexture(emu8->renderer, 
                                     SDL_PIXELFORMAT_ARGB8888, 
                                     SDL_TEXTUREACCESS_STREAMING, 
                                     SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!emu8->texture) {
        fprintf(stderr, "[%s] Texture creation failed: %s\n", 
                __TIME__, SDL_GetError());
        SDL_DestroyRenderer(emu8->renderer);
        SDL_DestroyWindow(emu8->window);
        SDL_Quit();
        exit(1);
    }

    SDL_RenderSetLogicalSize(emu8->renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
}

static void handle_interrupts(Emu8* emu8, Uint32 interval, void* data) {
    // Simulate CHIP-8 timer interrupts
    if (emu8->delay_timer > 0) emu8->delay_timer--;
    if (emu8->sound_timer > 0) emu8->sound_timer--;
}

void init_emu8(Emu8* emu8) {
    memset(emu8, 0, sizeof(Emu8));
    
    // Efficiently load fontset with memcpy
    memcpy(emu8->memory, fontset, sizeof(fontset));
    
    emu8->pc = ROM_START;
    printf("[%s] Initialized PC to 0x%04X\n", __TIME__, emu8->pc);

    // Initialize SDL with error handling
    initialize_sdl(emu8);

    // Initialize cache
    sprite_cache.address = 0;
    sprite_cache.size = 0;
    sprite_cache.last_access = time(NULL);

    // Create window and renderer with default scale and no fullscreen
    create_window_and_renderer(emu8, DEFAULT_SCALE, 0);

    // Set up interrupt timer for 60 Hz
    emu8->timer_id = SDL_AddTimer(1000 / TARGET_FPS, handle_interrupts, emu8);
    if (emu8->timer_id == 0) {
        fprintf(stderr, "[%s] Timer setup failed: %s\n", 
                __TIME__, SDL_GetError());
    }
}

void load_rom(Emu8* emu8, const char* filename) {
    FILE* rom = fopen(filename, "rb");
    if (!rom) {
        fprintf(stderr, "[%s] Failed to open ROM file: %s\n", __TIME__, filename);
        exit(1);
    }

    fseek(rom, 0, SEEK_END);
    long rom_size = ftell(rom);
    rewind(rom);

    if (rom_size > MEMORY_SIZE - ROM_START) {
        fprintf(stderr, "[%s] ROM too large: %ld bytes (max %d bytes)\n", 
                __TIME__, rom_size, MEMORY_SIZE - ROM_START);
        fclose(rom);
        exit(1);
    }

    size_t bytes_read = fread(&emu8->memory[ROM_START], 1, rom_size, rom);
    printf("[%s] Loaded %zu bytes from ROM, PC still at 0x%04X\n", 
           __TIME__, bytes_read, emu8->pc);

    // Cache the first part of the ROM for frequent access
    if (rom_size < CACHE_SIZE) {
        memcpy(sprite_cache.data, &emu8->memory[ROM_START], rom_size);
        sprite_cache.size = rom_size;
        sprite_cache.address = ROM_START;
        sprite_cache.last_access = time(NULL);
    }
    fclose(rom);
}

unsigned char* get_cached_memory(Emu8* emu8, unsigned short address, size_t size) {
    if (address >= sprite_cache.address && 
        address + size <= sprite_cache.address + sprite_cache.size) {
        sprite_cache.last_access = time(NULL);
        return &sprite_cache.data[address - sprite_cache.address];
    }
    
    // Update cache if not found (simple LRU-like replacement)
    if (size <= CACHE_SIZE) {
        memcpy(sprite_cache.data, &emu8->memory[address], size);
        sprite_cache.address = address;
        sprite_cache.size = size;
        sprite_cache.last_access = time(NULL);
        return sprite_cache.data;
    }
    
    return &emu8->memory[address]; // Fall back to direct access
}

void cleanup_emu8(Emu8* emu8) {
    if (emu8->timer_id) SDL_RemoveTimer(emu8->timer_id);
    if (emu8->texture) SDL_DestroyTexture(emu8->texture);
    if (emu8->renderer) SDL_DestroyRenderer(emu8->renderer);
    if (emu8->window) SDL_DestroyWindow(emu8->window);
    SDL_Quit();
}