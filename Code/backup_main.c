// #include "ti_msp_dl_config.h"
// #include <stdint.h>
// #include <stdbool.h>
// #include <math.h>

// // parameter

// // AS5600 I2C address and register
// #define AS5600_ADDR         0x36
// #define AS5600_RAW_ANGLE_H  0x0C

// // PWM period, 32MHz / 3200 = 10kHz step pulse)
// #define PWM_PERIOD          3200

// // Physical calibration
// #define STEPS_PER_MM        40.0      // how many steps does it take for the cart to move 1 millimeter, adjust later
// #define SCALE_FACTOR        (-1.0/100.0)  // scaling factor for the cart's position when calculating the PID input value
//                                           // negative sign indicates the direction
//                                           // Carey's non-inverted config
// #define OUTPUT_DIR          (-1)          // Motor rotation diretion, Carey's non-inverted config, adjust(1 or -1) later

// // PID gains (from Carey, tune after hardware test)
// #define KP  250.0
// #define KI    0.0
// #define KD    5.0

// // PID sample time in seconds, 5ms timer
// #define DT  0.005

// // Motor output limits, steps/s, maps to PWM duty cycle
// #define OUTPUT_MAX   5000.0
// #define OUTPUT_MIN  -5000.0

// // Pendulum considered fallen if |input| >= this threshold, then reset, adjust later
// #define FALL_THRESHOLD  150.0


// #define ANGLE_OFFSET_RAW  0  // measure and fill this later




// // Variables that need to be tracked during program

// // Global state
// // volatile: tell the compiler that this variable may be modified unexpectedly,
// // so it should not perform any optimizations and must re-read it from memory every time it's used
// volatile bool     g_timer_flag  = true; // start
// volatile uint16_t g_angle_raw   = 0;    // raw data read from AS5600, 0-4095
// volatile int32_t  g_step_count  = 0;   // cumulative steps, + -, for the position of cart


// double ang_deg; 

// // PID state
// // static: variable is only visible in the current file and remains in memory throughout programming
// static double g_integral   = 0.0; // the cumulative value of the integral term. 
//                                 // each time the control loop runs, the current error is multiplied by DT and added to this
//                                 // It records the accumulation of the error over time

// static double g_prev_error = 0.0; // the error value from the previous loop. 
//                                 // the derivative term is calculated by subtracting the previous error from the current error and dividing the result by DT
//                                 // estimate the rate of change of the error
//                                 // At the end of each loop, the current error is stored for use in the next loop

// uint16_t AS5600_ReadAngle(void)
// {
//     uint8_t reg = AS5600_RAW_ANGLE_H;

//     // ----- WRITE: send register address -----
//     // Wait for I2C to be idle before starting
//     while (!(DL_I2C_getControllerStatus(I2C_0_INST) &
//              DL_I2C_CONTROLLER_STATUS_IDLE));

//     DL_I2C_fillControllerTXFIFO(I2C_0_INST, &reg, 1);
//     DL_I2C_startControllerTransfer(I2C_0_INST, AS5600_ADDR,
//                                    DL_I2C_CONTROLLER_DIRECTION_TX, 1);

//     // Wait for transfer to finish
//     while (DL_I2C_getControllerStatus(I2C_0_INST) &
//            DL_I2C_CONTROLLER_STATUS_BUSY_BUS);

//     // Did the slave actually ACK?
//     if (DL_I2C_getControllerStatus(I2C_0_INST) &
//         DL_I2C_CONTROLLER_STATUS_ERROR) {
//         return 0xFFFF;  // sentinel for "comms failed"
//     }

//     // ----- READ: 2 bytes back -----
//     while (!(DL_I2C_getControllerStatus(I2C_0_INST) &
//              DL_I2C_CONTROLLER_STATUS_IDLE));

//     DL_I2C_startControllerTransfer(I2C_0_INST, AS5600_ADDR,
//                                    DL_I2C_CONTROLLER_DIRECTION_RX, 2);

//     // Wait for each byte to actually be in the RX FIFO before reading
//     while (DL_I2C_isControllerRXFIFOEmpty(I2C_0_INST));
//     uint8_t hi = DL_I2C_receiveControllerData(I2C_0_INST);

//     while (DL_I2C_isControllerRXFIFOEmpty(I2C_0_INST));
//     uint8_t lo = DL_I2C_receiveControllerData(I2C_0_INST);

//     // Wait for full transfer to complete and check error
//     while (DL_I2C_getControllerStatus(I2C_0_INST) &
//            DL_I2C_CONTROLLER_STATUS_BUSY_BUS);

//     if (DL_I2C_getControllerStatus(I2C_0_INST) &
//         DL_I2C_CONTROLLER_STATUS_ERROR) {
//         return 0xFFFF;
//     }

//     return ((uint16_t)(hi & 0x0F) << 8) | lo;
// }


// // // Read the current angle from the AS5600 via I2C, return a value between 0-4095
// // uint16_t AS5600_ReadAngle(void)
// // {
// //     uint8_t reg = AS5600_RAW_ANGLE_H; // address in temporary variable

