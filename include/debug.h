#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "settings.h"
#include "interpret.h"
#include "memory.h"
#include "display.h"

void dump_memory(FILE* fp, uint8_t* memory);
void dump_registers(FILE* fp, uint8_t* registers);
void dump_stack(FILE* fp, struct stack* stack);
void dump_display(FILE* fp, bool display[][WIDTH]);
void debugger(struct interpreter* interpreter, struct screen* screen);

#endif

