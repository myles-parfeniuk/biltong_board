#include "ZeroCross.h"
#include <stm32g070xx.h>

ZeroCross::ZeroCross(Device& d, TIM_HandleTypeDef* hdl_zx_timer, EventGroupHandle_t& evt_grp_lamp_ctrl_hdl)
    : d(d)
    , hdl_zx_timer(hdl_zx_timer)
    , evt_grp_lamp_ctrl_hdl(evt_grp_lamp_ctrl_hdl)

{
}

bool ZeroCross::init()
{
    OPEEngineRes_t op_success = OPEE_OK;

    ISRCbDispatch::register_zero_cross_ISR_cb(zero_cross_ISR_cb, this);
    sample_window = mains_hz_window_a;
    proc_window = mains_hz_window_b;

    op_success = d.heat_lamps.intensity.subscribe<16>(
            [this](uint8_t new_intensity)
            {
                float scale_factor = static_cast<float>(new_intensity) / 100.0f;
                int32_t mains_period_us = d.heat_lamps.mains_period_us.get();
                uint32_t oc_val = 0UL;

                if (mains_period_us > 0L)
                {
                    oc_val = static_cast<uint32_t>(static_cast<float>(mains_period_us) * scale_factor);

                    SerialService::LOG_ln<BB_LL_SUCCESS>(TAG, "****triac trig time set**** %ld/%ldus success", oc_val, mains_period_us);
                }
                else
                {
                    SerialService::LOG_ln<BB_LL_ERROR>(TAG, "****triac trig time set**** mains avg period uninit fail");
                }
            });

    return (op_success == OPEE_OK);
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
    result /= 2.0f;
    // convert to freq (Ttimer/Tavg_sin_wave)
    result = static_cast<float>(ZX_TIMER_TICK_FREQ_HZ) / result;

    return result;
}

uint32_t ZeroCross::grab_sample_and_reset_zx_timer(ZeroCross* _zx)
{
    uint32_t sample = TIM15->CNT;
    TIM15->CNT &= 0UL;
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
    BaseType_t higher_priority_task_awoken = pdFALSE;

    // notify ctrl task
    xEventGroupSetBitsFromISR(_zx->evt_grp_lamp_ctrl_hdl, EVT_GRP_LAMP_CTRL_HZ_CALC, &higher_priority_task_awoken);
    portYIELD_FROM_ISR(higher_priority_task_awoken);
}

void ZeroCross::zero_cross_ISR_cb(void* arg)
{
    ZeroCross* _zx = static_cast<ZeroCross*>(arg);
    const ZxSampleState state = _zx->state;
    static uint32_t mains_hz_sample = 0UL;

    if (state == ZxSampleState::DONE_SAMPLE)
        mains_hz_sample = grab_sample_and_reset_zx_timer(_zx);

    switch (state)
    {
        case ZxSampleState::INACTIVE:
            _zx->state = ZxSampleState::START;
            break;

        case ZxSampleState::START:
            HAL_TIM_Base_Start_IT(_zx->hdl_zx_timer);
            _zx->state = ZxSampleState::DEBOUNCE;
            break;

        case ZxSampleState::DEBOUNCE:
            if (TIM15->CNT > DEBOUNCE_THRESHOLD)
                _zx->state = ZxSampleState::DONE_SAMPLE;
            break;

        case ZxSampleState::DONE_SAMPLE:

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
            _zx->state = ZxSampleState::DEBOUNCE;

        default:
            break;
    }
}
