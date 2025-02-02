#pragma once

// cube mx
#include "gpio.h"
#include "FreeRTOS.h"
// third-party 
#include "task.h"
#include "event_groups.h"
// in-house
#include "bb_pin_defs.h"
#include "bb_task_defs.h"
#include "Device.h"
#include "ZeroCross.h"
#include "NSM201210XXR.h"

class HeatLampDriver
{
    public:
        HeatLampDriver(Device& d, TIM_HandleTypeDef* hdl_zx_timer, ADC_HandleTypeDef *hdl_isense_adc);
        bool init();
        static void task_lamp_ctrl_trampoline(void* arg);
        void task_lamp_ctrl();

    private:
        // constants
        inline static const constexpr EventBits_t EVT_GRP_LAMP_CTRL_HZ_CALC = (1UL << 0UL);
        inline static const constexpr EventBits_t EVT_GRP_LAMP_CTRL_ALL = EVT_GRP_LAMP_CTRL_HZ_CALC;

        Device& d;
        ZeroCross zero_cross;
        NSM201210XXR current_sens;

        inline static TaskHandle_t task_lamp_ctrl_hdl = NULL;
        inline static StaticTask_t task_lamp_ctrl_tcb;
        inline static StackType_t task_lamp_ctrl_stk[BB_HL_CTRL_TSK_SZ] = {0UL};

        EventGroupHandle_t evt_grp_lamp_ctrl_hdl = NULL;
        StaticEventGroup_t evt_grp_lamp_ctrl_buff;

        inline static const constexpr char* TAG = "HeatLampDriver";
};