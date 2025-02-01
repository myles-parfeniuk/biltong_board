#include "HeatLampDriver.h"

HeatLampDriver::HeatLampDriver(Device& d, TIM_HandleTypeDef* hdl_zx_timer)
    : d(d)
    , zero_cross(d, hdl_zx_timer, evt_grp_lamp_ctrl_hdl)
{
    // ensure relay is initially closed
    HAL_GPIO_WritePin(PIN_HEAT_LAMP_EN.port, PIN_HEAT_LAMP_EN.num, GPIO_PIN_RESET);
}

bool HeatLampDriver::init()
{

    OPEEngineRes_t op_success = OPEE_OK;

    op_success = d.heat_lamps.relay_closed.subscribe<8UL>(
            [](bool relay_closed)
            {
                if (relay_closed)
                    HAL_GPIO_WritePin(PIN_HEAT_LAMP_EN.port, PIN_HEAT_LAMP_EN.num, GPIO_PIN_SET);
                else
                    HAL_GPIO_WritePin(PIN_HEAT_LAMP_EN.port, PIN_HEAT_LAMP_EN.num, GPIO_PIN_RESET);
            });

    if (op_success != OPEE_OK)
        return false;

    if (!zero_cross.init())
        return false;

    evt_grp_lamp_ctrl_hdl = xEventGroupCreateStatic(&evt_grp_lamp_ctrl_buff);

    task_lamp_ctrl_hdl = xTaskCreateStatic(
            task_lamp_ctrl_trampoline, "bbLampCtrlTsk", BB_HL_CTRL_TSK_SZ, static_cast<void*>(this), 6, task_lamp_ctrl_stk, &task_lamp_ctrl_tcb);

    return true;
}

void HeatLampDriver::task_lamp_ctrl_trampoline(void* arg)
{
    HeatLampDriver* _heat_lamp_driver = static_cast<HeatLampDriver*>(arg);
    _heat_lamp_driver->task_lamp_ctrl();
}

void HeatLampDriver::task_lamp_ctrl()
{
    EventBits_t ctrl_bits = 0UL;
    int32_t zx_period_us = 0L;
    float mains_hz = 0.0f;

    while (1)
    {
        ctrl_bits = xEventGroupWaitBits(evt_grp_lamp_ctrl_hdl, EVT_GRP_LAMP_CTRL_ALL, pdFALSE, pdFALSE, portMAX_DELAY);

        if (ctrl_bits & EVT_GRP_LAMP_CTRL_HZ_CALC)
        {   
            zx_period_us = zero_cross.avg_window();
            mains_hz = zero_cross.hz_calc(zx_period_us);
            d.heat_lamps.zx_period_us.set(zx_period_us);
            d.heat_lamps.mains_hz.set(mains_hz);
            xEventGroupClearBits(evt_grp_lamp_ctrl_hdl, EVT_GRP_LAMP_CTRL_HZ_CALC);
        }
    }
}
