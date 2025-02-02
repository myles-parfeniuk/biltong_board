#include "ZeroCross.h"
#include <stm32g070xx.h>

#define SET_TRIAC_TRIGGER_INACTIVE() HAL_GPIO_WritePin(PIN_TRIAC_TRIG.port, PIN_TRIAC_TRIG.num, GPIO_PIN_SET)

ZeroCross::ZeroCross(Device& d, TIM_HandleTypeDef* hdl_zx_timer, EventGroupHandle_t& evt_grp_lamp_ctrl_hdl)
    : d(d)
    , hdl_zx_timer(hdl_zx_timer)
    , evt_grp_lamp_ctrl_hdl(evt_grp_lamp_ctrl_hdl)

{
}

bool ZeroCross::init()
{
    OPEEngineRes_t op_success = OPEE_OK;

    set_zx_timer_triac_trig_ch(MAX_ZX_TIMER_TICKS); // should never execute ccr1 isr until set by user
    SET_TRIAC_TRIGGER_INACTIVE();
    ISRCbDispatch::register_zero_cross_ISR_cb(zero_cross_ISR_cb, this);
    ISRCbDispatch::register_triac_trig_ISR_cb(triac_trig_ISR_cb, this);
    sample_window = mains_hz_window_a;
    proc_window = mains_hz_window_b;

    op_success = d.heat_lamps.intensity.subscribe<16>(
            [this](uint8_t new_intensity)
            {
                uint32_t new_trig_ticks = intensity2ticks(new_intensity);
                set_triac_trig_ticks(new_trig_ticks);
            });

    return (op_success == OPEE_OK);
}

uint32_t ZeroCross::intensity2ticks(uint8_t new_intensity)
{
    int32_t zx_period_us = d.heat_lamps.zx_period_us.get();
    int32_t new_trig_ticks = 0L;
    int32_t max_trig_ticks = 0L;
    int32_t min_trig_ticks = 0L;

    if (zx_period_us > 0L)
    {
        max_trig_ticks = zx_period_us - 1000L;
        min_trig_ticks = 1000L; 
        new_trig_ticks = zx_period_us -  (((new_intensity - 0UL) * (max_trig_ticks - min_trig_ticks)) / (100U - 0U) + min_trig_ticks);
        return static_cast<uint32_t>(new_trig_ticks);
    }
    else
    {
        BB_LOGE(TAG, "****intensity2ticks**** mains avg period uninit fail");
    }

    return MAX_ZX_TIMER_TICKS;
}

bool ZeroCross::set_triac_trig_ticks(uint32_t new_trig_ticks)
{
    int32_t zx_period_us = d.heat_lamps.zx_period_us.get();

    if (new_trig_ticks <= (zx_period_us - TRIAC_TRIGGING_TIME_US))
    {
        set_zx_timer_triac_trig_ch(new_trig_ticks);
        BB_LOGSUC(TAG, "****set_triac_trig_ticks**** new triag trig time: %ld/%d", new_trig_ticks, zx_period_us);

        return true;
    }
    else
    {
        set_zx_timer_triac_trig_ch(MAX_ZX_TIMER_TICKS);
        BB_LOGW(TAG, "****set_triac_trig_ticks**** tric dimmer disabled");
        return false;
    }
}

void ZeroCross::set_zx_timer_triac_trig_ch(uint32_t oc_ticks)
{
    HAL_TIM_OC_Stop(hdl_zx_timer, TIM_CHANNEL_1);
    triac_triggering = false; 
    SET_TRIAC_TRIGGER_INACTIVE();
    TIM15->CCR1 = oc_ticks;
    state = ZxSampleState::START;
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
    result = static_cast<float>(ZX_TIMER_TICK_FREQ_HZ) / result;

    return result;
}

uint32_t ZeroCross::grab_sample_and_reset_zx_timer(ZeroCross* _zx)
{
    HAL_TIM_OC_Stop_IT(_zx->hdl_zx_timer, TIM_CHANNEL_1);
    uint32_t sample = TIM15->CNT;
    TIM15->CNT &= 0UL;
    HAL_TIM_OC_Start_IT(_zx->hdl_zx_timer, TIM_CHANNEL_1);

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

void ZeroCross::triac_trig_ISR_cb(void* arg)
{
    ZeroCross* _zx = static_cast<ZeroCross*>(arg);

    HAL_GPIO_TogglePin(PIN_TRIAC_TRIG.port, PIN_TRIAC_TRIG.num);

    if (!_zx->triac_triggering)
        TIM15->CCR1 += TRIAC_TRIGGING_TIME_US;
    else
        TIM15->CCR1 -= TRIAC_TRIGGING_TIME_US;

    _zx->triac_triggering = !_zx->triac_triggering;
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
            TIM15->CNT &= 0UL;
            HAL_TIM_OC_Start_IT(_zx->hdl_zx_timer, TIM_CHANNEL_1);
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
