/**
 * Author: Henry Jochaniewicz
 * Date modified: December 30, 2025
 **/
#ifndef __INTERPRET_H__
#define __INTERPRET_H__

#include <stdbool.h>

#include "settings.h"
#include "memory.h"
#include "display.h"

struct interpreter {
    uint8_t memory[MEMORY_SIZE];
    bool display[HEIGHT][WIDTH];
    struct stack stack;
    uint8_t registers[REGISTER_SIZE];

    uint16_t program_counter;
    uint16_t index_register;
    uint8_t delay_timer;
    uint8_t sound_timer;
};

uint16_t fetch(struct interpreter* interpreter);
bool decode(struct interpreter* interpreter, uint16_t instruction);
void update_timers(struct interpreter* interpreter, struct screen* screen);

#endif
