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

    op_success = d.heat_lamps.triac_on_time_pct.subscribe<16>(
            [this](uint8_t new_on_time_pct)
            {
                uint32_t new_trig_ticks = triac.on_time_pct_2_ticks(new_on_time_pct);
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
