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

bool Triac::set_trig_ticks(uint32_t new_trig_ticks)
{
    int32_t zx_period_us = d.heat_lamps.zx_period_us.get();

    hw_timer_heat_lamp.stop();
    triggering = false;
    SET_TRIAC_TRIGGER_INACTIVE();

    if (new_trig_ticks <= (zx_period_us - TRIAC_TRIGGING_TIME_US))
    {
        trig_ticks = new_trig_ticks; 
        BB_LOGSC(TAG, "****set_trig_ticks**** new triag trig time: %ld/%d", new_trig_ticks, zx_period_us);
    }
    else
    {
        trig_ticks = HeatLampHWTimer::MAX_HEAT_LAMP_TIMER_TICKS;
        BB_LOGW(TAG, "****set_trig_ticks**** tric dimmer disabled");
    }

    hw_timer_heat_lamp.set_triac_trig_OC_ch(trig_ticks);

    if(!hw_timer_heat_lamp.start())
        return false;
    else
        return true; 
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