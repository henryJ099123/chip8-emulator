# Writing a CHIP8 Emulator Guide

[Guide here](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/)

## History

This is really a mini interpreter. Designed by Joe Weisbecker in 1977
for the COSMAC VIP microcomputer.

Type in hexadecimal instructions with a keypad that resembled machine code, but
was really higher level, and interpreted on the fly by a small program, the CHIP-8 interpreter.

## Specifications

- memory: 4KB RAM
- display: 64 by 32 pixels monochrome
- program counter
- one 16 bit index register to point at memory locations
- stack for 16 bit addresses
- 8 bit delay timer which decrements at 60Hz until it hits 0
- 8 bit sound timer that functions like the delay timer but gives off a beeping sound
as long as it is not 0
- 16 8-bit general purpose variable registers, V0 through VF
    - VF used as a flag register, i.e. most instructions will set it to 0 or 1

## Memory

- 4 KB memory; the index register can only address 12 bits
    - the PC and index register are actually 16 bits
- all memory writeable
- CHIP-8 games modify themselves
- the first CHIP-8 interpreter was located in RAM from 000 to 1FF and expects
the CHIP8 program to be loaded at address 0x200
    - do same for my interpreter

## Font

- have a built in font with sprite data representing hexadecimal numbers 0 to F
- each font character is 4 pixels wide by 5 pixels tall
    - drawn just like regular sprites
- store font data in memory because games draw characters like regular sprites
    - must be in the first 512 bytes

## Display

- only 64 pixels by 32 pixels, but can only be on or off
- recommended to only redraw screen when emulator executes an instruction that modifies
the display data
- `DXYN` instruction draws a sprite on the screen, consisting of 8-bits, where each bit
corresponds to a horizontal pixel
    - 16 bytes
    - 0s considered transparent and 1s flip the pixels in location of screen drawn to
- inevitably causes flickering

## Stack

- stack data structure where you can push or pop data
- can represent it on the language's stack or as an array
- basically for function calls
- can just use a variable outside of the emulated memory
- can limit this, as the original interpreters only had 16 two-byte entries

## Timers

- delay timer and sound timer that work the same way
- one byte in size
- decremented by 1 60 times a second until 0
- independent of the fetch/decode/execute loop
- sound timer should cause a beep while above 0
- interpreter should run as normal while the timers are being decremented;
the game will check their values and delay itself if it wants

## Keypad

- earliest computers for CHIP8 had hexadecimal keypads, 0-F,
with the layout
```text
1 2 3 C
4 5 6 D
7 8 9 E
A 0 b F
```
which is common now to use the leftside of the qwerty keyboard for:
```text
1 2 3 4
q w e r
a s d f
z x c v
```
- recommended to use keyboard scancodes rather than actual letters so people
with different keyboard layouts can use the same emulator

## the loop

- emulator of a CPU performs fetch, decode, and execute repeatedly
- the CHIP-8 computers ran at 1 MHz, and the 90s ones at 4 MHz
    - let this be a configurable constant
- fetch just like exepcted, but instructions are 16 bits
    - recommends moving the PC forward here
- decode very simple
    - first nibble of the instruction is the opcode (broad categories)
    - use a switch statement
    - X: the second nibble (register lookup)
    - Y: the third nibble (register lookup)
    - N: the fourth nibble (a number)
    - NN: the second byte (8 bit immediate)
    - NNN: last 3 nibbles (12 bit immediate memory address)
    - recommends extracting these before decoding
- execute should basically be done when decoding

##  Instructions

- can read this when actually implementing them
- recommends writing a small subset of instructions first to test with IBM logo program

## Troubleshooting

- add rudimentary debugging capabilities
