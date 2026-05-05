#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

void motor_command(int8_t dir, uint32_t step_hz);
void motor_stop(void);

#endif