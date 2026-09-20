# System Design Architecture Document
# Environment Monitoring System
## STM32L476RG + FreeRTOS + HAL

---

# 1. Project Overview

The Environment Monitoring System is a real-time embedded application developed using:

- STM32L476RG Microcontroller
- Embedded C
- STM32 HAL Drivers
- STM32CubeMX
- STM32CubeIDE
- FreeRTOS

The system continuously monitors environmental conditions using a smoke sensor (MQ-2) and a temperature sensor (DHT11), processes the acquired data, detects abnormal conditions, and displays the results on an OLED display.

The architecture follows a modular RTOS-based design where sensing, processing, logging, monitoring, and display functionalities are separated into independent tasks.

---

# 2. System Requirements

## Functional Requirements

- Read smoke sensor data.
- Read temperature data.
- Detect threshold violations.
- Generate environmental alerts.
- Display sensor information on OLED.
- Monitor task health.
- Log critical events.
- Support future IoT and CAN integration.

## Non-Functional Requirements

- Real-time responsiveness.
- Modular architecture.
- Scalability.
- Fault tolerance.
- Low CPU utilization.
- Easy maintenance.

---

# 3. Hardware Architecture

## Microcontroller

STM32L476RG

Features Used:

- GPIO
- ADC
- I2C
- TIM
- SysTick
- FreeRTOS Kernel

---

## Sensors

### MQ-2 Smoke Sensor

Interface:
- ADC

Output:
- Smoke Concentration

---

### DHT11 Temperature Sensor

Interface:
- Single Wire Protocol

Output:
- Temperature

---

### OLED Display (VH02850)

Interface:
- I2C

Displays:

- Smoke Level
- Temperature
- Alert Status
- System State

---

# 4. Software Architecture

Application Layer
│
├── Smoke Task
├── Temperature Task
├── Process Task
├── Display Task
├── Logger Task
├── Watchdog Task
│
├── Queues
├── Task Notifications
├── Mutexes
│
FreeRTOS Kernel
│
HAL Drivers
│
STM32 Hardware

---

# 5. System Architecture Diagram

MQ-2 Sensor
     |
     v
Smoke Task
     |
     v
Smoke Queue
     |
     |
     +------------------+
                        |
                        v

                  Process Task
                        ^
                        |
                        |
     +------------------+
     |
Temp Queue
     ^
     |
Temp Task
     ^
     |
DHT11 Sensor

                        |
                        |
                        v

                  Display Queue
                        |
                        v

                  Display Task
                        |
                        v

                  OLED Display


                  Logger Queue
                        |
                        v

                   Logger Task


                Health Monitor
                        |
                        v

                 Watchdog Task

---

# 6. Task Architecture

## Task 1 : Smoke Task

Priority : 4

Purpose:
Acquire smoke sensor data.

Responsibilities:

- Read ADC value.
- Convert raw ADC value.
- Create smoke message.
- Send message to Smoke Queue.
- Notify Process Task.

Flow:

Read Sensor
→ Package Data
→ Queue Send
→ Notify Process Task
→ Delay
→ Repeat

---

## Task 2 : Temperature Task

Priority : 3

Purpose:
Acquire temperature data.

Responsibilities:

- Read DHT11.
- Validate data.
- Package message.
- Send to Temperature Queue.
- Notify Process Task.

Flow:

Read DHT11
→ Package Data
→ Queue Send
→ Notify Process Task
→ Delay
→ Repeat

---

## Task 3 : Process Task

Priority : 2

Purpose:
Central decision-making task.

Responsibilities:

- Receive sensor data.
- Maintain latest system values.
- Compare against thresholds.
- Generate alerts.
- Determine system state.
- Forward data to display.
- Send logs when required.

System States:

NORMAL
WARNING
CRITICAL

Flow:

Wait For Notification
→ Read Queues
→ Process Data
→ Threshold Check
→ State Update
→ Display Queue Send
→ Logger Queue Send
→ Repeat

---

## Task 4 : Display Task

Priority : 1

Purpose:
Update OLED screen.

Responsibilities:

- Receive processed data.
- Lock display mutex.
- Update OLED.
- Release mutex.

Flow:

Receive Display Data
→ Take Mutex
→ OLED Update
→ Give Mutex
→ Repeat

---

## Task 5 : Logger Task

Priority : 1

Purpose:
Event recording.

Responsibilities:

- Receive events.
- Store logs in memory.
- Provide debugging information.

Examples:

- Smoke Threshold Crossed
- Temperature Threshold Crossed
- Sensor Failure
- Queue Overflow

---

## Task 6 : Watchdog Task

