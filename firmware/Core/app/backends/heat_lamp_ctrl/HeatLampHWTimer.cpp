#include "HeatLampHWTimer.h"

HeatLampHWTimer::HeatLampHWTimer(TIM_HandleTypeDef* hdl_heatlamp_timer)
    : hdl_heatlamp_timer(hdl_heatlamp_timer)
{
}

bool HeatLampHWTimer::stop()
{
    HAL_StatusTypeDef op_success = HAL_ERROR;

    op_success = HAL_TIM_OC_Stop_IT(hdl_heatlamp_timer, TIM_CHANNEL_1);

    return (op_success == HAL_OK);
}

bool HeatLampHWTimer::start()
{
    HAL_StatusTypeDef op_success = HAL_ERROR;

    op_success = HAL_TIM_OC_Start_IT(hdl_heatlamp_timer, TIM_CHANNEL_1);

    return (op_success == HAL_OK);
}

bool HeatLampHWTimer::restart()
{
    if (!stop())
        return false;

    HW_TIM->CNT &= 0UL;

    if (!start())
        return false;

    return true;
}

uint32_t HeatLampHWTimer::get_ticks()
{
    return HW_TIM->CNT;
}

uint32_t HeatLampHWTimer::get_triac_trig_ticks()
{
    return HW_TIM->CCR1;
}

void HeatLampHWTimer::set_triac_trig_OC_ch(uint32_t oc_ticks)
{
    HW_TIM->CCR1 = oc_ticks;
}