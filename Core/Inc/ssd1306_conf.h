#ifndef SSD1306_CONF_H
#define SSD1306_CONF_H

/* MCU family - required by ssd1306.h's internal #if/#elif chain */
#define STM32L4

/* Interface selection - pick ONE (this project uses I2C) */
#define SSD1306_USE_I2C

/* I2C handle + 7-bit address (shifted left by 1, per HAL convention) */
#define SSD1306_I2C_PORT   hi2c1
#define SSD1306_I2C_ADDR   (0x3C << 1)

/* Display geometry - adjust HEIGHT to 32 if yours is the shorter variant */
#define SSD1306_WIDTH      128
#define SSD1306_HEIGHT     64

#define SSD1306_INCLUDE_FONT_7x10

/* Uncomment only if your display renders upside-down/mirrored */
/* #define SSD1306_MIRROR_VERT */
/* #define SSD1306_MIRROR_HORIZ */

/* Uncomment only if the image looks scrambled/interlaced */
/* #define SSD1306_COM_LR_REMAP */

#endif /* SSD1306_CONF_H */
