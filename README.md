# Chip-8 Emulator / Interpreter

Henry Jochaniewicz

## TODO

- Write tests for most instructions
- test with pong or tetris
- update the buzzer sound to be more interesting
- have the colors be configurable
- have the frequency be configurable
- Update the README

## Quirks

- For all key instructions from registers, it simply reads the lower nibble
(in terms of endianness). Some online things do not do this.
