#include <stdint.h>
#include <stdio.h>

#include "debug.h"
#include "memory.h"
#include "display.h"

void dump_memory(FILE* fp, uint8_t* memory) {
    fprintf(fp, "\t== MEMORY ==");
    for(uint32_t i = 0; i < MEMORY_SIZE; i++) {
        if((i & 0xF) == 0) {
            fprintf(fp, "\n%4d (%03x).", i, i);
        }

        fprintf(fp, " %02X", memory[i]);
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

