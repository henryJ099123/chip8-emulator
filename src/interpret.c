/**
 * Author: Henry Jochaniewicz
 * Date modified: 01/01/26
 **/
#include <stdbool.h>
#include <stdlib.h>

#include "settings.h"
#include "memory.h"
#include "interpret.h"
#include "display.h"

#define NIBBLE_1_BYTE(byte) (((byte) >> 4) & 0x0F)
#define NIBBLE_2_BYTE(byte) ((byte) & 0x0F)

#define NIBBLE_1(two_bytes) (((two_bytes) >> 12) & 0x000F)
#define NIBBLE_2(two_bytes) (((two_bytes) >> 8)  & 0x000F)
#define NIBBLE_3(two_bytes) (((two_bytes) >> 4)  & 0x000F)
#define NIBBLE_4(two_bytes) ((two_bytes) & 0x000F)
#define BYTE_1(two_bytes)   (((two_bytes) >> 8)  & 0x00FF)
#define BYTE_2(two_bytes)   ((two_bytes) & 0x00FF)
#define AFTER_NIBBLE_1(two_bytes) \
    ((two_bytes) & 0x0FFF)

#define GET_BIT(bytes, n)    (((bytes) >> (n)) & 0x01)
#define SET_BIT(bytes, n)    ((0x01 << (n)) | (bytes))
#define CLEAR_BIT(bytes, n)  (~(0x01 << (n)) & (bytes))
#define TOGGLE_BIT(bytes, n) ((0x01 << (n)) ^ (bytes))

uint16_t fetch(struct interpreter* interpreter) {
    uint8_t b1 = interpreter->memory[interpreter->program_counter++];
    uint8_t b2 = interpreter->memory[interpreter->program_counter++];
    return (b1 << 8) | b2;
}

void update_internals(struct interpreter* interpreter, struct screen* screen) {
    if(interpreter->delay_timer != 0) interpreter->delay_timer--;
    if(interpreter->sound_timer != 0) interpreter->sound_timer--;
    draw_display(screen->renderer, interpreter->display);
    play_sound(screen->stream, interpreter->sound_timer);
}

// returns the value that VF register should be set to.
static uint8_t draw_sprite(struct interpreter* interpreter, uint8_t x, uint8_t y, uint8_t h) {
    uint8_t min_height = h < HEIGHT - y ? h : HEIGHT - y;
    uint8_t* sprite_start = interpreter->memory + interpreter->index_register;
    int set_vf_value = 0;
    for(int j = 0; j < min_height; j++) {
        for(int i = 0; i < 8; i++) {
            if(x + i > WIDTH) break;
            // we need bits from most to least significant, therefore 7 - i.
            // the GET_BITS macro gets them from least to most significant
            if(GET_BIT(sprite_start[j], 7 - i) == 0) continue;
            if(interpreter->display[j + y][x + i]) set_vf_value = 1;
            interpreter->display[j + y][x + i] = !interpreter->display[j + y][x + i];
        }
    }
    return set_vf_value;
}

