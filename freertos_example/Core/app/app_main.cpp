//in house
#include "app_main.h"
//third party
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "gpio.h"
#include "timers.h"
#include <cstring>
#include "usart.h"
#include "semphr.h"
#include "task_class.h"

#define MAX_STR_LEN 50

void red_led_task(void* arg);
void blue_led_task(void* arg);
void send_uart(void* arg);
void read_button_state(void* arg);
void semaphoreExample(void* arg);

TaskHandle_t task_test_hdl;
QueueHandle_t queue;
TimerHandle_t debounceTimer = NULL;
SemaphoreHandle_t xSemaphore;

bool buttonState;
bool debounceFlag;

typedef struct {
    uint16_t pin_number;
    uint16_t normal_delay_time;
    uint16_t short_delay_time;
} BlinkingTaskParams;

typedef struct {
    const char* messages[3];
    uint8_t message_count;
} UartMessageCycle;

/*


*/
void vDebounceCallback(TimerHandle_t debounceTimer)
{
    debounceFlag = false;
}

QueueHandle_t queueSetup()
{
    QueueHandle_t newQueue = xQueueCreate(5, sizeof(buttonState));

    const char* str = (newQueue == NULL) ?  "failed in creating queue\n\r" : "Queue created successfully\n\r";
    HAL_UART_Transmit(&huart4, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);

    return newQueue;
}


int app_main()
{
    /* Instantiate data */
    static BlinkingTaskParams blink_params = 
    {
        .pin_number = GPIO_PIN_7, 
        .normal_delay_time = 1000, 
        .short_delay_time = 100
    };

    static UartMessageCycle uart_cycle = 
    {
        .messages = {
            "first\n\r",
            "second\n\r",
            "third\n\r"
        },
        .message_count = 0
    };
    debounceFlag = false;
    buttonState = false;

    /* Setup queue */
    queue = queueSetup();
    
    /* Create tasks */
    xTaskCreate(blue_led_task, "Blue LED Task", 128, (void*)&blink_params, 1, &task_test_hdl);
    xTaskCreate(red_led_task, "Red LED Task", 128, (void*)&blink_params, 1, &task_test_hdl);
    xTaskCreate(send_uart, "UART transmission Task", 128, (void*)&uart_cycle, 1, &task_test_hdl);
    xTaskCreate(semaphoreExample, "Example of task using semaphore", 128, NULL, 1, &task_test_hdl);

    /* Create timer */  
    debounceTimer = xTimerCreate("debounce timer", pdMS_TO_TICKS(150), pdFALSE, NULL, vDebounceCallback);

    vTaskStartScheduler();

    return 0; 
}

void send_uart(void* arg)
{
    UartMessageCycle* poopoo = (UartMessageCycle*)arg;
    while(1)
    {
        vTaskDelay(2000/portTICK_PERIOD_MS);
        HAL_UART_Transmit(&huart4, (uint8_t*)poopoo->messages[poopoo->message_count], strlen(poopoo->messages[poopoo->message_count]), HAL_MAX_DELAY);
        poopoo->message_count++;
        if(poopoo->message_count >= 3)
            poopoo->message_count = 0;
    }
}

/* toggling button led */
void red_led_task(void* arg)
{
    BlinkingTaskParams* params = (BlinkingTaskParams*)arg; //cast void pointer to correct type
    char* fail_msg = "fail in receiving from queue\n\r";
    char* success_msg = "Successfully received from queue\n\r";
    while(1)
    {
        if(xQueueReceive(queue, &buttonState, portMAX_DELAY) != pdTRUE)
        {
            HAL_UART_Transmit(&huart4, (uint8_t*)fail_msg, strlen(fail_msg), HAL_MAX_DELAY);
        }
        else
        {
            if(!buttonState) //button is not pressed (exti interrupt is on rising/falling edge)
            {
                HAL_GPIO_TogglePin(GPIOB, RED_LED);
            }
            debounceFlag = true;
            HAL_UART_Transmit(&huart4, (uint8_t*)success_msg, strlen(success_msg), HAL_MAX_DELAY);
        }
    }
}

/* blinking led */
void blue_led_task(void* arg)
{
    BlinkingTaskParams* params = (BlinkingTaskParams*)arg;
    while(1)
    {
        if(!buttonState) //button is not pressed
        {
            HAL_GPIO_TogglePin(GPIOB, BLUE_LED);
            vTaskDelay(params->normal_delay_time/portTICK_PERIOD_MS);
        }
        else
        {
            HAL_GPIO_TogglePin(GPIOB, BLUE_LED);
            vTaskDelay(params->short_delay_time/portTICK_PERIOD_MS);
        }

    }
}

/* task using semaphores */
void semaphoreExample(void* arg)
{
    char* msg = "inside the semaphore task\n\r";
    xSemaphore = xSemaphoreCreateBinary();
    while(1)
    {
        if ( xSemaphore != NULL )
        {
            if (xSemaphoreTake(xSemaphore, 10) == pdTRUE)
            {
                HAL_GPIO_TogglePin(GPIOB, RED_LED);
                HAL_UART_Transmit(&huart4, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
            }
        }
    }
}