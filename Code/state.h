#ifndef STATE_H
#define STATE_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    STATE_DISARMED = 0,
    STATE_ARMED
} ctrl_state_t;

void update_state(int32_t error);

#endif