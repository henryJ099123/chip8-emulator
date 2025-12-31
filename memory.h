/**
 * Author: Henry Jochaniewicz
 * Date modified: December 30, 2025
 **/
#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>
#include <unistd.h>

#define MEMORY_SIZE 4096
#define STACK_SIZE 1024
#define REGISTER_SIZE 16
#define START_ADDRESS 0x200
#define FONT_START_ADDRESS 0x50

#define STACK_PUSH(stack, value) \
    ((stack)->data[(stack)->pointer++] = (value))
#define STACK_POP(stack) \
    ((stack)->data[--(stack)->pointer])

struct stack {
    uint16_t pointer;
    uint16_t data[STACK_SIZE];
};

void initialize_font(uint8_t* memory);
ssize_t load_code(uint8_t* memory, int fd);

#endif
