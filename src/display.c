/**
 * Author: Henry Jochaniewicz
 * Date modified: December 30, 2025
 **/
#include <SDL3/SDL.h>
#include <SDL3/SDL_keyboard.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "display.h"
#include "interpret.h"
#include "debug.h"

#define FACTOR 10

/**
 * Array to define the scancodes for 0 - F.
 * For the following structure (keypad):
 *  1 2 3 C
 *  4 5 6 D
 *  7 8 9 E
 *  A 0 B F
 * Made these the following keys on a keyboard:
 *  1 2 3 4
 *  q w e r
 *  a s d f
 *  z x c v
 * This is independent of QWERTY/AZERTY/etc.
 **/
static SDL_Scancode codes[] = {
    SDL_SCANCODE_X, // 0.
    SDL_SCANCODE_1, // 1.
    SDL_SCANCODE_2, // 2.
    SDL_SCANCODE_3, // 3.
    SDL_SCANCODE_Q, // 4.
    SDL_SCANCODE_W, // 5.
    SDL_SCANCODE_E, // 6.
    SDL_SCANCODE_A, // 7.
    SDL_SCANCODE_S, // 8.
    SDL_SCANCODE_D, // 9.
    SDL_SCANCODE_Z, // A.
    SDL_SCANCODE_C, // B.
    SDL_SCANCODE_4, // C.
    SDL_SCANCODE_R, // D.
    SDL_SCANCODE_F, // E.
    SDL_SCANCODE_V  // F.
};

/**
 * Callback to generate frequency for sound sampling.
 * @param   additional_amount   how much audio stream needs than what is queued currently
 * @param   total_amount        how much data audio stream eating atm
 **/
void SDLCALL callback(void* userdata, SDL_AudioStream* stream, 
        int additional_amount, int total_amount) {
    static uint32_t current_sample = 0;
    additional_amount /= sizeof(float);
#define SAMPLE_SIZE 128
    while(additional_amount > 0) {
        float samples[SAMPLE_SIZE];
        const int total = additional_amount < SAMPLE_SIZE ? additional_amount : SAMPLE_SIZE;
        for(int i = 0; i < total; i++) {
            const int freq = 220;
            const float phase = current_sample * freq / 8000.0f;
            samples[i] = SDL_sinf(phase * 2 * SDL_PI_F);
            current_sample++;
        }
        current_sample %= 8000;
        SDL_PutAudioStreamData(stream, samples, total * sizeof(float));
        additional_amount -= total;
    }
}

bool init_screen(struct screen* screen) {
    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return false;
    }

    if(!SDL_CreateWindowAndRenderer("CHIP-8", WIDTH * FACTOR, HEIGHT * FACTOR,
            SDL_WINDOW_RESIZABLE, &screen->window, &screen->renderer)) {
        fprintf(stderr, "Failed to create window and renderer: %s\n", SDL_GetError());
        return false;
    }

    if(!SDL_SetRenderLogicalPresentation(screen->renderer, WIDTH, HEIGHT, 
                SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
        fprintf(stderr, "FAILURE: %s\n", SDL_GetError());
    }

    SDL_AudioSpec spec = {
        .format = SDL_AUDIO_F32,
        .channels = 1,
        .freq = 8000
    };

    screen->stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
            &spec, callback, NULL);

    if(screen->stream == NULL) {
        fprintf(stderr, "Failure in generating audio device: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

bool handle_event() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_EVENT_QUIT:
                return false;
            case SDL_EVENT_KEY_DOWN:
                if(event.key.key == SDLK_ESCAPE) {
                    return false;
                }
        }
    }

    return true;
}

bool is_key_pressed(uint8_t num) {
    if(num > 0xF) return false;
    SDL_Scancode code = codes[num];
    int length = 0;
    const bool* keys = SDL_GetKeyboardState(&length);
    return keys[code];
}

// Returns 0xFF on failure.
uint8_t any_key_pressed() {
    int length = 0;
    const bool* keys = SDL_GetKeyboardState(&length);
    for(uint8_t i = 0; i <= 0xF; i++) {
        if(keys[codes[i]]) return i;
    }
    return 0xFF;
}

void draw_display(SDL_Renderer* renderer, bool display[][WIDTH]) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    if(!SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, SDL_ALPHA_OPAQUE)) {
        fprintf(stdout, "FAILURE: %s\n", SDL_GetError());
    }
    SDL_FRect r = {0};
    r.w = 1;
    r.h = 1;

    for(uint32_t i = 0; i < HEIGHT; i++) {
        for(uint32_t j = 0; j < WIDTH; j++) {
            if(!display[i][j]) continue;
            r.x = (float) j;
            r.y = (float) i;
            if(!SDL_RenderFillRect(renderer, &r)) {
                fprintf(stderr, "FAILURE: %s\n", SDL_GetError());
            }
        }
    }
    
    if(!SDL_RenderPresent(renderer)) {
        fprintf(stderr, "FAILURE: %s\n", SDL_GetError());
    }
}

void clear_display(bool display[][WIDTH]) {
    memset(display, false, HEIGHT * WIDTH);
}

void destroy_screen(struct screen* screen) {
    SDL_DestroyRenderer(screen->renderer);
    SDL_DestroyWindow(screen->window);
    SDL_DestroyAudioStream(screen->stream);
    SDL_Quit();
}

void play_sound(SDL_AudioStream* stream, uint8_t timer_value) {
    static bool is_playing = false;

    if(timer_value == 0 && is_playing) {
        is_playing = false;
        SDL_PauseAudioStreamDevice(stream);
    } else if(timer_value != 0 && !is_playing) {
        is_playing = true;
        SDL_ResumeAudioStreamDevice(stream);
    }
}

