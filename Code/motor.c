/*
* code for stepper motor driver
*/
#include "motor.h"
#include "ti_msp_dl_config.h"
#include "config.h"

//whether PWM for motor is running
static bool g_motor_on = false;

//direction control for motor
static void motor_dir_pos(void) { DL_GPIO_setPins(GPIO_GRP_0_PORT, GPIO_GRP_0_DIR_PIN); }
static void motor_dir_neg(void) { DL_GPIO_clearPins(GPIO_GRP_0_PORT, GPIO_GRP_0_DIR_PIN); }

//configure PWM for the requested frequency 
static void motor_run(uint32_t step_hz)
{
    //hard limit on smallest PWM
    if (step_hz < 10) step_hz = 10;
    uint32_t period = TIMER_CLK_HZ / step_hz;
    //hardware needs minimum count
    if (period < 4) period = 4;

    //use load val for period and compare val for duty cycle (50%)
    DL_TimerG_setLoadValue(PWM_0_INST, period - 1);
    DL_TimerG_setCaptureCompareValue(PWM_0_INST, period / 2, DL_TIMER_CC_0_INDEX);

    //only start counter if it isn't alr running
    if (!g_motor_on) {
        DL_TimerG_startCounter(PWM_0_INST);
        g_motor_on = true;
    }
}

// stop motor by stopping PWM
void motor_stop(void)
{
    if (g_motor_on) {
        //stop PWM
        DL_TimerG_stopCounter(PWM_0_INST);
        //set flag to false
        g_motor_on = false;
    }
}

//set direction and speed of motor
void motor_command(int8_t dir, uint32_t step_hz)
{
    if (dir != 0) {
        //set DIR pin before STEP to allow for setup time
        if (dir > 0) motor_dir_pos();
        else         motor_dir_neg();

        //wait a few us so DIR settles before next STEP
        for(volatile int i=0; i<100; i++);
        //alter motor PWM to change speed
        motor_run(step_hz);
    } else {
        motor_stop();
    }
}