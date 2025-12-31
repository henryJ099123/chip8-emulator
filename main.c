/**
 * Author: Henry Jochaniewicz
 * Date modified: December 30, 2025
 **/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL3/SDL.h>
#include <fcntl.h>
#include <time.h>

#include "memory.h"
#include "display.h"
#include "interpret.h"
#include "debug.h"

#define FREQUENCY 400 // in Hz. Modifiable.
#define CYCLE_TIME (1.0 / (FREQUENCY))

#define SHIFT_OPTION
#define JUMP_OFFSET_OPTION
#define INDEX_ADD_OPTION
#define INDEX_INC_MEMORY_OPTION

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

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    if(!init_screen(&window, &renderer)) {
        return EXIT_FAILURE;
    }

    uint16_t instruction = 0xABCD;

    if(debug) {
        for(;;) {
            if(!handle_event()) break;
            fprintf(stdout, ">> ");
            switch(fgetc(stdin)) {
                case 'n':
                    instruction = fetch(&interpreter);
                    fprintf(stdout, "Fetched instruction %4X\n", instruction);
                    decode(&interpreter, instruction); 
                    break;
                case 'i':
                    fprintf(stdout, "I: %4X\n", interpreter.index_register);
                    break;
                case 'p':
                    fprintf(stdout, "PC: %4X\n", interpreter.program_counter);
                    break;
                case 'm':
                    dump_memory(stdout, interpreter.memory);
                    break;
                case 'd':
                    dump_display(stdout, interpreter.display);
                    break;
                case 'r':
                    dump_registers(stdout, interpreter.registers);
                    break;
                case 'q':
                    return EXIT_SUCCESS;
            }
            fgetc(stdin); // eat '\n'
        }
        return 1;
    }

    clock_t clock_time = clock();
    clock_t second_counter = clock();

    int instr_count = 0;
    for(;;) {
        if((double) (clock() - clock_time) / CLOCKS_PER_SEC < CYCLE_TIME) continue;
        clock_time = clock();
        if(!handle_event()) break;
        uint16_t instruction = fetch(&interpreter);
        if(decode(&interpreter, instruction)) {
            draw_display(renderer, interpreter.display);
        }
        if(interpreter.delay_timer != 0) interpreter.delay_timer--;
        if(interpreter.sound_timer != 0) interpreter.sound_timer--;
        instr_count++;
        if((clock() - second_counter) / CLOCKS_PER_SEC > 1) {
            fprintf(stdout, "Instruction count: %d\n", instr_count);
            instr_count = 0;
            second_counter = clock();
        }
    }

    destroy_screen(window, renderer);
    return 0;
}

