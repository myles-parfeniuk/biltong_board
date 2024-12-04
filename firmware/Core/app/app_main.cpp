
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
#include "ButtonDriver.h"

const constexpr char* TAG = "Main";

TaskHandle_t task_idle_hdl = NULL;

void task_idle(void* arg);

extern "C" int app_main()
{
    xTaskCreate(task_idle, "IdleTask", 256, NULL, 1, &task_idle_hdl);
    vTaskStartScheduler();

    return 0;
}

void task_idle(void* arg)
{
    // Create device model
    Device d;
    
    // call constructors for modules

    static SH1122Oled oled(&hspi1);
    static ButtonDriver button(d);

    //static TempHumiditySensor th_sens_A(&hi2c2);


    //static TempHumiditySensor th_sens_B(&hi2c1);


    // initialize modules:
    if (!SerialService::init(&huart3))
        vTaskDelete(NULL); // self delete task and crash program on failure

    if (!CbHelper::init())
        vTaskDelete(NULL);


/*
    if (!th_sens_A.init())
       vTaskDelete(NULL);  
*/
 


/*
    if (!th_sens_B.init())
        vTaskDelete(NULL);
*/



    if (!oled.init())
        vTaskDelete(NULL);

    d.buttons.enter.subscribe([TAG](ButtonEvent new_button_event){
        switch (new_button_event) {
            case ButtonEvent::held:
                SerialService::print_log_ln(TAG, "button held");
            break;

            case ButtonEvent::long_press:
                SerialService::print_log_ln(TAG, "button PRESSED LONG");
            break;

            case ButtonEvent::quick_press:
                SerialService::print_log_ln(TAG, "quick pressed");
            break;

            default:
            break;
        }
    });


    while (1)
    {

        SerialService::print_log_ln(TAG, "Idling...");
        d.buttons.enter.set(ButtonEvent::held);
        vTaskDelay(1000UL / portTICK_PERIOD_MS);
        d.buttons.enter.set(ButtonEvent::quick_press);
        vTaskDelay(1000UL / portTICK_PERIOD_MS);
        d.buttons.enter.set(ButtonEvent::long_press);
        vTaskDelay(1000UL / portTICK_PERIOD_MS);
    }
}