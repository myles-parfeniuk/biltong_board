
#include "app_main.h"

// std library includes
#include <stdio.h>
#include <stdint.h>
// third party includes
#include "FreeRTOS.h"
#include "task.h"
// in house includes
#include "SerialService.h"
#include "SH1122Oled.h"
#include "sh1122_fonts/sh1122_font_10x20_me.h"
#include "Device.h"
#include "SwitchDriver.h"
#include "TempRHDriver.h"
#include "HeatLampDriver.h"



const constexpr char* TAG = "Main";

TaskHandle_t task_idle_hdl = NULL;

void task_idle(void* arg);

extern "C" int app_main()
{
    xTaskCreate(task_idle, "bbIdleTsk", 128 * 4, NULL, 1, &task_idle_hdl);
    vTaskStartScheduler();

    return 0;
}

void task_idle(void* arg)
{
    // initialize serial backend for debug
    SerialService::init(&huart3);
    // todo: move to backends/gui
    SH1122Oled oled(&hspi1);
    oled.init();

    // launch OPEEngine
    opee::OPEEngine_init();
    // create device model
    static Device d;
    // create backends and populate with device model
    SwitchDriver switch_driver(d);
    TempRHDriver temp_rh_driver(d, &hi2c2, &hi2c1);
    HeatLampDriver heat_lamp_driver(d);
    // initialize backends
    switch_driver.init();
    temp_rh_driver.init();

    d.sensors.temperature.celsius.subscribe<16>(
            [](temp_data_t new_temp)
            {
                static constexpr const char* CB_TAG = "Temp";

                SerialService::print_log_ln(CB_TAG, "Temp A: %li TempB: %li", new_temp.A, new_temp.B);
            });

    d.switches.enter.subscribe<32>(
            [&oled](SwitchEvent new_event)
            {
                static constexpr const char* CB_TAG = "EnterSwitch";
                static uint16_t qp_cnt = 0UL;

                switch (new_event)
                {
                    case SwitchEvent::quick_press:
                        SerialService::print_log_ln(CB_TAG, "quick_press");
                        oled.load_font(sh1122_font_10x20_me);
                        oled.clear_buffer();
                        oled.draw_string({0, 0}, SH1122PixIntens::level_7, "%d", qp_cnt);
                        oled.update_screen();
                        qp_cnt++;
                        break;

                    case SwitchEvent::long_press:
                        SerialService::print_log_ln(CB_TAG, "long_press");
                        break;

                    case SwitchEvent::held:
                        SerialService::print_log_ln(CB_TAG, "held");
                        break;

                    case SwitchEvent::release:
                        SerialService::print_log_ln(CB_TAG, "release");
                        break;

                    default:

                        break;
                }
            });

    d.switches.down.subscribe<32>(
            [](SwitchEvent new_event)
            {
                static constexpr const char* CB_TAG = "DownSwitch";

                switch (new_event)
                {
                    case SwitchEvent::quick_press:
                        SerialService::print_log_ln(CB_TAG, "quick_press");
                        break;

                    case SwitchEvent::long_press:
                        SerialService::print_log_ln(CB_TAG, "long_press");
                        break;

                    case SwitchEvent::held:
                        SerialService::print_log_ln(CB_TAG, "held");
                        break;

                    case SwitchEvent::release:
                        SerialService::print_log_ln(CB_TAG, "release");
                        break;

                    default:

                        break;
                }
            });

    d.switches.up.subscribe<32>(
            [](SwitchEvent new_event)
            {
                static constexpr const char* CB_TAG = "UpSwitch";

                switch (new_event)
                {
                    case SwitchEvent::quick_press:
                        SerialService::print_log_ln(CB_TAG, "quick_press");
                        break;

                    case SwitchEvent::long_press:
                        SerialService::print_log_ln(CB_TAG, "long_press");
                        break;

                    case SwitchEvent::held:
                        SerialService::print_log_ln(CB_TAG, "held");
                        break;

                    case SwitchEvent::release:
                        SerialService::print_log_ln(CB_TAG, "release");
                        break;

                    default:

                        break;
                }
            });

    while (1)
    {
        vTaskDelay(100UL / portTICK_PERIOD_MS);
    }
}