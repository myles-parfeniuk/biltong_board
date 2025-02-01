#pragma once

// cube mx
#include "gpio.h"
// in-house
#include "bb_pin_defs.h"
#include "bb_task_defs.h"
#include "Device.h"
#include "ZeroCross.h"

class HeatLampDriver
{
    public:
        HeatLampDriver(Device& d, TIM_HandleTypeDef* hdl_zx_timer);
        bool init();
        static void task_lamp_ctrl_trampoline(void* arg);
        void task_lamp_ctrl();

    private:
        Device& d;
        ZeroCross zero_cross;

        inline static TaskHandle_t task_lamp_ctrl_hdl = NULL;
        inline static StaticTask_t task_lamp_ctrl_tcb;
        inline static StackType_t task_lamp_ctrl_stk[BB_HL_CTRL_TSK_SZ] = {0UL};

        EventGroupHandle_t evt_grp_lamp_ctrl_hdl = NULL;
        StaticEventGroup_t evt_grp_lamp_ctrl_buff;

        static const constexpr EventBits_t EVT_GRP_LAMP_CTRL_HZ_CALC = (1UL << 0UL);
        static const constexpr EventBits_t EVT_GRP_LAMP_CTRL_TRIAC_TRIG = (1UL << 1UL);
        static const constexpr EventBits_t EVT_GRP_LAMP_CTRL_ALL = EVT_GRP_LAMP_CTRL_HZ_CALC | EVT_GRP_LAMP_CTRL_TRIAC_TRIG;
        static const constexpr char* TAG = "HeatLampDriver";
};