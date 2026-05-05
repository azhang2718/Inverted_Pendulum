#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "state.h"

void control_step(uint16_t angle);

extern volatile uint32_t g_step_hz;
extern volatile int8_t   g_motor_dir;
extern volatile ctrl_state_t g_state;
extern volatile bool g_limit_tripped;
extern volatile int32_t g_integral;
extern volatile int32_t g_error_filt;
extern volatile int32_t g_error;

extern int32_t abs32(int32_t x);

#endif