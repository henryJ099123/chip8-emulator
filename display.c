#include <SDL3/SDL.h>
#include <stdbool.h>
#include <string.h>

#include "display.h"
#include "interpret.h"
#include "debug.h"

#define FACTOR 10

bool init_screen(SDL_Window** window, SDL_Renderer** renderer) {
    if(!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return false;
    }

    if(!SDL_CreateWindowAndRenderer("CHIP-8", WIDTH * FACTOR, HEIGHT * FACTOR,
            SDL_WINDOW_RESIZABLE, window, renderer)) {
        fprintf(stderr, "Failed to create window and renderer: %s\n", SDL_GetError());
        return false;
    }

    if(!SDL_SetRenderLogicalPresentation(*renderer, WIDTH, HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
        fprintf(stderr, "FAILURE: %s\n", SDL_GetError());
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

void destroy_screen(SDL_Window* window, SDL_Renderer* renderer) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