// //     // I2C read
// //     DL_I2C_fillControllerTXFIFO(I2C_0_INST, &reg, 1); // put address in buffer
// //     DL_I2C_startControllerTransfer(I2C_0_INST, AS5600_ADDR,
// //         DL_I2C_CONTROLLER_DIRECTION_TX, 1); // TX: send
// //     while (DL_I2C_getControllerStatus(I2C_0_INST) &
// //            DL_I2C_CONTROLLER_STATUS_BUSY_BUS); // wait until send end

// //     // accept
// //     DL_I2C_startControllerTransfer(I2C_0_INST, AS5600_ADDR,
// //         DL_I2C_CONTROLLER_DIRECTION_RX, 2); // RX: accept
// //     while (DL_I2C_getControllerStatus(I2C_0_INST) &
// //            DL_I2C_CONTROLLER_STATUS_BUSY_BUS);

// //     // 12-digit angle number, 0-360
// //     uint8_t hi = DL_I2C_receiveControllerData(I2C_0_INST); // high byte
// //     uint8_t lo = DL_I2C_receiveControllerData(I2C_0_INST); // low byte
// //     return ((uint16_t)(hi & 0x0F) << 8) | lo; // add together
// // }



// // Motor
// // speed_signed: positive = forward, negative = backward
// // magnitude maps to PWM duty cycle
// void Motor_Set(double speed_signed)
// {
//     // control between -5000-5000
//     if (speed_signed > OUTPUT_MAX)  speed_signed =  OUTPUT_MAX;
//     if (speed_signed < OUTPUT_MIN)  speed_signed =  OUTPUT_MIN;


//     uint8_t dir;
//     double  spd;

//     if (speed_signed >= 0) {
//         dir = 1; // sign of speed
//         spd = speed_signed; // absolute value
//     } else {
//         dir = 0;
//         spd = -speed_signed;
//     }

//     int32_t steps_this_cycle = (int32_t)(spd * DT);

//     // Map speed (0~OUTPUT_MAX) to PWM compare value (0~PWM_PERIOD)
//     uint32_t compare = PWM_PERIOD - (uint32_t)(spd / OUTPUT_MAX * PWM_PERIOD); // explaination

//     if (dir) {
//         DL_GPIO_setPins(GPIO_GRP_0_PORT, GPIO_GRP_0_DIR_PIN);
//         g_step_count += steps_this_cycle; 
//     } else {
//         DL_GPIO_clearPins(GPIO_GRP_0_PORT, GPIO_GRP_0_DIR_PIN);
//         g_step_count -= steps_this_cycle; 
//     }
//     DL_TimerG_setCaptureCompareValue(PWM_0_INST, compare, DL_TIMER_CC_0_INDEX);
// }

// // stop
// void Motor_Stop(void)
// {
//     DL_TimerG_setCaptureCompareValue(PWM_0_INST, PWM_PERIOD, DL_TIMER_CC_0_INDEX);
// }

// // PID
// // double: 64-bit float
// double PID_Compute(double input)
// {
//     // setpoint is 0: we want input == 0
//     double error = 0.0 - input;

//     g_integral   += error * DT;

//     double deriv  = (error - g_prev_error) / DT;
//     g_prev_error  = error;
//     // If the object is tilting rapidly to one side, the differential component will detect this
//     // and apply a counterforce before the error becomes significant, prevent oscillation

//     return KP * error + KI * g_integral + KD * deriv; // Final output of PID
//     // KI=0 first
// }

// // Timer ISR (every 5ms), interrupt
// void TIMA0_IRQHandler(void)
// {
//     switch (DL_TimerA_getPendingInterrupt(TIMER_0_INST)) {
//         case DL_TIMERA_IIDX_ZERO:
//             g_timer_flag = true;
//             break;
//         default:
//             break;
//     }
// }

// // Main
// int main(void)
// {
//     SYSCFG_DL_init(); // start all
//     NVIC_EnableIRQ(TIMA0_INT_IRQn); // start check the interrrupt of TIMA0
//     DL_TimerA_startCounter(TIMER_0_INST);
//     DL_TimerG_startCounter(PWM_0_INST);

//     while (1) {
//         if (!g_timer_flag) continue;
//         g_timer_flag = true;
//         // prevent an interrupt being dealt twice

//         // 0-4095 -> angle -> degrees
//         g_angle_raw     = AS5600_ReadAngle();
//         int16_t corrected = (int16_t) g_angle_raw - ANGLE_OFFSET_RAW;
//         ang_deg  = corrected * (360.0 / 4096.0);
//         double ang_rad  = ang_deg * (3.14159265 / 180.0);

//         // Combine cart position and pendulum angle
//         // same as Carey's design
//         double cart_mm  = (double)g_step_count / STEPS_PER_MM;
//         double input    = SCALE_FACTOR * cart_mm + 200.0 * sin(ang_rad);

//         // Stop if pendulum has fallen
//         if (input > FALL_THRESHOLD || input < -FALL_THRESHOLD) {
//             Motor_Stop();
//             g_integral   = 0.0;
//             g_prev_error = 0.0;
//             continue;
//         }

//         // PID -> motor
//         double output = PID_Compute(input);
//         Motor_Set((double)OUTPUT_DIR * output);
//     }
// }