Priority : 5

Purpose:
System health monitoring.

Responsibilities:

- Monitor task heartbeat.
- Detect stalled tasks.
- Detect deadlocks.
- Trigger watchdog action.

Health Checks:

- Smoke Task Alive
- Temp Task Alive
- Process Task Alive
- Display Task Alive

---

# 7. Queue Architecture

## Smoke Queue

Producer:
- Smoke Task

Consumer:
- Process Task

Purpose:
Transfer smoke measurements.

---

## Temperature Queue

Producer:
- Temperature Task

Consumer:
- Process Task

Purpose:
Transfer temperature measurements.

---

## Display Queue

Producer:
- Process Task

Consumer:
- Display Task

Purpose:
Transfer processed display information.

---

## Logger Queue

Producer:
- Process Task
- Watchdog Task

Consumer:
- Logger Task

Purpose:
Store system events.

---

# 8. Inter-Task Communication

## Queues

Used For:

- Data Transfer
- Event Transfer

---

## Task Notifications

Used For:

- Fast signaling
- Reduced RAM usage
- Immediate Process Task wake-up

Example:

Smoke Task
→ xTaskNotifyGive()

Process Task
→ ulTaskNotifyTake()

---

## Mutex

OLED Mutex

Purpose:

Prevent simultaneous OLED access from multiple tasks.

Example:

xSemaphoreTake(oledMutex, portMAX_DELAY);

OLED_Update();

xSemaphoreGive(oledMutex);

---

# 9. Data Structures

## Sensor Type

```c
typedef enum
{
    SENSOR_SMOKE,
    SENSOR_TEMPERATURE
} SensorType_t;
```

## Sensor Message

```c
typedef struct
{
    SensorType_t sensorType;
    float value;
    uint32_t timestamp;
} SensorMessage_t;
```

## Display Message

```c
typedef enum
{
    SYSTEM_NORMAL,
    SYSTEM_WARNING,
    SYSTEM_CRITICAL
} SystemState_t;

typedef struct
{
    float temperature;
    float smoke;
    SystemState_t state;
    uint8_t alertFlag;
} DisplayMessage_t;
```

## Logger Message

```c
typedef struct
{
    uint32_t timestamp;
    char message[64];
} LogMessage_t;
```

---

# 10. State Machine Design

## NORMAL

Conditions:

- Smoke < Threshold
- Temperature < Threshold

Action:

- Normal display

---

## WARNING

Conditions:

- One sensor exceeds threshold

Action:

- Warning display
- Log event

---

## CRITICAL

Conditions:

- Multiple threshold violations
- Extreme values

Action:

- Alert display
- Critical log generation

---

# 11. Threshold Management

## Smoke Threshold

```c
#define SMOKE_THRESHOLD 500
```

## Temperature Threshold

```c
#define TEMP_THRESHOLD 40
```

Example:

```c
if(smokeValue > SMOKE_THRESHOLD)
{
    alertFlag = 1;
}
```

---

# 12. Scheduling Strategy

Scheduler:

FreeRTOS Preemptive Scheduler

Scheduling Method:

- Fixed Priority Scheduling
- Round Robin among equal priorities

Task Priorities:

| Task | Priority |
|--------|----------|
| Watchdog Task | 5 |
| Smoke Task | 4 |
| Temperature Task | 3 |
| Process Task | 2 |
| Display Task | 1 |
| Logger Task | 1 |

Design Rationale:

- Data acquisition is time critical.
- Processing must occur quickly.
- Display is less critical.
- Logging is background activity.
- Watchdog has highest priority.

---

# 13. Fault Handling Strategy

## Sensor Failure

Detection:

- Invalid DHT11 response
- ADC out-of-range

Action:

- Log event
- Display sensor fault

---

## Queue Overflow

Detection:

Queue send failure.

Action:

- Log event
- Increment fault counter

---

## Task Starvation

Detection:

Watchdog timeout.

Action:

- Trigger recovery mechanism.

---

# 14. Future Enhancements

- Humidity Sensor
- Buzzer Alert
- CAN Communication
- MQTT Connectivity
- SD Card Logging
- LoRaWAN Integration
- Cloud Dashboard
- OTA Firmware Updates
- Power Saving Modes
- External Watchdog IC

---

# 15. Conclusion

The architecture follows industrial embedded software design principles by separating sensing, processing, display, monitoring, and logging into dedicated FreeRTOS tasks. The use of queues, task notifications, mutexes, watchdog supervision, and state-machine-based processing results in a scalable, maintainable, and fault-tolerant real-time monitoring system suitable for firmware engineering and embedded systems projects.
