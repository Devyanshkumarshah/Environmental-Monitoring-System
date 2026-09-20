/*
 * dht11.h
 *
 * DHT11 Temperature & Humidity Sensor Driver
 * Bit-banged single-wire protocol implementation for STM32L4 (HAL)
 *
 * IMPORTANT (CubeMX setup):
 *  - Configure the DHT11 data pin as GPIO_Output, Open-Drain (or Push-Pull),
 *    No pull-up/pull-down internally is fine if you have an EXTERNAL 4.7k-10k
 *    pull-up resistor on the data line (standard DHT11 wiring).
 *  - This driver switches the pin direction at runtime (output -> input -> output),
 *    so the exact CubeMX initial mode doesn't matter much, but Open-Drain is safest.
 */

#ifndef INC_DHT11_H_
#define INC_DHT11_H_

#include "main.h"   // Brings in HAL + your GPIO port/pin defines
#include <stdint.h>

typedef enum {
    DHT11_OK = 0,
    DHT11_ERROR_NO_RESPONSE,
    DHT11_ERROR_TIMEOUT,
    DHT11_ERROR_CHECKSUM
} DHT11_Status_t;

typedef struct {
    uint8_t humidity_int;
    uint8_t humidity_dec;
    uint8_t temperature_int;
    uint8_t temperature_dec;
    uint8_t checksum;
} DHT11_Data_t;

/* Call once at startup (e.g. in main() before scheduler starts) */
void DHT11_Init(GPIO_TypeDef *port, uint16_t pin);

/* Blocking read - takes ~20-25ms worst case. Call at most once every 1s (DHT11 spec). */
DHT11_Status_t DHT11_Read(DHT11_Data_t *data);

#endif /* INC_DHT11_H_ */
