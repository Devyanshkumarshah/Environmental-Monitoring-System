/*
 * dht11.c
 *
 * DHT11 driver implementation using DWT cycle counter for microsecond delays.
 * No hardware timer peripheral required.
 */

#include "dht11.h"

static GPIO_TypeDef *dht_port;
static uint16_t       dht_pin;

/* ---------------------------------------------------------------------
 * Microsecond delay using Cortex-M4 DWT (Data Watchpoint and Trace) unit
 * ------------------------------------------------------------------- */
static void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
}

static void DWT_Delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000U);
    while ((DWT->CYCCNT - start) < ticks) { /* busy wait */ }
}

/* ---------------------------------------------------------------------
 * GPIO direction switching helpers
 * ------------------------------------------------------------------- */
static void DHT11_SetPinOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin   = dht_pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;   // Open-drain (safe with pull-up)
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(dht_port, &GPIO_InitStruct);
}

static void DHT11_SetPinInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin   = dht_pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;           // external pull-up handles idle-high
    HAL_GPIO_Init(dht_port, &GPIO_InitStruct);
}

/* ---------------------------------------------------------------------
 * Public init
 * ------------------------------------------------------------------- */
void DHT11_Init(GPIO_TypeDef *port, uint16_t pin)
{
    dht_port = port;
    dht_pin  = pin;
    DWT_Init();
    DHT11_SetPinOutput();
    HAL_GPIO_WritePin(dht_port, dht_pin, GPIO_PIN_SET);  // idle high
}

/* ---------------------------------------------------------------------
 * Send start signal and wait for sensor's response pulse
 * Returns DHT11_OK if the sensor responded correctly
 * ------------------------------------------------------------------- */
static DHT11_Status_t DHT11_StartSignal(void)
{
    uint32_t timeout;

    /* MCU pulls line low for >18ms to wake the sensor */
    DHT11_SetPinOutput();
    HAL_GPIO_WritePin(dht_port, dht_pin, GPIO_PIN_RESET);
    HAL_Delay(18);

    /* MCU releases line (pulls high) for 20-40us, then listens */
    HAL_GPIO_WritePin(dht_port, dht_pin, GPIO_PIN_SET);
    DWT_Delay_us(30);
    DHT11_SetPinInput();

    /* Sensor should pull line LOW for ~80us (response) */
    timeout = 0;
    while (HAL_GPIO_ReadPin(dht_port, dht_pin) == GPIO_PIN_SET) {
        DWT_Delay_us(1);
        if (++timeout > 100) return DHT11_ERROR_NO_RESPONSE;
    }

    /* Sensor pulls line HIGH for ~80us before sending data */
    timeout = 0;
    while (HAL_GPIO_ReadPin(dht_port, dht_pin) == GPIO_PIN_RESET) {
        DWT_Delay_us(1);
        if (++timeout > 100) return DHT11_ERROR_TIMEOUT;
    }

    /* Wait for that high pulse to end -> data transmission begins next */
    timeout = 0;
    while (HAL_GPIO_ReadPin(dht_port, dht_pin) == GPIO_PIN_SET) {
        DWT_Delay_us(1);
        if (++timeout > 100) return DHT11_ERROR_TIMEOUT;
    }

    return DHT11_OK;
}

/* ---------------------------------------------------------------------
 * Read a single bit.
 * Every bit starts with a ~50us LOW pulse.
 * The following HIGH duration determines the bit value:
 *    ~26-28us high -> bit = 0
 *    ~70us    high -> bit = 1
 * ------------------------------------------------------------------- */
static uint8_t DHT11_ReadBit(void)
{
    uint32_t timeout = 0;

    /* Wait out the ~50us LOW that precedes every bit */
    while (HAL_GPIO_ReadPin(dht_port, dht_pin) == GPIO_PIN_RESET) {
        DWT_Delay_us(1);
        if (++timeout > 100) return 0;
    }

    /* Measure how long the line stays HIGH */
    DWT_Delay_us(40);  // if still high after 40us (past the ~28us "0" window), it's a "1"

    if (HAL_GPIO_ReadPin(dht_port, dht_pin) == GPIO_PIN_SET) {
        /* Still high -> this was a "1" bit; wait for it to fall before returning */
        timeout = 0;
        while (HAL_GPIO_ReadPin(dht_port, dht_pin) == GPIO_PIN_SET) {
            DWT_Delay_us(1);
            if (++timeout > 100) break;
        }
        return 1;
    }

    return 0;
}

static uint8_t DHT11_ReadByte(void)
{
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        byte <<= 1;
        byte |= DHT11_ReadBit();
    }
    return byte;
}

/* ---------------------------------------------------------------------
 * Public read function
 * ------------------------------------------------------------------- */
DHT11_Status_t DHT11_Read(DHT11_Data_t *data)
{
    DHT11_Status_t status = DHT11_StartSignal();
    if (status != DHT11_OK) {
        DHT11_SetPinOutput();
        HAL_GPIO_WritePin(dht_port, dht_pin, GPIO_PIN_SET);  // release line, idle high
        return status;
    }

    data->humidity_int    = DHT11_ReadByte();
    data->humidity_dec    = DHT11_ReadByte();
    data->temperature_int = DHT11_ReadByte();
    data->temperature_dec = DHT11_ReadByte();
    data->checksum        = DHT11_ReadByte();

    /* Release the line back to output/idle-high state for next cycle */
    DHT11_SetPinOutput();
    HAL_GPIO_WritePin(dht_port, dht_pin, GPIO_PIN_SET);

    uint8_t sum = (uint8_t)(data->humidity_int + data->humidity_dec +
                            data->temperature_int + data->temperature_dec);

    if (sum != data->checksum) {
        return DHT11_ERROR_CHECKSUM;
    }

    return DHT11_OK;
}
