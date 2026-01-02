/**
 * Author: Henry Jochaniewicz
 * Date modified: 01/01/26
 **/
#include <stdint.h>
#include <stdio.h>

#include "settings.h"
#include "debug.h"
#include "memory.h"
#include "screen.h"

void dump_memory(FILE* fp, uint8_t* memory) {
    fprintf(fp, "\t== MEMORY ==");
    for(uint32_t i = 0; i < MEMORY_SIZE; i+= 2) {
        if((i & 0xF) == 0) {
            fprintf(fp, "\n%4d (%03x).", i, i);
        }

        fprintf(fp, " %02X%02X", memory[i], memory[i+1]);
    }
    fprintf(fp, "\n\t== MEMORY END ==\n");
}

void dump_stack(FILE* fp, struct stack* stack) {
    fprintf(fp, "\t== STACK ==");
    for(uint32_t i = 0; i < stack->pointer; i++) {
        if((i & 0xF) == 0) {
            fprintf(fp, "\n%4d (%03x). ", i, i);
        }
        fprintf(fp, "[%04X]", stack->data[i]);
    }
    fprintf(fp, "\n\t== STACK END==\n");
}

void dump_registers(FILE* fp, uint8_t* registers) {
    fprintf(fp, "\t== REGISTERS ==");
    for(uint8_t i = 0; i < REGISTER_SIZE; i++) {
        if((i & 0x3) == 0) {
            fputc('\n', stdout);
        }
        fprintf(fp, "V%1X: %4d (%02X)\t", i, registers[i], registers[i]);
    }
    fprintf(fp, "\n\t== REGISTERS END ==\n");
}

void dump_display(FILE *fp, bool display[][WIDTH]) {
    fprintf(fp, "\t== DISPLAY ==\n");
    for(int i = 0; i < HEIGHT; i++) {
        fprintf(fp, "|");
        for(int j = 0; j < WIDTH; j++) {
            fprintf(fp, "%c", display[i][j] ? 'X' : ' ');
        }
        fprintf(fp, "|\n");
    }
    fprintf(fp, "\t== DISPLAY END ==\n");
}

void menu(FILE* fp) {
    fprintf(fp, "CHIP-8 DEBUGGER\n");
    fprintf(fp, "\th: help! (see this menu)\n");
    fprintf(fp, "\tn: fetch, decode, and execute next instruction\n");
    fprintf(fp, "\t1: move time forward 1 step, i.e. update timers and refresh screen\n");
    fprintf(fp, "\ti: see index register contents\n");
    fprintf(fp, "\tp: see program counter contents\n");
    fprintf(fp, "\tm: dump memory\n");
    fprintf(fp, "\td: dump display\n");
    fprintf(fp, "\tr: dump registers\n");
    fprintf(fp, "\ts: dump stack\n");
    fprintf(fp, "\to: see sound timer\n");
    fprintf(fp, "\tt: see delay timer\n");
    fprintf(fp, "\tq: quit\n");
}

void debugger(struct interpreter* interpreter, struct screen* screen) {
    uint16_t instruction = 0x0000;
    for(;;) {
        if(!handle_event()) break;
        fprintf(stdout, ">> ");
        switch(fgetc(stdin)) {
            case 'h':
                menu(stdout);
                break;
            case 'n':
                instruction = fetch(interpreter);
                decode(interpreter, instruction);
                fprintf(stdout, "Performed instruction: %04X\n", instruction);
                break;
            case '1':
                update_internals(interpreter, screen);
                break;
            case 'i':
                fprintf(stdout, "I: %u (%04X)\n", interpreter->index_register, 
                        interpreter->index_register);
                break;
            case 'p':
                fprintf(stdout, "PC: %u (%04X)\n", interpreter->program_counter, 
                        interpreter->program_counter);
                break;
            case 'm':
                dump_memory(stdout, interpreter->memory);
                break;
            case 'd':
                dump_display(stdout, interpreter->display);
                break;
            case 'r':
                dump_registers(stdout, interpreter->registers);
                break;
            case 's':
                dump_stack(stdout, &interpreter->stack);
                break;
            case 'o':
                fprintf(stdout, "Sound timer: %u (%02X)\n", interpreter->sound_timer,
                        interpreter->sound_timer);
                break;
            case 't':
                fprintf(stdout, "Delay timer: %u (%02X)\n", interpreter->delay_timer,
                        interpreter->delay_timer);
                break;
            case 'q':
                return;
        }
        // clear stdin.
        for(char c = fgetc(stdin); c != '\n' && c != EOF; c = fgetc(stdin));
    }
}

