/*
 * mq2.h
 *
 * MQ-2 Smoke/Gas Sensor Driver — analog, single ADC channel
 * Target: STM32L476RG, HAL
 *
 * MQ-2 is not a digital/protocol sensor - it's a simple analog resistive
 * gas sensor. This driver wraps the ADC read + basic sanity handling so
 * your task code stays clean and consistent with the other sensor drivers.
 *
 * Wiring:
 *   VCC  -> 5V (MQ-2 heater needs 5V; check your board's AOUT max swing
 *               against the STM32's 3.3V ADC limit - add a voltage divider
 *               if needed, see project notes)
 *   GND  -> GND
 *   AOUT -> an ADC-capable GPIO (e.g. PA0 / ADC1_IN5)
 *   DOUT -> not used (thresholding is done in software, in process_task)
 */

#ifndef INC_MQ2_H_
#define INC_MQ2_H_

#include "main.h"
#include <stdint.h>

typedef enum {
    MQ2_OK = 0,
    MQ2_ERROR_ADC,
    MQ2_ERROR_NOT_INIT
} MQ2_Status_t;

/* Call once at startup, after MX_ADCx_Init() has run. */
MQ2_Status_t MQ2_Init(ADC_HandleTypeDef *hadc);

/* Blocking read of the raw 12-bit ADC value (0-4095).
 * Higher value = more smoke/gas detected (MQ-2's output rises with
 * increasing gas concentration, given the standard analog front-end). */
MQ2_Status_t MQ2_ReadRaw(uint16_t *raw_value);

#endif /* INC_MQ2_H_ */
