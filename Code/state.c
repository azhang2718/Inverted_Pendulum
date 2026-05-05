#include "state.h"
#include "ti_msp_dl_config.h"
#include "config.h" 
#include "control.h"

volatile ctrl_state_t  g_state                  = STATE_DISARMED;
volatile bool          g_limit_tripped          = false; // HARD STOP FLAG



// =====================================================================
// State machine
// =====================================================================
void update_state(int32_t error)
{
    if (g_limit_tripped) {
        g_state = STATE_DISARMED;
        return;
    }

    int32_t mag = abs32(error);
    switch (g_state) {
        case STATE_DISARMED:
            if (mag <= ENGAGE_THRESHOLD_COUNTS) {
                g_state = STATE_ARMED;
                g_integral = 0;
                g_error_filt = error;
            }
            break;
        case STATE_ARMED:
            if (mag > GIVE_UP_THRESHOLD_COUNTS) {
                g_state = STATE_DISARMED;
            }
            break;
    }
}