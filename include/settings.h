/**
 * Author: Henry Jochaniewicz
 * Date modified: December 31, 2025
 **/
#ifndef __SETTINGS_H__
#define __SETTINGS_H__

/* Configurables parameters. */
#define TIMER_FREQUENCY 60 // in Hz.
#define FREQUENCY 500 // in Hz. Modifiable.
#define CYCLE_TIME (1.0 / (FREQUENCY))
#define TIMER_CYCLE_TIME (1.0 / (TIMER_FREQUENCY))
#define WIDTH 64
#define HEIGHT 32

/* Configurable settings. */
#define SHIFT_OPTION
#define JUMP_OFFSET_OPTION
#undef  INDEX_ADD_OPTION
#undef  INDEX_INC_MEMORY_OPTION

#endif
