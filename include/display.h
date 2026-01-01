/**
 * Author: Henry Jochaniewicz
 * Date modified: December 30, 2025
 **/
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#include "settings.h"

struct screen {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_AudioStream* stream;
};

bool init_screen(struct screen* screen);
bool handle_event(void);
bool is_key_pressed(uint8_t num);
uint8_t any_key_pressed();
void draw_display(SDL_Renderer* renderer, bool display[][WIDTH]);
void clear_display(bool display[][WIDTH]);
void destroy_screen(struct screen* screen);
void play_sound(SDL_AudioStream* stream, uint8_t timer_value);

#endif
