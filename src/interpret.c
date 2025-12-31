/**
 * Author: Henry Jochaniewicz
 * Date modified: December 30, 2025
 **/
#include <stdbool.h>
#include <stdlib.h>

#include "memory.h"
#include "interpret.h"
#include "display.h"

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


// returns the value that VF register should be set to.
int draw_sprite(struct interpreter* interpreter, uint8_t x, uint8_t y, uint8_t h) {
    int min_height = h < HEIGHT - y ? h : HEIGHT - y;

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

// returns whether or not a screen refresh should occur.
bool decode(struct interpreter* interpreter, uint16_t instruction) {
    bool refresh = false;
    switch(NIBBLE_1(instruction)) {
        // clear screen or return from subroutine.
        case 0x0:
            if(AFTER_NIBBLE_1(instruction) == 0x0E0) {
                clear_display(interpreter->display);

            } else if(AFTER_NIBBLE_1(instruction) == 0x0EE) {
                interpreter->program_counter = STACK_POP(&interpreter->stack);
            }
            break;
        // jump.
        case 0x1:
            interpreter->program_counter = AFTER_NIBBLE_1(instruction);
            break;
        // subroutine / call.
        case 0x2:
            STACK_PUSH(&interpreter->stack, interpreter->program_counter);
            interpreter->program_counter = AFTER_NIBBLE_1(instruction);
            break;
        // conditional jump, eq. to immediate
        case 0x3:
            if(interpreter->registers[NIBBLE_2(instruction)] == BYTE_2(instruction)) {
                interpreter->program_counter += 2; // skip next instruction.
            }
            break;
        // conditional jump, neq. to immediate
        case 0x4:
            if(interpreter->registers[NIBBLE_2(instruction)] != BYTE_2(instruction)) {
                interpreter->program_counter += 2; // skip next instruction.
            }
            break;
        // conditional jump, eq. to register 
        case 0x5:
            if(interpreter->registers[NIBBLE_2(instruction)] ==
                    interpreter->registers[NIBBLE_3(instruction)]) {
                interpreter->program_counter += 2; // skip next instruction.
            }
            break;
        // V[N2] <- N3:N4
        case 0x6:
            interpreter->registers[NIBBLE_2(instruction)] = BYTE_2(instruction);
            break;
        // V[N2] <- V[N2] + N3:N4
        case 0x7:
            interpreter->registers[NIBBLE_2(instruction)] += BYTE_2(instruction);
            break;
        // various arithmetic, decided by the last nibble of opcode.
        case 0x8: {
            switch(NIBBLE_4(instruction)) {
                // Assignment: V[N3] <- V[N4]
                case 0x0:
                    interpreter->registers[NIBBLE_2(instruction)] =
                        interpreter->registers[NIBBLE_3(instruction)];
                    break;
                // Or: V[N3] <- V[N3] | V[N4]
                case 0x1:
                    interpreter->registers[NIBBLE_2(instruction)] |=
                        interpreter->registers[NIBBLE_3(instruction)];
                    break;
                // And: V[N3] <- V[N3] & V[N4]
                case 0x2:
                    interpreter->registers[NIBBLE_2(instruction)] &=
                        interpreter->registers[NIBBLE_3(instruction)];
                    break;
                // Xor: V[N3] <- V[N3] ^ V[N4]
                case 0x3:
                    interpreter->registers[NIBBLE_2(instruction)] ^=
                        interpreter->registers[NIBBLE_3(instruction)];
                    break;
                // Add: V[N3] <- V[N3] + V[N4]. Overflow sets VF to 1, else 0.
                case 0x4: {
                    uint8_t first = interpreter->registers[NIBBLE_2(instruction)];
                    uint8_t second = interpreter->registers[NIBBLE_3(instruction)];
                    interpreter->registers[0xF] = first > UINT8_MAX - second ? 1 : 0;
                    interpreter->registers[NIBBLE_3(instruction)] += second;
                    break;
                }
                // Add: V[N3] <- V[N3] + V[N4]. Underflow sets VF to 0, else 1.
                case 0x5: {
                    uint8_t first = interpreter->registers[NIBBLE_2(instruction)];
                    uint8_t second = interpreter->registers[NIBBLE_3(instruction)];
                    interpreter->registers[0xF] = first > second ? 1 : 0;
                    interpreter->registers[NIBBLE_3(instruction)] = first - second;
                    break;
                }
                // Shift: An ambiguous instruction.
                // Optional Step: V[N3] <- V[N4]
                // The same: V[N3] -> V[N3] >> 1
                // VF is set to the shifted out bit.
                case 0x6: {
#ifdef SHIFT_OPTION
                    interpreter->registers[NIBBLE_2(instruction)] =
                        interpreter->registers[NIBBLE_3(instruction)];
#endif
                    interpreter->registers[0xF] = GET_BIT(
                            interpreter->registers[NIBBLE_2(instruction)], 0);
                    interpreter->registers[NIBBLE_2(instruction)] >>= 1;
                    break;
                }
                // Shift: An ambiguous instruction.
                // Optional Step: V[N3] <- V[N4]
                // The same: V[N3] -> V[N3] << 1
                // VF is set to the shifted out bit.
                case 0xE: {
#ifdef SHIFT_OPTION
                    interpreter->registers[NIBBLE_2(instruction)] =
                        interpreter->registers[NIBBLE_3(instruction)];
#endif
                    interpreter->registers[0xF] = GET_BIT(
                            interpreter->registers[NIBBLE_2(instruction)], 7);
                    interpreter->registers[NIBBLE_2(instruction)] <<= 1;
                    break;
                }
                default:
                    fprintf(stderr, "Unknown instruction %4X.\n", instruction);
                    break;
            }
        }
        // conditional jump, neq. to register 
        case 0x9:
            if(interpreter->registers[NIBBLE_2(instruction)] != 
                    interpreter->registers[NIBBLE_3(instruction)]) {
                interpreter->program_counter += 2; // skip next instruction.
            }
            break;
        // Set index: I <- N2:N4
        case 0xA:
            interpreter->index_register = AFTER_NIBBLE_1(instruction);
            break;
        // Jump with offset: Ambiguous instruction.
        // Either PC <- V0 + N2:N4, or
        // PC <- V[N2] + N2:N4. This is silly. e.g. B220 will set PC <- V2 + 220.
        case 0xB:
#ifdef JUMP_OFFSET_OPTION
            interpreter->program_counter = interpreter->registers[0x0] + AFTER_NIBBLE_1(instruction);
#else
            interpreter->program_counter = interpreter->registers[NIBBLE_2(instruction)] + AFTER_NIBBLE_1(instruction);
#endif
            break;
        // random: V[N2] <- random_number & N3:N4
        case 0xC:
            interpreter->registers[NIBBLE_2(instruction)] = rand() & BYTE_2(instruction); 
        case 0xD: {
            uint8_t x = interpreter->registers[NIBBLE_2(instruction)] & (WIDTH - 1);
            uint8_t y = interpreter->registers[NIBBLE_3(instruction)] & (HEIGHT - 1);
            uint8_t height = NIBBLE_4(instruction);

            interpreter->registers[0xF] = draw_sprite(interpreter, x, y, height);
            refresh = true;
            break;
        }
        // key: skip next instruction if key in V[N2] is being pressed, i.e. poll for input.
        case 0xE:
            if(BYTE_2(instruction) == 0x9E &&
                    is_key_pressed(interpreter->registers[NIBBLE_2(instruction)])) {
                interpreter->program_counter += 2;
            } else if(BYTE_2(instruction) == 0xA1 &&
                    !is_key_pressed(interpreter->registers[NIBBLE_2(instruction)])) {
                interpreter->program_counter += 2;
            } else {
                fprintf(stderr, "Unknown instruction %4X.\n", instruction);
            }
            break;
        /**
         * wildcards.
         * FX07: set VX <- delay timer
         * FX15: delay timer <- VX 
         * FX18: sound timer <- VX 
         * FX1E: I <- I + VX, where it is ambiguous if VF is set on overflow.
         * FX0A: blocking instruction that waits for any key, whose value is put into VX.
         *       note that this does not stop execution entirely. timers still decrease.
         * FX29: font character: I <- address of hexadecimal character in VX
         * FX33: Binary-coded decimal conversion.
         *       Takes number in VX (one byte) and converts it to three decimal digits,
         *       stores these at M[I], M[I+1], and M[I+2].
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
                case 0x15:
                    interpreter->delay_timer = interpreter->registers[NIBBLE_2(instruction)];
                    break;
                case 0x18:
                    interpreter->sound_timer = interpreter->registers[NIBBLE_2(instruction)];
                    break;
                case 0x1E:
#ifdef INDEX_ADD_OPTION
                    interpreter->registers[0xF] = interpreter->index_register > 
                        UINT8_MAX - interpreter->registers[NIBBLE_2(instruction)] ?
                        0 : 1;
#endif
                    interpreter->index_register += interpreter->registers[NIBBLE_2(instruction)];
                    break;
                case 0x0A:
                    if(!any_key_pressed()) {
                        interpreter->program_counter -= 2;
                    }
                    break;
                // this looks at the lower nibble of VX for the character.
                case 0x29: {
                    uint8_t character = NIBBLE_2(interpreter->registers[NIBBLE_2(instruction)]);
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
                    for(int i = 0; i < REGISTER_SIZE; i++) {
                        interpreter->memory[interpreter->index_register + i] =
                            interpreter->registers[i];
                    }
#ifdef INDEX_INC_MEMORY_OPTION
                    interpreter->index_register += REGISTER_SIZE;
#endif
                    break;
                }
                case 0x65:
                    for(int i = 0; i < REGISTER_SIZE; i++) {
                        interpreter->registers[i] =
                            interpreter->memory[interpreter->index_register + i];
                    }
#ifdef INDEX_INC_MEMORY_OPTION
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

    return refresh;
}
