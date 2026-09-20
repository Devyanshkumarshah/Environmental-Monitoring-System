/*
 * bmp180.h
 *
 * BMP180 (GY-68) Temperature Sensor Driver — I2C, temperature-only
 * Target: STM32L476RG, HAL + CMSIS-RTOS v2 (matches generated freertos.c style)
 *
 * Wiring (shared I2C1 bus with OLED):
 *   VIN -> 3.3V
 *   GND -> GND
 *   SCL -> PB8 (I2C1_SCL)
 *   SDA -> PB9 (I2C1_SDA)
 *
 * I2C address: 0x77 (7-bit) -> shifted to 0xEE for HAL's 8-bit address field
 */

#ifndef INC_BMP_H_
#define INC_BMP_H_

#include "main.h"
#include <stdint.h>

typedef enum {
    BMP180_OK = 0,
    BMP180_ERROR_I2C,
    BMP180_ERROR_NOT_INIT
} BMP180_Status_t;

/* Call once at startup, after MX_I2C1_Init() has run.
 * Reads the sensor's factory calibration coefficients from EEPROM. */
BMP180_Status_t BMP180_Init(I2C_HandleTypeDef *hi2c);

/* Blocking read - internally waits ~4.5ms for sensor conversion using osDelay()
 * (yields to scheduler, does not busy-wait). Call from a FreeRTOS task context. */
BMP180_Status_t BMP180_ReadTemperature(float *temperature_c);

#endif /* INC_BMP180_H_ */
