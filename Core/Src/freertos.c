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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

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
  display_queueHandle = osMessageQueueNew (5, 4, &display_queue_attributes); //this is for the display queue

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

  if (oled_taskHandle == NULL)
  {
    Error_Handler();  // put a breakpoint inside Error_Handler() itself
  }

}


void StartDefaultTask(void *argument)    //smoke task
{

  for(;;)
  {
    osDelay(100);
  }

}


float readTemperature(void)          //helper function to read the temperature sensor
{
	static float temp = 25;
	temp = temp + 0.5;
	if(temp > 45)
	{
		temp = 25;
	}
	return temp;
}

void StartTask02(void *argument)     // temperature task
{
  float temp;
  for(;;)
  {
	  temp = readTemperature();
	  osMessageQueuePut(sensor_queueHandle, &temp, 0, 0);
	  osDelay(1000);
  }
}

void StartTask03(void *argument)       //process task
{

  float receivedTemp;
  for(;;)
  {
	  osMessageQueueGet(sensor_queueHandle, &receivedTemp, NULL, osWaitForever);
	  osMessageQueuePut(display_queueHandle, &receivedTemp, 0, osWaitForever);     //till now we have we the data in the display queue

	  osDelay(50);
  }

}


void StartTask04(void *argument)    //display task
{
	float display_temp;
  for(;;)
  {
	  if(osMessageQueueGet(display_queueHandle, &display_temp, 0, osWaitForever) == osOK)
	  {

	  }

	  osDelay();
  }
}



