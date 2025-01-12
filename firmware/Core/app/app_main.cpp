
#include "app_main.h"

// std library includes
#include <stdio.h>
#include <stdint.h>
// third party includes
#include "FreeRTOS.h"
#include "task.h"
// in house includes
#include "SerialService.h"
#include "TempHumiditySensor.h"
#include "SH1122Oled.h"
#include "Device.h"
#include "OPEEngineTestSuite.h"
#include "ButtonDriver.h"

const constexpr char* TAG = "Main";

TaskHandle_t task_idle_hdl = NULL;

void task_idle(void* arg);

extern "C" int app_main()
{
    xTaskCreate(task_idle, "IdleTask", 256*4, NULL, 1, &task_idle_hdl);
    vTaskStartScheduler();

    return 0;
}

void task_idle(void* arg)
{
    SerialService::init(&huart3);

    OPEEngineTestSuite::run_all_tests();


    //opee::OPEEngine_init();

    //Device d;

    while (1)
    {
        vTaskDelay(100UL / portTICK_PERIOD_MS);
    }
}