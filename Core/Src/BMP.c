/*
 * bmp180.c
 *
 * BMP180 temperature-only driver.
 * Uses blocking HAL I2C calls (transactions are 1-3 bytes, negligible bus time
 * at this read frequency) and osDelay() for the mandatory sensor conversion
 * wait, so the FreeRTOS scheduler can run other tasks during that time
 * instead of busy-waiting.
 */

#include "BMP.h"
#include "cmsis_os.h"

#define BMP180_I2C_ADDR   (0x77 << 1)   /* HAL wants the 8-bit shifted address */

/* Calibration coefficient EEPROM registers (datasheet section 3.3) */
#define REG_CAL_AC1   0xAA
#define REG_CAL_AC2   0xAC
#define REG_CAL_AC3   0xAE
#define REG_CAL_AC4   0xB0
#define REG_CAL_AC5   0xB2
#define REG_CAL_AC6   0xB4
#define REG_CAL_B1    0xB6
#define REG_CAL_B2    0xB8
#define REG_CAL_MB    0xBA
#define REG_CAL_MC    0xBC
#define REG_CAL_MD    0xBE

#define REG_CONTROL      0xF4
#define REG_TEMPDATA     0xF6
#define CMD_READ_TEMP    0x2E

typedef struct {
    int16_t  AC1, AC2, AC3;
    uint16_t AC4, AC5, AC6;
    int16_t  B1, B2;
    int16_t  MB, MC, MD;
} BMP180_Calib_t;

static I2C_HandleTypeDef *bmp_hi2c   = NULL;
static BMP180_Calib_t     calib;
static uint8_t            initialized = 0;

/* Read a big-endian 16-bit value from a register pair */
static BMP180_Status_t BMP180_ReadInt16(uint8_t reg, int16_t *out)
{
    uint8_t buf[2];
    if (HAL_I2C_Mem_Read(bmp_hi2c, BMP180_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT,
                          buf, 2, HAL_MAX_DELAY) != HAL_OK) {
        return BMP180_ERROR_I2C;
    }
    *out = (int16_t)((buf[0] << 8) | buf[1]);
    return BMP180_OK;
}

static BMP180_Status_t BMP180_ReadUInt16(uint8_t reg, uint16_t *out)
{
    uint8_t buf[2];
    if (HAL_I2C_Mem_Read(bmp_hi2c, BMP180_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT,
                          buf, 2, HAL_MAX_DELAY) != HAL_OK) {
        return BMP180_ERROR_I2C;
    }
    *out = (uint16_t)((buf[0] << 8) | buf[1]);
    return BMP180_OK;
}

BMP180_Status_t BMP180_Init(I2C_HandleTypeDef *hi2c)
{
    bmp_hi2c = hi2c;

    if (BMP180_ReadInt16(REG_CAL_AC1, &calib.AC1)  != BMP180_OK) return BMP180_ERROR_I2C;
    if (BMP180_ReadInt16(REG_CAL_AC2, &calib.AC2)  != BMP180_OK) return BMP180_ERROR_I2C;
    if (BMP180_ReadInt16(REG_CAL_AC3, &calib.AC3)  != BMP180_OK) return BMP180_ERROR_I2C;
    if (BMP180_ReadUInt16(REG_CAL_AC4, &calib.AC4) != BMP180_OK) return BMP180_ERROR_I2C;
    if (BMP180_ReadUInt16(REG_CAL_AC5, &calib.AC5) != BMP180_OK) return BMP180_ERROR_I2C;
    if (BMP180_ReadUInt16(REG_CAL_AC6, &calib.AC6) != BMP180_OK) return BMP180_ERROR_I2C;
    if (BMP180_ReadInt16(REG_CAL_B1,  &calib.B1)   != BMP180_OK) return BMP180_ERROR_I2C;
    if (BMP180_ReadInt16(REG_CAL_B2,  &calib.B2)   != BMP180_OK) return BMP180_ERROR_I2C;
    if (BMP180_ReadInt16(REG_CAL_MB,  &calib.MB)   != BMP180_OK) return BMP180_ERROR_I2C;
    if (BMP180_ReadInt16(REG_CAL_MC,  &calib.MC)   != BMP180_OK) return BMP180_ERROR_I2C;
    if (BMP180_ReadInt16(REG_CAL_MD,  &calib.MD)   != BMP180_OK) return BMP180_ERROR_I2C;

    initialized = 1;
    return BMP180_OK;
}

BMP180_Status_t BMP180_ReadTemperature(float *temperature_c)
{
    if (!initialized) {
        return BMP180_ERROR_NOT_INIT;
    }

    /* 1. Trigger temperature conversion */
    uint8_t cmd = CMD_READ_TEMP;
    if (HAL_I2C_Mem_Write(bmp_hi2c, BMP180_I2C_ADDR, REG_CONTROL, I2C_MEMADD_SIZE_8BIT,
                           &cmd, 1, HAL_MAX_DELAY) != HAL_OK) {
        return BMP180_ERROR_I2C;
    }

    /* 2. Datasheet requires ~4.5ms conversion time.
     *    osDelay() yields to the scheduler instead of busy-waiting. */
    osDelay(5);

    /* 3. Read the uncompensated temperature (UT) */
    int16_t ut_raw;
    if (BMP180_ReadInt16(REG_TEMPDATA, &ut_raw) != BMP180_OK) {
        return BMP180_ERROR_I2C;
    }
    int32_t UT = ut_raw;

    /* 4. Apply Bosch's compensation formula (datasheet section 3.5) */
    int32_t X1 = ((UT - (int32_t)calib.AC6) * (int32_t)calib.AC5) >> 15;
    int32_t X2 = ((int32_t)calib.MC << 11) / (X1 + calib.MD);
    int32_t B5 = X1 + X2;
    int32_t T  = (B5 + 8) >> 4;   /* result in units of 0.1 degC */

    *temperature_c = T / 10.0f;

    return BMP180_OK;
}
