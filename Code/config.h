#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

// === Hardware constants ===
#define AS5600_ADDR             0x36
#define AS5600_RAW_ANGLE_H      0x0C
#define COUNTS_PER_REV          4096
#define TIMER_CLK_HZ            1000000UL
#define TICK_MS                 5

// === Calibration ===
#define ANGLE_REFERENCE_COUNTS  44

// === Step rate window ===
#define MAX_STEP_HZ             2500U
#define MIN_STEP_HZ             400U
#define DEADBAND_COUNTS         5       

// === PID gains ===
#define KP_x100                 800
#define KI_x100                 0
#define KD_x100                 2000

// === Angle bands ===
#define ENGAGE_THRESHOLD_COUNTS 120
#define GIVE_UP_THRESHOLD_COUNTS 600
#define INTEGRAL_LIMIT          50000

// === Direction sign ===
#define MOTOR_DIRECTION_SIGN    (-1)


#endif