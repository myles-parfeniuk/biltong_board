#include "ZeroCross.h"
#include <stm32g070xx.h>

ZeroCross::ZeroCross(Device& d, HeatLampHWTimer& hw_timer_heat_lamp)
    : d(d)
    , hw_timer_heat_lamp(hw_timer_heat_lamp)
{
}

bool ZeroCross::init()
{
    ISRCbDispatch::register_zero_cross_ISR_cb(zero_cross_ISR_cb, this);
    sample_window = mains_hz_window_a;
    proc_window = mains_hz_window_b;

    return true;
}

int32_t ZeroCross::avg_window()
{
    int32_t result = 0UL;

    // sum the window
    for (size_t i = 0UL; i < MAINS_HZ_WINDOW_SZ; i++)
        result += proc_window[i];

    // take the average
    result /= MAINS_HZ_WINDOW_SZ;

    return result;
}

float ZeroCross::hz_calc(int32_t window_avg)
{
    float result = static_cast<float>(window_avg);
    // account for 2 ISR calls == 1 sin wave
    result *= 2.0f;
    // convert to freq (Ttimer/Tavg_sin_wave)
    result = static_cast<float>(HeatLampHWTimer::HEAT_LAMP_TIMER_TICK_FREQ_HZ) / result;

    return result;
}

uint32_t ZeroCross::grab_sample_and_reset_zx_timer(ZeroCross* _zx)
{
    uint32_t sample = _zx->hw_timer_heat_lamp.get_ticks();
    _zx->hw_timer_heat_lamp.restart();

    return sample;
}

void ZeroCross::mv_hz_smpl_to_window(ZeroCross* _zx, const uint32_t sample)
{
    _zx->sample_window[_zx->window_pos] = sample;
    _zx->window_pos++;
}

void ZeroCross::window_swp(ZeroCross* _zx)
{
    if (_zx->sample_window == _zx->mains_hz_window_a)
    {
        _zx->sample_window = _zx->mains_hz_window_b;
        _zx->proc_window = _zx->mains_hz_window_a;
    }
    else
    {
        _zx->sample_window = _zx->mains_hz_window_a;
        _zx->proc_window = _zx->mains_hz_window_b;
    }
    _zx->window_pos = 0U;
}

void ZeroCross::hz_calc_evt(ZeroCross* _zx)
{
    BBEventHandler::send_event_to_handler_from_ISR(BB_EVT_MAINS_HZ_CALC);
}

void ZeroCross::zero_cross_ISR_cb(void* arg)
{
    ZeroCross* _zx = static_cast<ZeroCross*>(arg);
    const ZxSampleState state = _zx->state;
    static uint32_t mains_hz_sample = 0UL;

    if (state == ZxSampleState::SAMPLING)
        mains_hz_sample = grab_sample_and_reset_zx_timer(_zx);

    switch (state)
    {
        case ZxSampleState::INACTIVE:
            _zx->state = ZxSampleState::START;
            break;

        case ZxSampleState::START:
            _zx->hw_timer_heat_lamp.restart();
            _zx->state = ZxSampleState::SAMPLING;
            break;

        case ZxSampleState::SAMPLING:

            if (_zx->window_pos > MAINS_HZ_WINDOW_SZ)
            {
                window_swp(_zx);
                mv_hz_smpl_to_window(_zx, mains_hz_sample);
                hz_calc_evt(_zx);
            }
            else
            {
                mv_hz_smpl_to_window(_zx, mains_hz_sample);
            }

        default:
            break;
    }
}