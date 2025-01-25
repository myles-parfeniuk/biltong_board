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
    ISRCbDispatch::register_zero_cross_ISR_cb(zero_cross_ISR_cb, this);
    sample_window = mains_hz_window_a;
    proc_window = mains_hz_window_b;
    return true;
}

uint16_t ZeroCross::hz_calc()
{
    uint32_t result = 0UL;

    // sum the window
    for (size_t i = 0UL; i < MAINS_HZ_WINDOW_SZ; i++)
        result += proc_window[i];

    // take the average
    result /= MAINS_HZ_WINDOW_SZ;
    // account for 2 ISR calls == 1 sin wave
    result /= 2UL;
    // convert to freq (Ttimer/Tavg_sin_wave)
    result = ZX_TIMER_TICK_FREQ_HZ / result;

    return static_cast<uint16_t>(result);
}

void ZeroCross::window_swp()
{
    sample_window = (sample_window == mains_hz_window_a) ? mains_hz_window_b : mains_hz_window_a;
    proc_window = (proc_window == mains_hz_window_a) ? mains_hz_window_b : mains_hz_window_a;
    window_pos = 0U;
}

void ZeroCross::hz_calc_evt()
{
    BaseType_t higher_priority_task_awoken = pdFALSE;

    // notify ctrl task
    xEventGroupSetBitsFromISR(evt_grp_lamp_ctrl_hdl, EVT_GRP_LAMP_CTRL_HZ_CALC, &higher_priority_task_awoken);
    portYIELD_FROM_ISR(higher_priority_task_awoken);
}

void ZeroCross::mv_hz_smpl_to_window()
{
    sample_window[window_pos] = TIM15->CNT;
    TIM15->CNT = 0UL;
    window_pos++;
}

void ZeroCross::zero_cross_ISR_cb(void* arg)
{
    ZeroCross* _zx = static_cast<ZeroCross*>(arg);

    switch (_zx->state)
    {
        case ZxSampleState::INACTIVE:
            _zx->state = ZxSampleState::START;
            break;

        case ZxSampleState::START:
            HAL_TIM_Base_Start_IT(_zx->hdl_zx_timer);
            _zx->state = ZxSampleState::DEBOUNCE;
            break;

        case ZxSampleState::DEBOUNCE:
            if (TIM15->CNT > (EXPECTED_ZX_PERIOD / 5))
                _zx->state = ZxSampleState::SAMPLING;
            break;

        case ZxSampleState::SAMPLING:

            HAL_TIM_Base_Stop_IT(_zx->hdl_zx_timer);

            if (_zx->window_pos > MAINS_HZ_WINDOW_SZ)
            {
                _zx->window_swp();
                _zx->hz_calc_evt();
            }

            _zx->mv_hz_smpl_to_window();

            HAL_TIM_Base_Start_IT(_zx->hdl_zx_timer);
            _zx->state = ZxSampleState::DEBOUNCE;

        default:
            break;
    }
}
