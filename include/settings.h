/**
 * Author: Henry Jochaniewicz
 * Date modified: 01/02/26
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
#define OFF_COLOR 0x480000
#define ON_COLOR  0xE86A43
#define SOUND_FREQUENCY 440

/* Configurable settings. */
#undef  SHIFT_OPTION
#undef  JUMP_OFFSET_OPTION
#undef  INDEX_ADD_OOB_OPTION
#undef  LOAD_STORE_MODIFY_INDEX_OPTION

#endif