void decode(struct interpreter* interpreter, uint16_t instruction) {
    switch(NIBBLE_1(instruction)) {
        case 0x0:
            // 0x00E0: clear screen.
            if(AFTER_NIBBLE_1(instruction) == 0x0E0) {
                clear_display(interpreter->display);
            // 0x00EE: Return, i.e. PC <- STACK_POP
            } else if(AFTER_NIBBLE_1(instruction) == 0x0EE) {
                interpreter->program_counter = STACK_POP(&interpreter->stack);
            }
            break;
        // 0x1NNN: jump, i.e. PC <- NNN
        case 0x1:
            interpreter->program_counter = AFTER_NIBBLE_1(instruction);
            break;
        // 0x2NNN: subroutine / call, i.e. STACK_PUSH(PC), PC <- NNN
        case 0x2:
            STACK_PUSH(&interpreter->stack, interpreter->program_counter);
            interpreter->program_counter = AFTER_NIBBLE_1(instruction);
            break;
        // 0x3XNN: skip if equal, i.e. if(VX == NN) PC+=2
        case 0x3:
            if(interpreter->registers[NIBBLE_2(instruction)] == BYTE_2(instruction)) {
                interpreter->program_counter += 2; // skip next instruction.
            }
            break;
        // 0x3XNN: skip if not equal, i.e. if(VX != NN) PC+=2
        case 0x4:
            if(interpreter->registers[NIBBLE_2(instruction)] != BYTE_2(instruction)) {
                interpreter->program_counter += 2; // skip next instruction.
            }
            break;
        // 0x5XY0: skip if equal (registers), i.e. if(VX == VY) PC+=2
        case 0x5:
            if(interpreter->registers[NIBBLE_2(instruction)] ==
                    interpreter->registers[NIBBLE_3(instruction)]) {
                interpreter->program_counter += 2; // skip next instruction.
            }
            break;
        // 0x6XNN: assignment with immediate, i.e. V6 <- NN
        case 0x6:
            interpreter->registers[NIBBLE_2(instruction)] = BYTE_2(instruction);
            break;
        // 0x7XNN: addition with immediate, i.e. VX <- VX + NN
        case 0x7:
            interpreter->registers[NIBBLE_2(instruction)] += BYTE_2(instruction);
            break;
        // various arithmetic between registers.
        case 0x8: {
            switch(NIBBLE_4(instruction)) {
                // 0x8XY0: assignment between registers, i.e. VX <- VY
                case 0x0:
                    interpreter->registers[NIBBLE_2(instruction)] =
                        interpreter->registers[NIBBLE_3(instruction)];
                    break;
                // 0x8XY1: bitwise or, i.e. VX <- VX | VY
                case 0x1:
                    interpreter->registers[NIBBLE_2(instruction)] |=
                        interpreter->registers[NIBBLE_3(instruction)];
                    break;
                // 0x8XY2: bitwise and, i.e. VX <- VX & VY
                case 0x2:
                    interpreter->registers[NIBBLE_2(instruction)] &=
                        interpreter->registers[NIBBLE_3(instruction)];
                    break;
                // 0x8XY3: bitwise xor, i.e. VX <- VX ^ VY
                case 0x3:
                    interpreter->registers[NIBBLE_2(instruction)] ^=
                        interpreter->registers[NIBBLE_3(instruction)];
                    break;
                // 0x8XY4: addition, i.e. VX <- VX + VY, VF = carry
                case 0x4: {
                    uint8_t first = interpreter->registers[NIBBLE_2(instruction)];
                    uint8_t second = interpreter->registers[NIBBLE_3(instruction)];
                    interpreter->registers[NIBBLE_2(instruction)] += second;
                    interpreter->registers[0xF] = first > UINT8_MAX - second;
                    break;
                }
                // 0x8XY5: subtraction, i.e. VX <- VX - VY, VF = no borrow
                case 0x5: {
                    uint8_t first = interpreter->registers[NIBBLE_2(instruction)];
                    uint8_t second = interpreter->registers[NIBBLE_3(instruction)];
                    interpreter->registers[NIBBLE_2(instruction)] = first - second;
                    interpreter->registers[0xF] = first > second;
                    break;
                }
                // 0x8XY6 shift right: An ambiguous instruction.
                // the ambiguous bit: VX <- VY
                // The same: VX -> VX >> 1
                // VF is set to the shifted out bit.
                case 0x6: {
#ifdef SHIFT_OPTION
                    interpreter->registers[NIBBLE_2(instruction)] =
                        interpreter->registers[NIBBLE_3(instruction)];
#endif
                    uint8_t bit = GET_BIT(interpreter->registers[
                            NIBBLE_2(instruction)], 0);
                    interpreter->registers[NIBBLE_2(instruction)] >>= 1;
                    interpreter->registers[0xF] = bit;
                    break;
                }
                // 0x8XY7: subtraction reverse, i.e. VX <- VY - VX, VF = no borrow
                case 0x7: {
                    uint8_t first = interpreter->registers[NIBBLE_2(instruction)];
                    uint8_t second = interpreter->registers[NIBBLE_3(instruction)];
                    interpreter->registers[NIBBLE_2(instruction)] = second - first;
                    interpreter->registers[0xF] = second > first;
                    break;
                }
                // 0x8XY6 shift left: An ambiguous instruction.
                // the ambiguous bit: VX <- VY
                // The same: VX -> VX << 1
                // VF is set to the shifted out bit.
                case 0xE: {
#ifdef SHIFT_OPTION
                    interpreter->registers[NIBBLE_2(instruction)] =
                        interpreter->registers[NIBBLE_3(instruction)];
#endif
                    uint8_t bit = GET_BIT(interpreter->registers[
                            NIBBLE_2(instruction)], 0);
                    interpreter->registers[NIBBLE_2(instruction)] <<= 1;
                    interpreter->registers[0xF] = bit;
                    break;
                }
                default:
                    fprintf(stderr, "Unknown instruction %4X.\n", instruction);
                    break;
            }
            break;
        }
        // 0x9XY0: skip if not equal (registers), i.e. if(VX != VY) PC+=2
        case 0x9:
            if(interpreter->registers[NIBBLE_2(instruction)] != 
                    interpreter->registers[NIBBLE_3(instruction)]) {
                interpreter->program_counter += 2; // skip next instruction.
            }
            break;
        // 0xANNN: assignment of index register, i.e. I <- NNN
        case 0xA:
            interpreter->index_register = AFTER_NIBBLE_1(instruction);
            break;
        // 0xBXNN jump with offset: ambiguous instruction.
        // Either PC <- V0 + XNN, or
        // PC <- VX + XNN. This is silly. e.g. B220 will set PC <- V2 + 220.
        case 0xB:
#ifdef JUMP_OFFSET_OPTION
            interpreter->program_counter = interpreter->registers[NIBBLE_2(instruction)] + AFTER_NIBBLE_1(instruction);
#else
            interpreter->program_counter = interpreter->registers[0x0] + AFTER_NIBBLE_1(instruction);
#endif
            break;
        // 0xCXNN: random, i.e. VX <- rand[0, 255] & NN
        case 0xC:
            interpreter->registers[NIBBLE_2(instruction)] = 
                rand() & BYTE_2(instruction); 
            break;
        // 0xDXYN: display an N-byte sprite starting at M[I] at position (VX, VY).
        // This display is an XOR with the existing bit of the screen.
        // VF = collision, i.e. whether a pixel is erased / st off..
        // Only wraps around if the WHOLE sprite is off-screen.
        case 0xD: {
            uint8_t x = interpreter->registers[NIBBLE_2(instruction)] & (WIDTH - 1);
            uint8_t y = interpreter->registers[NIBBLE_3(instruction)] & (HEIGHT - 1);
            uint8_t height = NIBBLE_4(instruction);

            interpreter->registers[0xF] = draw_sprite(interpreter, x, y, height);
            break;
        }
        // key: skip next instruction if key in V[N2] is being pressed, i.e. poll for input.
        case 0xE:
            // 0xEX9E: skip if key pressed, i.e. if(key_pressed(VX)) PC+=2
            if(BYTE_2(instruction) == 0x9E &&
                    is_key_pressed(NIBBLE_2_BYTE(
                    interpreter->registers[NIBBLE_2(instruction)]))) {
                interpreter->program_counter += 2;
            // 0xEX9E: skip if not key pressed, i.e. if(!key_pressed(VX)) PC+=2
            } else if(BYTE_2(instruction) == 0xA1 &&
                    !is_key_pressed(NIBBLE_2_BYTE(
                    interpreter->registers[NIBBLE_2(instruction)]))) {
                interpreter->program_counter += 2;
            } else if(BYTE_2(instruction) != 0x9E && BYTE_2(instruction) != 0xA1) {
                fprintf(stderr, "Unknown instruction %4X.\n", instruction);
            }
            break;
        /**
         * wildcards.
         * FX07: set VX <- delay timer
         * FX0A: blocking instruction that waits for any key, whose value is put into VX.
         *       note that this does not stop execution entirely. timers still decrease.
         * FX15: delay timer <- VX 
         * FX18: sound timer <- VX 
         * FX1E: I <- I + VX, where it is ambiguous if VF is set on overflow.
         * FX29: font character: I <- address of character VX in memory
         * FX33: Binary-coded decimal conversion.
         *       Takes number in VX (one byte) and converts it to three decimal digits,
         *       stores these at M[I], M[I+1], and M[I+2] for 100s, 10s, 1s respectively.
         * FX55: store registers subsequently to memory, where M[I + i] <- Vi.
         * FX65: load registers, where Vi <- M[I + i].
         *       For these instructions, it is ambiguous whether I is incremented or not.
         *       Modern implementations do NOT increment I.
         **/
        case 0xF: {
            switch(BYTE_2(instruction)) {
                case 0x07:
                    interpreter->registers[NIBBLE_2(instruction)] = interpreter->delay_timer;
                    break;
                case 0x0A: {
                    uint8_t response;
                    if((response = any_key_pressed()) == 0xFF) {
                        interpreter->program_counter -= 2;
                    } else {
                        interpreter->registers[NIBBLE_2(instruction)] = response;
                    }
                    break;
                }
                case 0x15:
                    interpreter->delay_timer = interpreter->registers[NIBBLE_2(instruction)];
                    break;
                case 0x18:
                    interpreter->sound_timer = interpreter->registers[NIBBLE_2(instruction)];
                    break;
                case 0x1E:
                    interpreter->index_register += interpreter->registers[NIBBLE_2(instruction)];
#ifdef INDEX_ADD_OBB_OPTION
                    interpreter->registers[0xF] = interpreter->index_register > 0xFFF; 
#endif
                    break;
                // this looks at the lower nibble of VX for the character.
                // All character fonts take up 5 bytes of memory.
                case 0x29: {
                    uint8_t character = NIBBLE_2_BYTE(
                            interpreter->registers[NIBBLE_2(instruction)]);
                    interpreter->index_register = FONT_START_ADDRESS + character * 5;
                    break;
                }
                // e.g. if VX stores 123, M[I] <- 1, M[I+1] <- 2, M[I+2] <- 3
                case 0x33: {
                    uint8_t digits = interpreter->registers[NIBBLE_2(instruction)];
                    interpreter->memory[interpreter->index_register + 2] = digits % 10;
                    digits /= 10;
                    interpreter->memory[interpreter->index_register + 1] = digits % 10;
                    digits /= 10;
                    interpreter->memory[interpreter->index_register] = digits;
                    break;
                }
                case 0x55: {
                    for(uint8_t i = 0; i <= NIBBLE_2(instruction); i++) {
                        interpreter->memory[interpreter->index_register + i] =
                            interpreter->registers[i];
                    }
#ifdef LOAD_STORE_MODIFY_INDEX_OPTION 
                    interpreter->index_register += NIBBLE_2(instruction);
#endif
                    break;
                }
                case 0x65:
                    for(uint8_t i = 0; i <= NIBBLE_2(instruction); i++) {
                        interpreter->registers[i] =
                            interpreter->memory[interpreter->index_register + i];
                    }
#ifdef LOAD_STORE_MODIFY_INDEX_OPTION 
                    interpreter->index_register += REGISTER_SIZE;
#endif
                    break;
                default:
                    fprintf(stderr, "Unknown instruction %4X.\n", instruction);
                    break;
            }
            break;
        }
        default:
            fprintf(stderr, "Unknown instruction %4X.\n", instruction);
            break;
    }
}

