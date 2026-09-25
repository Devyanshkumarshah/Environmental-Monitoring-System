/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#include <mq2.h>
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "i2c.h"
#include "BMP.h"
#include "i2c.h"
#include "adc.h"
#include "BMP.h"
#include "oled.h"

/* USER CODE END PTD */


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE BEGIN PTD */
typedef struct {
    uint8_t  flags;
    int16_t  value;
} SensorQueueItem_t;

#define SRC_SMOKE_BIT   (0U << 0)
#define SRC_TEMP_BIT    (1U << 0)
#define ALERT_BIT       (1U << 1)

#define IS_TEMP(item)   ((item).flags & SRC_TEMP_BIT)
#define IS_ALERT(item)  ((item).flags & ALERT_BIT)
/* USER CODE END PTD */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SMOKE_THRESHOLD_RAW   2000
#define TEMP_THRESHOLD_C10    500
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for smoke_task */
osThreadId_t smoke_taskHandle;
const osThreadAttr_t smoke_task_attributes = {
  .name = "smoke_task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for temp_task */
osThreadId_t temp_taskHandle;
const osThreadAttr_t temp_task_attributes = {
  .name = "temp_task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for process_task */
osThreadId_t process_taskHandle;
const osThreadAttr_t process_task_attributes = {
  .name = "process_task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for oled_task */
osThreadId_t oled_taskHandle;
const osThreadAttr_t oled_task_attributes = {
  .name = "oled_task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for sensor_queue */
osMessageQueueId_t sensor_queueHandle;
const osMessageQueueAttr_t sensor_queue_attributes = {
  .name = "sensor_queue"
};
/* Definitions for display_queue */
osMessageQueueId_t display_queueHandle;
const osMessageQueueAttr_t display_queue_attributes = {
  .name = "display_queue"
};
/* Definitions for hi2c1_mutex */
osMutexId_t hi2c1_mutexHandle;
const osMutexAttr_t hi2c1_mutex_attributes = {
  .name = "hi2c1_mutex"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTask02(void *argument);
void StartTask03(void *argument);
void StartTask04(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of hi2c1_mutex */
  hi2c1_mutexHandle = osMutexNew(&hi2c1_mutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of sensor_queue */
  sensor_queueHandle = osMessageQueueNew (10, 4, &sensor_queue_attributes);

  /* creation of display_queue */
  display_queueHandle = osMessageQueueNew (5, 4, &display_queue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of smoke_task */
  smoke_taskHandle = osThreadNew(StartDefaultTask, NULL, &smoke_task_attributes);

  /* creation of temp_task */
  temp_taskHandle = osThreadNew(StartTask02, NULL, &temp_task_attributes);

  /* creation of process_task */
  process_taskHandle = osThreadNew(StartTask03, NULL, &process_task_attributes);

  /* creation of oled_task */
  oled_taskHandle = osThreadNew(StartTask04, NULL, &oled_task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */

  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the smoke_task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)            // it uses ADC
{
  /* USER CODE BEGIN StartDefaultTask */
	SensorQueueItem_t item;
	uint16_t smoke_raw;

	MQ2_Init(&hadc1);
  /* Infinite loop */
  for(;;)
  {
	  if (MQ2_ReadRaw(&smoke_raw) == MQ2_OK)
	  {
		  item.flags = SRC_SMOKE_BIT;
	      item.value = (int16_t)smoke_raw;
	      osMessageQueuePut(sensor_queueHandle, &item, 0, pdMS_TO_TICKS(50));
	  }


    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the temp_task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)                   //uses I2C inteface
{
  /* USER CODE BEGIN StartTask02 */
  /* Infinite loop */
//	HAL_StatusTypeDef ready = HAL_I2C_IsDeviceReady(&hi2c1, 0x77 << 1, 3, 100);
	float temp_c;
	SensorQueueItem_t item;

	if (BMP180_Init(&hi2c1) != BMP180_OK)
	{
		Error_Handler();   /* or set an error flag instead, your call */
	}

	for(;;)
	{
		if (osMutexAcquire(hi2c1_mutexHandle, pdMS_TO_TICKS(100)) == osOK)
		{
			if (BMP180_ReadTemperature(&temp_c) == BMP180_OK)
				{
			    	item.flags = SRC_TEMP_BIT;
			        item.value = (int16_t)(temp_c * 10.0f);
			        osMessageQueuePut(sensor_queueHandle, &item, 0, pdMS_TO_TICKS(50));
			     }
			osMutexRelease(hi2c1_mutexHandle);
		}
		osDelay(200);
	}
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the process_task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void *argument)
{
  /* USER CODE BEGIN StartTask03 */
	SensorQueueItem_t item;
	  /* Infinite loop */
	for(;;)
	{
		osMessageQueueGet(sensor_queueHandle, &item, NULL, osWaitForever);

		/* Threshold check - set/clear the alert bit based on source */
		if (IS_TEMP(item))
		{
		    if (item.value > TEMP_THRESHOLD_C10)
		    {
		    	item.flags |= ALERT_BIT;
		    }
		    else
		    {
		    	item.flags &= ~ALERT_BIT;
		    }
		}
		else /* smoke */
		{
		    if (item.value > SMOKE_THRESHOLD_RAW)
		    {
		    	item.flags |= ALERT_BIT;
		    }
		    else
		    {
		    	item.flags &= ~ALERT_BIT;
		    }
		}

		osMessageQueuePut(display_queueHandle, &item, 0, osWaitForever);
		osDelay(20);
	  }
	  /* USER CODE END StartTask03 */
}

/* USER CODE BEGIN Header_StartTask04 */
/**
* @brief Function implementing the oled_task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask04 */
void StartTask04(void *argument)         // uses I2C interface
{
  /* USER CODE BEGIN StartTask04 */
  SensorQueueItem_t item;

  /* One-time init - must happen before the loop, and needs the I2C bus,
   * so take the mutex briefly here too (temp_task could be mid-transaction). */
  if (osMutexAcquire(hi2c1_mutexHandle, pdMS_TO_TICKS(100)) == osOK)
  {
      OLED_Init(&hi2c1);
      osMutexRelease(hi2c1_mutexHandle);
  }

  for(;;)
  {
      if (osMessageQueueGet(display_queueHandle, &item, 0, osWaitForever) == osOK)
      {
          if (osMutexAcquire(hi2c1_mutexHandle, pdMS_TO_TICKS(100)) == osOK)
          {
              if (IS_TEMP(item))
              {
                  OLED_ShowTemp(item.value / 10.0f, IS_ALERT(item));
              }
              else
              {
                  OLED_ShowSmoke(item.value, IS_ALERT(item));
              }
              osMutexRelease(hi2c1_mutexHandle);
          }
      }
      osDelay(1);
  }
  /* USER CODE END StartTask04 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

