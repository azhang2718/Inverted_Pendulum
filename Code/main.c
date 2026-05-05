#include "ti_msp_dl_config.h"
#include "config.h"
#include "control.h"
#include "motor.h"
#include "state.h"
#include "angle_sensor.h"

// =====================================================================
// ISR SHARED FLAGS (must be here or externed)
// =====================================================================
static volatile bool g_tick = false;

// =====================================================================
// TIMER ISR
// =====================================================================
void TIMA0_IRQHandler(void)
{
    if (DL_TimerA_getPendingInterrupt(TIMER_0_INST) == DL_TIMERA_IIDX_ZERO) {
        g_tick = true;
    }
}

// =====================================================================
// GPIO ISR (LIMIT SWITCH HARD STOP)
// =====================================================================
void GROUP1_IRQHandler(void)
{
    uint32_t status = DL_GPIO_getEnabledInterruptStatus(
        LIMITS_PORT,
        LIMITS_LEFT_PIN | LIMITS_RIGHT_PIN
    );

    if (status & (LIMITS_LEFT_PIN | LIMITS_RIGHT_PIN)) {
        g_limit_tripped = true;   // global from control.c
        g_state = STATE_DISARMED;
        motor_stop();
    }

    DL_GPIO_clearInterruptStatus(LIMITS_PORT, status);
}

// =====================================================================
// MAIN LOOP
// =====================================================================
int main(void)
{
    SYSCFG_DL_init();

    NVIC_EnableIRQ(TIMA0_INT_IRQn);
    NVIC_EnableIRQ(GPIOA_INT_IRQn);

    DL_TimerA_startCounter(TIMER_0_INST);

    while (1) {
        if (!g_tick) {
            __WFI();
            continue;
        }

        g_tick = false;

        uint16_t angle;
        if (as5600_read_angle(&angle) == 0) {
            control_step(angle);          // computes g_error from angle
            update_state(g_error);        // <-- this is what's missing
            // then PID + motor_command based on g_state
        }

    }
}