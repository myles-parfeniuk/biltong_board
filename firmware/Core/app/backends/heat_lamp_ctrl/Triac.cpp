#include "Triac.h"

#define SET_TRIAC_TRIGGER_INACTIVE() HAL_GPIO_WritePin(PIN_TRIAC_TRIG.port, PIN_TRIAC_TRIG.num, GPIO_PIN_SET)
#define TOGGLE_TRIAC_TRIGGER() HAL_GPIO_TogglePin(PIN_TRIAC_TRIG.port, PIN_TRIAC_TRIG.num)

Triac::Triac(Device& d, HeatLampHWTimer& hw_timer_heat_lamp)
    : d(d)
    , hw_timer_heat_lamp(hw_timer_heat_lamp)
{
}

bool Triac::init()
{

    SET_TRIAC_TRIGGER_INACTIVE();
    ISRCbDispatch::register_triac_trig_ISR_cb(triac_trig_ISR_cb, this);
    hw_timer_heat_lamp.set_triac_trig_OC_ch(HeatLampHWTimer::MAX_HEAT_LAMP_TIMER_TICKS); // should never execute ccr1 isr until set by user

    return true;
}

uint32_t Triac::on_time_pct_2_ticks(uint8_t on_time_pct)
{
    const constexpr uint32_t PCT_MIN = 1UL;
    const constexpr uint32_t PCT_MAX = 100UL;
    const constexpr uint32_t TICKS_MIN = (40UL * TRIAC_TRIGGING_TIME_US);

    int32_t zx_period_us = d.heat_lamps.zx_period_us.get();
    const uint32_t ticks_max = zx_period_us;
    uint32_t trig_ticks = 0UL;

    if (on_time_pct == 0)
    {
        trig_ticks = HeatLampHWTimer::MAX_HEAT_LAMP_TIMER_TICKS;
    }
    else
    {
        trig_ticks = (on_time_pct - PCT_MIN) * (ticks_max - TICKS_MIN) / (PCT_MAX - PCT_MIN) + TICKS_MIN;
        trig_ticks = zx_period_us - trig_ticks;
    }

    BB_LOGSC(TAG, "****on_time_pct_2_ticks**** new triac trig time pct: %u delay: %lu/%ld", on_time_pct, trig_ticks, zx_period_us);
    return trig_ticks;
}

bool Triac::set_trig_ticks(uint32_t new_trig_ticks)
{
    trig_ticks = new_trig_ticks;
    hw_timer_heat_lamp.set_triac_trig_OC_ch(trig_ticks);
}

void Triac::triac_trig_ISR_cb(void* arg)
{
    Triac* _triac = static_cast<Triac*>(arg);

    TOGGLE_TRIAC_TRIGGER();

    if (_triac->triggering)
        _triac->trig_ticks -= TRIAC_TRIGGING_TIME_US;
    else
        _triac->trig_ticks += TRIAC_TRIGGING_TIME_US;

    _triac->hw_timer_heat_lamp.set_triac_trig_OC_ch(_triac->trig_ticks);
    _triac->triggering = !_triac->triggering;
}