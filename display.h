#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <SDL3/SDL.h>
#include <stdbool.h>

#include "interpret.h"

bool init_screen(SDL_Window** window, SDL_Renderer** renderer);
bool handle_event(void);
void draw_display(SDL_Renderer* renderer, bool display[][WIDTH]);
void clear_display(bool display[][WIDTH]);
void destroy_screen(SDL_Window* window, SDL_Renderer* renderer);

#endif
