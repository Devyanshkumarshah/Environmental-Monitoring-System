/*
 * oled.h
 *
 * Thin wrapper around the afiskon/stm32-ssd1306 library, providing the
 * OLED_ShowTemp() / OLED_ShowSmoke() interface used by StartTask04.
 *
 * PREREQUISITE: download ssd1306.h/.c, ssd1306_conf.h, fonts.h/.c from
 * https://github.com/afiskon/stm32-ssd1306 and add them to your project
 * (Core/Inc and Core/Src) before this file will compile.
 */

#ifndef INC_OLED_H_
#define INC_OLED_H_

#include "main.h"
#include <stdint.h>

/* Call once at startup (e.g. inside oled_task, before its main loop) */
void OLED_Init(I2C_HandleTypeDef *hi2c);

/* Update the temperature line. value is degrees C, alert = 1 highlights it. */
void OLED_ShowTemp(float value_c, uint8_t alert);

/* Update the smoke line. value is raw ADC counts, alert = 1 highlights it. */
void OLED_ShowSmoke(int16_t value_raw, uint8_t alert);

#endif /* INC_OLED_H_ */
