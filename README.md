## Environment Monitoring System

A real-time embedded monitoring system built on **STM32L476RG** using **FreeRTOS**,
combining smoke and temperature sensing with alert-driven OLED display output.

### RTOS Architecture
- 4 prioritized tasks (smoke, temperature, processing, display) with a 16KB heap
  and 128-word stacks per task
- Two FreeRTOS queues implementing a producer-consumer pipeline for sensor data
  and display updates
- Mutex-protected shared I2C1 bus to safely coordinate concurrent access
  between the BMP180 sensor and SSD1306 OLED

### Sensor Integration
- **BMP180** (I2C) — temperature readings with Bosch's compensation algorithm
- **MQ-2** (ADC) — smoke/gas concentration via analog sampling
- Threshold-based alert logic with real-time visual feedback on the OLED

**Stack:** STM32CubeIDE, HAL, FreeRTOS (CMSIS-RTOS v2), C
