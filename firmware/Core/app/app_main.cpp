
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
#include "sh1122_fonts/sh1122_font_10x20_me.h"

const constexpr char* TAG = "Main";

TaskHandle_t task_idle_hdl = NULL;

void task_idle(void* arg);

extern "C" int app_main()
{
    xTaskCreate(task_idle, "IdleTask", 128 * 3, NULL, 1, &task_idle_hdl);
    vTaskStartScheduler();

    return 0;
}

void task_idle(void* arg)
{
    opee::OPEEngine_init();

    Device d;
    SH1122Oled oled(&hspi1);

    SerialService::init(&huart3);
    oled.init();

    ButtonDriver button_driver(d);
    button_driver.init();

    d.buttons.enter.subscribe<32>(
            [&oled](ButtonEvent new_event)
            {
                static constexpr const char* CB_TAG = "EnterButton";
                static uint16_t qp_cnt = 0UL;

                switch (new_event)
                {
                    case ButtonEvent::quick_press:
                        SerialService::print_log_ln(CB_TAG, "quick_press");
                        oled.load_font(sh1122_font_10x20_me);
                        oled.clear_buffer();
                        oled.draw_string({0, 0}, SH1122PixIntens::level_7, "%d", qp_cnt);
                        oled.update_screen();
                        qp_cnt++;
                        break;

                    case ButtonEvent::long_press:
                        SerialService::print_log_ln(CB_TAG, "long_press");
                        break;

                    case ButtonEvent::held:
                        SerialService::print_log_ln(CB_TAG, "held");
                        break;

                    case ButtonEvent::release:
                        SerialService::print_log_ln(CB_TAG, "release");
                        break;

                    default:

                        break;
                }
            });

    d.buttons.down.subscribe<32>(
            [](ButtonEvent new_event)
            {
                static constexpr const char* CB_TAG = "DownButton";

                switch (new_event)
                {
                    case ButtonEvent::quick_press:
                        SerialService::print_log_ln(CB_TAG, "quick_press");
                        break;

                    case ButtonEvent::long_press:
                        SerialService::print_log_ln(CB_TAG, "long_press");
                        break;

                    case ButtonEvent::held:
                        SerialService::print_log_ln(CB_TAG, "held");
                        break;

                    case ButtonEvent::release:
                        SerialService::print_log_ln(CB_TAG, "release");
                        break;

                    default:

                        break;
                }
            });

    d.buttons.up.subscribe<32>(
            [](ButtonEvent new_event)
            {
                static constexpr const char* CB_TAG = "UpButton";

                switch (new_event)
                {
                    case ButtonEvent::quick_press:
                        SerialService::print_log_ln(CB_TAG, "quick_press");
                        break;

                    case ButtonEvent::long_press:
                        SerialService::print_log_ln(CB_TAG, "long_press");
                        break;

                    case ButtonEvent::held:
                        SerialService::print_log_ln(CB_TAG, "held");
                        break;

                    case ButtonEvent::release:
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