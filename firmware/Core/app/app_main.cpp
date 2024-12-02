
#include "app_main.h"

// std library includes
#include <stdio.h>
#include <stdint.h>
// third party includes
#include "FreeRTOS.h"
#include "task.h"
// cube mx inclues
#include "gpio.h"
// in house includes
#include "SerialService.h"
#include "TempHumiditySensor.h"

void task_test(void* arg);

TaskHandle_t task_test_hdl;

extern "C" int app_main()
{
    // call constructors for non-static modules
    // TempHumiditySensor th_sens_1(&hi2c1);

    // initialize modules:
    if (!SerialService::init(&huart3))
        return 0;

    /*if(!th_sens_1.init())
        return 0;*/

    xTaskCreate(task_test, "TaskTest", 256, NULL, 1, &task_test_hdl);

    vTaskStartScheduler();

    return 0;
}

void task_test(void* arg)
{
    const constexpr char* TAG = "TaskTest";
    uint32_t counter = 0;

    while (1)
    {
        SerialService::print_log_ln(TAG, "Counting up test: %ld", counter);
        counter++;
        vTaskDelay(1000UL / portTICK_PERIOD_MS);
    }
}