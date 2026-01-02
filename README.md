# Chip-8 Emulator

Henry Jochaniewicz

[!Title](title.png)

## Description

This is a CHIP-8 emulator written in C and using SDL3.
It follows the major specifications found in the [resources](#Resources) section.
In a few places I found conflicting specification data, so anything abnormal
is listed in the [quirks](#Quirks) section.

The CHIP-8 emulator includes a debugging mode. See the [run instructions](#Run-Instructions)
section on how to activate this, and the [debugger](#Debugger) section
on features.

I have also included a host of tests in the `tests/` directory. Nearly all of them
are not unique to my implementation, so feel free to use them if developing
your own emulator. See `tests/README.md` for more information.

### Resources

I used the following resources to learn about how the CHIP-8 specifications:

1. [Guide to making a CHIP-8 emulator](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/#stack)
by Tobias V. Langhoff
2. Wikipedia's [CHIP-8](https://en.wikipedia.org/wiki/CHIP-8) site
3. [Cowgod's Chip-8 Technical Reference v1.0](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#0nnn)

## Build Instructions

The Makefile included is for MacOS architecture. You will likely need to modify
the `IFLAGS` or `LFLAGS` variables for your own compilation that point
to the location of the `SDL3` library.

Speaking of which, this requires SDL3, which you can download [here](https://github.com/libsdl-org/SDL/releases/tag/release-3.4.0).
If you have homebrew installed, simply do the following:

```sh
brew install sdl3
```

When this works, simply run:

```sh
make
```

There will now be a `chip8` executable in the home directory, which is the emulator!

## Run Instructions

Usage:

```sh
./chip8 [-g] <romname.rom>
```

For a given rom `test.rom`, if in the home directory:

```sh
./chip8 test.rom
```

This will run the emulator for the given rom.
To run in debug mode, add the `-g` flag *before the rom*:

```sh
./chip8 -g test.rom
```

When running a ROM, hit the **escape key** to end the emulation.

To make sure this works, I recommend downloading an IBM Logo ROM, which for
legal reasons is not included here.

## Debugger

The emulator comes equipped with a debug mode. See
the [running instructions](#Run-Instructions) on how to activate it.

The debugger will start up the program, but not run any instructions.
Here are the options:
- `h`: see help menu
- `n`: execute next instruction, and print out the instruction to the console
- `1`: move time forward 1Hz, i.e. update timers and refresh screen
- `i`: see index register contents
- `p`: see program counter contents
- `m`: dump memory
- `d`: dump display data
- `r`: dump registers
- `s`: dump stack
- `o`: see sound timer contents
- `t`: see delay timer contents
- `q`: quit

## Quirks

Listed here are any quirks of the implementation.

- For all key instructions from registers, it simply reads the lower nibble
(in terms of endianness). Some online things do not do this.
- For the instruction `FX0A`, which blocks and gets a key, `VX` will be
stored with the key being pressed down. If multiple keys are being
pressed down, `VX` will store the least key in terms of value.
- If the emulator tries to run an instruction not recognized, it will
print the instruction to the console with the message `"Unknown instruction
0xNNNN."`.

## TODO

- add breakpoints to the debugger
- perhaps write a CHIP-8 assembler (and thus write an assembly language)
- move the display data to memory instead of a separate double array of booleans

