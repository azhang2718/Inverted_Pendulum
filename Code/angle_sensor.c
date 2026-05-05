#include "angle_sensor.h"
#include "ti_msp_dl_config.h"
#include "config.h"

//reads the returned angle of sensor using i2c
uint8_t as5600_read_angle(uint16_t *out)
{
    uint8_t reg = AS5600_RAW_ANGLE_H;
    uint32_t to;

    to = 200000;
    while (!(DL_I2C_getControllerStatus(I2C_0_INST) & DL_I2C_CONTROLLER_STATUS_IDLE))
        if (--to == 0) return 1;
    DL_I2C_flushControllerTXFIFO(I2C_0_INST);

    DL_I2C_fillControllerTXFIFO(I2C_0_INST, &reg, 1);
    DL_I2C_startControllerTransfer(I2C_0_INST, AS5600_ADDR, 
                                   DL_I2C_CONTROLLER_DIRECTION_TX, 1);
    to = 200000;
    while (DL_I2C_getControllerStatus(I2C_0_INST) & DL_I2C_CONTROLLER_STATUS_BUSY_BUS)
        if (--to == 0) return 2;
    if (DL_I2C_getControllerStatus(I2C_0_INST) & DL_I2C_CONTROLLER_STATUS_ERROR) return 2;

    to = 200000;
    while (!(DL_I2C_getControllerStatus(I2C_0_INST) & DL_I2C_CONTROLLER_STATUS_IDLE))
        if (--to == 0) return 3;

    DL_I2C_startControllerTransfer(I2C_0_INST, AS5600_ADDR, 
                                   DL_I2C_CONTROLLER_DIRECTION_RX, 2);

    uint8_t buf[2];
    for (uint8_t i = 0; i < 2; i++) {
        to = 1000000;
        while (DL_I2C_isControllerRXFIFOEmpty(I2C_0_INST)) {
            if (DL_I2C_getControllerStatus(I2C_0_INST) & DL_I2C_CONTROLLER_STATUS_ERROR) return 5;
            if (--to == 0) return 4;
        }
        buf[i] = DL_I2C_receiveControllerData(I2C_0_INST);
    }
    *out = ((uint16_t)(buf[0] & 0x0F) << 8) | buf[1];
    return 0;
}
