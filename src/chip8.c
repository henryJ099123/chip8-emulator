/**
 * Author: Henry Jochaniewicz
 * Date modified: December 31, 2025
 **/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL3/SDL.h>
#include <fcntl.h>
#include <time.h>

#include "settings.h"
#include "memory.h"
#include "display.h"
#include "interpret.h"
#include "debug.h"

int main(int argc, char* argv[]) {
    int debug = 0;
    srand(time(NULL));
    
    if(argc != 2) {
        fprintf(stderr, "Usage: ./chip8 <file>\n");
        return EXIT_SUCCESS;
    }

    int fd = open(argv[1], O_RDONLY);
    if(open < 0) {
        fprintf(stderr, "Failure in reading '%s'\n", argv[1]);
        fprintf(stderr, "Usage: ./chip8 <file>\n");
        return EXIT_FAILURE;
    }

    struct interpreter interpreter = {0};
    interpreter.program_counter = START_ADDRESS;

    if(load_code(interpreter.memory, fd) < 0) {
        fprintf(stderr, "Failure in copying code from '%s'\n", argv[1]);
        return EXIT_FAILURE;
    }

    initialize_font(interpreter.memory);

    struct screen screen = {0};
    if(!init_screen(&screen)) {
        return EXIT_FAILURE;
    }

    if(debug) {
        debugger(&interpreter, &screen);
        return EXIT_SUCCESS;
    }

    clock_t cpu_clock = clock();
    clock_t timer_clock = clock();

    for(;;) {
        if((double) (clock() - cpu_clock) < CYCLE_TIME * CLOCKS_PER_SEC) continue;
        cpu_clock += CYCLE_TIME * CLOCKS_PER_SEC;
        if(!handle_event()) break;
        uint16_t instruction = fetch(&interpreter);
        if(decode(&interpreter, instruction)) {
            draw_display(screen.renderer, interpreter.display);
        }

        if((double) (clock() - timer_clock) < TIMER_CYCLE_TIME * CLOCKS_PER_SEC) continue;
        timer_clock += TIMER_CYCLE_TIME * CLOCKS_PER_SEC;
        update_timers(&interpreter, &screen);
    }

    destroy_screen(&screen);
    return 0;
}

