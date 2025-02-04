#include "HeatLampDriver.h"

#define SET_HEAT_LAMP_RELAY_INACTIVE() HAL_GPIO_WritePin(PIN_HEAT_LAMP_EN.port, PIN_HEAT_LAMP_EN.num, GPIO_PIN_RESET)
#define SET_HEAT_LAMP_RELAY_ACTIVE() HAL_GPIO_WritePin(PIN_HEAT_LAMP_EN.port, PIN_HEAT_LAMP_EN.num, GPIO_PIN_SET)

HeatLampDriver::HeatLampDriver(Device& d, TIM_HandleTypeDef* hdl_heat_lamp_timer)
    : d(d)
    , hw_timer_heat_lamp(hdl_heat_lamp_timer)
    , triac(d, hw_timer_heat_lamp)
    , zero_cross(d, hw_timer_heat_lamp)
{
    // ensure relay is initially closed
    HAL_GPIO_WritePin(PIN_HEAT_LAMP_EN.port, PIN_HEAT_LAMP_EN.num, GPIO_PIN_RESET);
}

bool HeatLampDriver::init()
{

    OPEEngineRes_t op_success = OPEE_OK;

    op_success = d.heat_lamps.relay_closed.subscribe<8UL>(
            [this](bool relay_closed)
            {
                if (relay_closed)
                    SET_HEAT_LAMP_RELAY_ACTIVE();
                else
                    SET_HEAT_LAMP_RELAY_INACTIVE();
            });

    if (op_success != OPEE_OK)
        return false;

    op_success = d.heat_lamps.intensity.subscribe<16>(
            [this](uint8_t new_intensity)
            {
                uint32_t new_trig_ticks = intensity_2_triac_trig_ticks(new_intensity);
                triac.set_trig_ticks(new_trig_ticks);
            });

    if (op_success != OPEE_OK)
        return false;

    if (!triac.init())
        return false;

    if (!zero_cross.init())
        return false;

    if (!BBEventHandler::register_event_cb(BB_EVT_MAINS_HZ_CALC, mains_hz_event_cb, this))
        return false;

    return true;
}

uint32_t HeatLampDriver::intensity_2_triac_trig_ticks(uint8_t new_intensity)
{
    int32_t zx_period_us = d.heat_lamps.zx_period_us.get();
    int32_t new_trig_ticks = 0L;
    int32_t max_trig_ticks = 0L;
    int32_t min_trig_ticks = 0L;

    if (zx_period_us > 0L)
    {
        max_trig_ticks = zx_period_us - 1000L;
        min_trig_ticks = 1000L;
        new_trig_ticks = zx_period_us - (((new_intensity - 0UL) * (max_trig_ticks - min_trig_ticks)) / (100U - 0U) + min_trig_ticks);
        return static_cast<uint32_t>(new_trig_ticks);
    }
    else
    {
        BB_LOGE(TAG, "****intensity_2_triac_trig_ticks**** mains avg period uninit fail");
    }

    return HeatLampHWTimer::MAX_HEAT_LAMP_TIMER_TICKS;
}

void HeatLampDriver::mains_hz_event_cb(void* arg)
{
    HeatLampDriver* _driver = static_cast<HeatLampDriver*>(arg);
    int32_t zx_period_us = 0L;
    float mains_hz = 0.0f;

    zx_period_us = _driver->zero_cross.avg_window();
    mains_hz = _driver->zero_cross.hz_calc(zx_period_us);
    _driver->d.heat_lamps.zx_period_us.set(zx_period_us);
    _driver->d.heat_lamps.mains_hz.set(mains_hz);
}
