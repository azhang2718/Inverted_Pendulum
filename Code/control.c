#include "control.h"
#include "state.h"

// =====================================================================
// Telemetry
// =====================================================================
volatile uint16_t     g_raw_angle              = 0;
volatile int32_t      g_error                  = 0;
volatile int32_t      g_error_filt             = 0;
volatile int32_t      g_derivative             = 0;
volatile int32_t      g_integral               = 0;
volatile int32_t      g_pid_output             = 0;
volatile uint32_t     g_step_hz                = 0;
volatile int8_t       g_motor_dir              = 0;
volatile uint32_t     g_loop_count             = 0;
volatile uint8_t      g_i2c_err                = 0;

// =====================================================================
// Internals
// =====================================================================
static int32_t signed_dist(uint16_t a, uint16_t b)
{
    int32_t d = (int32_t)a - (int32_t)b;
    if (d >  COUNTS_PER_REV / 2) d -= COUNTS_PER_REV;
    if (d < -COUNTS_PER_REV / 2) d += COUNTS_PER_REV;
    return d;
}

int32_t abs32(int32_t x) { return x < 0 ? -x : x; }

// =====================================================================
// Control step (simplified hook)
// =====================================================================
void control_step(uint16_t angle)
{
    g_raw_angle = angle;

    g_error = signed_dist(angle, ANGLE_REFERENCE_COUNTS);

    // (You can move full PID logic here later)
}