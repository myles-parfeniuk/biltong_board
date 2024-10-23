//in house
#include "app_main.h"
//third party
#include "FreeRTOS.h"
#include "task.h"
#include "gpio.h"
#include "usart.h"

void task_test(void *arg);

TaskHandle_t task_test_hdl; 

int app_main()
{
    xTaskCreate(task_test, "Task Test", 128, NULL, 1, &task_test_hdl);

    vTaskStartScheduler();

    return 0; 
}


void task_test(void *arg)
{

    while(1)
    {
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}