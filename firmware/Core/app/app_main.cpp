
#include "app_main.h"

// std library includes
#include <stdint.h>
// third party includes
#include "FreeRTOS.h"
#include "task.h"
// in house includes
#include "bb_task_defs.h"
#include "SerialService.h"
#include "SH1122Oled.h"
#include "sh1122_fonts/sh1122_font_8x13B_tf.h"
#include "Device.h"
#include "SwitchDriver.h"
#include "TempRHDriver.h"
#include "HeatLampDriver.h"

const constexpr char* TAG = "Main";

StaticTask_t task_idle_hdl;
StackType_t task_idle_stk[BB_IDLE_TSK_SZ] = {0UL};

void task_idle(void* arg);

extern "C" int app_main()
{

    xTaskCreateStatic(task_idle, "bbIdleTsk", BB_IDLE_TSK_SZ, NULL, 1, task_idle_stk, &task_idle_hdl);
    vTaskStartScheduler();

    return 0;
}

void task_idle(void* arg)
{
    // initialize serial backend for debug
    SerialService::init(&huart3);
    SerialService::LOG_ln<BB_LL_SUCCESS>(TAG, "****init()**** serial_svc success");

    // todo: move to backends/gui
    SH1122Oled oled(&hspi1);

    if (!oled.init())
        SerialService::LOG_ln<BB_LL_ERROR>(TAG, "****init()**** oled fail");
    else
        SerialService::LOG_ln<BB_LL_SUCCESS>(TAG, "****init()**** oled success...");

    // launch OPEEngine
    opee::OPEEngine_init();
    SerialService::LOG_ln<BB_LL_SUCCESS>(TAG, "****init()**** OPEEngine success...");

    // create device model
    static Device d;

    // create backends and populate with device model
    SwitchDriver switch_driver(d);
    TempRHDriver temp_rh_driver(d, &hi2c2, &hi2c1);
    HeatLampDriver heat_lamp_driver(d, &htim15);

    // initialize backends
    if (!switch_driver.init())
        SerialService::LOG_ln<BB_LL_ERROR>(TAG, "****init()**** switch_driver fail");
    else
        SerialService::LOG_ln<BB_LL_SUCCESS>(TAG, "****init()**** switch_driver success...");

    if (!temp_rh_driver.init())
        SerialService::LOG_ln<BB_LL_ERROR>(TAG, "****init()**** temp_rh_driver fail");
    else
        SerialService::LOG_ln<BB_LL_SUCCESS>(TAG, "****init()**** temp_rh_driver success...");

    if (!heat_lamp_driver.init())
        SerialService::LOG_ln<BB_LL_ERROR>(TAG, "****init() heat_lamp_driver fail");
    else
        SerialService::LOG_ln<BB_LL_SUCCESS>(TAG, "****init()**** heat_lamp_driver success...");

    OPEEngineRes_t res;

    res = d.heat_lamps.mains_hz.subscribe<8>(
            [](float mains_hz)
            {
                static constexpr const char* CB_TAG = "MainsFreq";
                SerialService::LOG_ln<BB_LL_INFO>(CB_TAG, "%.2f(Hz)", mains_hz);
            });

    res = d.sensors.temperature.celsius.subscribe<16>(
            [](temp_data_t new_temp)
            {
                static constexpr const char* CB_TAG = "Temp";

                SerialService::LOG_ln<BB_LL_INFO>(CB_TAG, "A: %li B: %li", new_temp.A, new_temp.B);
            });

    res = d.sensors.humidity.relative.subscribe<16>(
            [](rh_data_t new_rh)
            {
                static constexpr const char* CB_TAG = "RH";

                SerialService::LOG_ln<BB_LL_INFO>(CB_TAG, "A: %li B: %li", new_rh.A, new_rh.B);
            });

    d.switches.enter.subscribe<32>(
            [&oled](SwitchEvent new_event)
            {
                static constexpr const char* CB_TAG = "EnterSwitch";
                static uint16_t qp_cnt = 0UL;

                switch (new_event)
                {
                    case SwitchEvent::quick_press:
                        SerialService::LOG_ln<BB_LL_SUCCESS>(CB_TAG, "oled_count_up");
                        oled.load_font(sh1122_font_8x13B_tf);
                        oled.clear_buffer();
                        oled.draw_string({0, 0}, SH1122PixIntens::level_7, "count: %d oled state: %s", qp_cnt,
                                d.heat_lamps.relay_closed.get() ? "closed" : "open");
                        oled.update_screen();
                        qp_cnt++;
                        break;

                    case SwitchEvent::long_press:
                        SerialService::LOG_ln<BB_LL_INFO>(CB_TAG, "long_press");
                        break;

                    case SwitchEvent::held:
                        SerialService::LOG_ln<BB_LL_INFO>(CB_TAG, "held");
                        break;

                    case SwitchEvent::release:
                        SerialService::LOG_ln<BB_LL_INFO>(CB_TAG, "release");
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
                        SerialService::LOG_ln<BB_LL_INFO>(CB_TAG, "quick_press");
                        break;

                    case SwitchEvent::long_press:
                        SerialService::LOG_ln<BB_LL_INFO>(CB_TAG, "long_press");
                        break;

                    case SwitchEvent::held:
                        SerialService::LOG_ln<BB_LL_INFO>(CB_TAG, "held");
                        break;

                    case SwitchEvent::release:
                        SerialService::LOG_ln<BB_LL_INFO>(CB_TAG, "release");
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

                        if (d.heat_lamps.relay_closed.get())
                        {
                            SerialService::LOG_ln<BB_LL_WARNING>(CB_TAG, "relay_enable");
                            d.heat_lamps.relay_closed.set(false);
                        }
                        else
                        {
                            SerialService::LOG_ln<BB_LL_WARNING>(CB_TAG, "relay_disable");
                            d.heat_lamps.relay_closed.set(true);
                        }

                        break;

                    case SwitchEvent::long_press:
                        SerialService::LOG_ln<BB_LL_INFO>(CB_TAG, "long_press");
                        break;

                    case SwitchEvent::held:
                        SerialService::LOG_ln<BB_LL_INFO>(CB_TAG, "held");
                        break;

                    case SwitchEvent::release:
                        SerialService::LOG_ln<BB_LL_INFO>(CB_TAG, "release");
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