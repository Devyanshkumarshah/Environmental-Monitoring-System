/*
 * oled.c
 *
 * Keeps the last known temp and smoke readings in static state, and
 * redraws the WHOLE screen (both lines) every time either one updates.
 * This avoids one sensor's line disappearing when the other sensor's
 * update comes in and clears the shared framebuffer.
 *
 * Adjust the ssd1306_* calls below if your downloaded version of the
 * library uses an SSD1306_t* handle instead of implicit global state -
 * check your actual ssd1306.h for the exact function signatures.
 */

#include "oled.h"
#include "ssd1306.h"
#include <stdio.h>
#include <string.h>
#include "ssd1306_fonts.h"   /* filename may be fonts.h depending on which fork you grabbed */


static float   last_temp_c   = 0.0f;
static uint8_t last_temp_alert = 0;
static int16_t last_smoke_raw  = 0;
static uint8_t last_smoke_alert = 0;

static void OLED_Redraw(void)
{
    char line[24];

    ssd1306_Fill(Black);

    /* --- Temperature line --- */
    ssd1306_SetCursor(2, 0);
    snprintf(line, sizeof(line), "Temp: %.1f C", last_temp_c);
    ssd1306_WriteString(line, Font_7x10, White);

    if (last_temp_alert)
    {
        ssd1306_SetCursor(90, 0);
        ssd1306_WriteString("!", Font_7x10, White);
    }

    /* --- Smoke line --- */
    ssd1306_SetCursor(2, 20);
    snprintf(line, sizeof(line), "Smoke: %d", last_smoke_raw);
    ssd1306_WriteString(line, Font_7x10, White);

    if (last_smoke_alert)
    {
        ssd1306_SetCursor(90, 20);
        ssd1306_WriteString("!", Font_7x10, White);
    }

    ssd1306_UpdateScreen();
}

void OLED_Init(I2C_HandleTypeDef *hi2c)
{
    ssd1306_Init();   /* some forks want ssd1306_Init(hi2c) - match to your downloaded header */
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();
}

void OLED_ShowTemp(float value_c, uint8_t alert)
{
    last_temp_c     = value_c;
    last_temp_alert = alert;
    OLED_Redraw();
}

void OLED_ShowSmoke(int16_t value_raw, uint8_t alert)
{
    last_smoke_raw    = value_raw;
    last_smoke_alert  = alert;
    OLED_Redraw();
}
