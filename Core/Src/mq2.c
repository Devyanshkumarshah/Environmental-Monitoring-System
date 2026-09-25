
#include <mq2.h>

static ADC_HandleTypeDef *mq2_hadc = NULL;
static uint8_t            initialized = 0;

#define ADC_CONVERSION_TIMEOUT_MS   50

MQ2_Status_t MQ2_Init(ADC_HandleTypeDef *hadc)
{
    mq2_hadc = hadc;
    initialized = 1;
    return MQ2_OK;
}

MQ2_Status_t MQ2_ReadRaw(uint16_t *raw_value)
{
    if (!initialized)
    {
        return MQ2_ERROR_NOT_INIT;
    }

    if (HAL_ADC_Start(mq2_hadc) != HAL_OK)
    {
        return MQ2_ERROR_ADC;
    }

    if (HAL_ADC_PollForConversion(mq2_hadc, ADC_CONVERSION_TIMEOUT_MS) != HAL_OK)
    {
        HAL_ADC_Stop(mq2_hadc);
        return MQ2_ERROR_ADC;
    }

    *raw_value = (uint16_t)HAL_ADC_GetValue(mq2_hadc);

    HAL_ADC_Stop(mq2_hadc);

    return MQ2_OK;
}
