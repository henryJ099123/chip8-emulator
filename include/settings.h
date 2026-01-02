/**
 * Author: Henry Jochaniewicz
 * Date modified: 01/01/26
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
#undef  INDEX_ADD_OOB_OPTION
#undef  LOAD_STORE_MODIFY_INDEX_OPTION

#endif
