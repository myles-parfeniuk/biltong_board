
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
#include "sh1122_fonts/sh1122_font_5x8_tf.h"
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
    uint16_t qp_cnt = (uint16_t)0U;

    // initialize serial backend for debug
    SerialService::init(&huart3);
    BB_LOGSUC(TAG, "****init()**** serial_svc success");

    // todo: move to backends/gui
    SH1122Oled oled(&hspi1);

    if (!oled.init())
        BB_LOGE(TAG, "****init()**** oled fail");
    else
        BB_LOGSUC(TAG, "****init()**** oled success...");

    // launch OPEEngine
    opee::OPEEngine_init();
    BB_LOGSUC(TAG, "****init()**** OPEEngine success...");

    // create device model
    static Device d;

    // create backends and populate with device model
    SwitchDriver switch_driver(d);
    TempRHDriver temp_rh_driver(d, &hi2c2, &hi2c1);
    HeatLampDriver heat_lamp_driver(d, &htim15, &hadc1);

    // initialize backends
    if (!switch_driver.init())
        BB_LOGE(TAG, "****init()**** switch_driver fail");
    else
        BB_LOGSUC(TAG, "****init()**** switch_driver success...");

    if (!temp_rh_driver.init())
        BB_LOGE(TAG, "****init()**** temp_rh_driver fail");
    else
        BB_LOGSUC(TAG, "****init()**** temp_rh_driver success...");

    if (!heat_lamp_driver.init())
        BB_LOGE(TAG, "****init() heat_lamp_driver fail");
    else
        BB_LOGSUC(TAG, "****init()**** heat_lamp_driver success...");

    d.heat_lamps.mains_hz.subscribe<8>(
            [](float mains_hz)
            {
                static constexpr const char* CB_TAG = "MainsFreq";
                BB_LOGI(CB_TAG, "%.4f(Hz)", mains_hz);
            });

    d.sensors.temperature.celsius.subscribe<16>(
            [](temp_data_t new_temp)
            {
                static constexpr const char* CB_TAG = "Temp";

                BB_LOGI(CB_TAG, "A: %li B: %li", new_temp.A, new_temp.B);
            });

    d.sensors.humidity.relative.subscribe<16>(
            [](rh_data_t new_rh)
            {
                static constexpr const char* CB_TAG = "RH";

                BB_LOGI(CB_TAG, "A: %li B: %li", new_rh.A, new_rh.B);
            });

    d.switches.enter.subscribe<16>(
            [&oled, &qp_cnt](SwitchEvent new_event)
            {
                static constexpr const char* CB_TAG = "EnterSwitch";

                switch (new_event)
                {
                    case SwitchEvent::quick_press:
                        BB_LOGSPEC(CB_TAG, "quick_press");
                        BB_LOGSUC(CB_TAG, "oled_count_up");
                        oled.load_font(sh1122_font_5x8_tf);
                        oled.clear_buffer();
                        oled.draw_string({0, 0}, SH1122PixIntens::level_7, "press cnt: %d | relay state: %s", qp_cnt,
                                d.heat_lamps.relay_closed.get() ? "closed" : "open");
                        oled.update_screen();
                        qp_cnt++;
                        break;

                    case SwitchEvent::long_press:
                        BB_LOGSPEC(CB_TAG, "long_press");
                        break;

                    case SwitchEvent::held:
                        BB_LOGSPEC(CB_TAG, "held");
                        break;

                    case SwitchEvent::release:
                        BB_LOGSPEC(CB_TAG, "release");
                        break;

                    default:

                        break;
                }
            });

    d.switches.down.subscribe<32>(
            [](SwitchEvent new_event)
            {
                static constexpr const char* CB_TAG = "DownSwitch";
                static uint8_t pct_intensity = 0U;

                switch (new_event)
                {
                    case SwitchEvent::quick_press:
                        BB_LOGSPEC(CB_TAG, "quick_press");
                        d.heat_lamps.intensity.set(pct_intensity);
                        pct_intensity += 1U;
                        if (pct_intensity > 100U)
                            pct_intensity = 0U;
                        break;

                    case SwitchEvent::long_press:
                        BB_LOGSPEC(CB_TAG, "long_press");
                        pct_intensity = 0U;
                        d.heat_lamps.intensity.set(pct_intensity);
                        break;

                    case SwitchEvent::held:
                        BB_LOGSPEC(CB_TAG, "held");
                        break;

                    case SwitchEvent::release:
                        BB_LOGSPEC(CB_TAG, "release");
                        break;

                    default:

                        break;
                }
            });

    d.switches.up.subscribe<32>(
            [&oled, &qp_cnt](SwitchEvent new_event)
            {
                static constexpr const char* CB_TAG = "UpSwitch";

                oled.load_font(sh1122_font_5x8_tf);
                oled.clear_buffer();

                switch (new_event)
                {
                    case SwitchEvent::quick_press:
                        BB_LOGSPEC(CB_TAG, "quick_press");
                        if (d.heat_lamps.relay_closed.get())
                        {
                            BB_LOGW(CB_TAG, "relay_disable");
                            d.heat_lamps.relay_closed.set(false);
                            oled.draw_string({0, 0}, SH1122PixIntens::level_7, "press cnt: %d | relay state: %s", qp_cnt, "open");
                        }
                        else
                        {
                            BB_LOGW(CB_TAG, "relay_enable");
                            d.heat_lamps.relay_closed.set(true);
                            oled.draw_string({0, 0}, SH1122PixIntens::level_7, "press cnt: %d | relay state: %s", qp_cnt, "closed");
                        }

                        oled.update_screen();

                        break;

                    case SwitchEvent::long_press:
                        BB_LOGSPEC(CB_TAG, "long_press");
                        break;

                    case SwitchEvent::held:
                        BB_LOGSPEC(CB_TAG, "held");
                        break;

                    case SwitchEvent::release:
                        BB_LOGSPEC(CB_TAG, "release");
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