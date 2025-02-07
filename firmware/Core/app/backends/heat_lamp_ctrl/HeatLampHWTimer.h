#pragma once

// std-lib
#include <stdint.h>
// cube-mx
#include "tim.h"

class HeatLampHWTimer
{
    public:
        HeatLampHWTimer(TIM_HandleTypeDef* hdl_heatlamp_timer);
        bool stop(); 
        bool start();
        bool restart(); 
        uint16_t get_ticks(); 
        uint16_t get_triac_trig_ticks(); 
        void set_triac_trig_OC_ch(uint32_t oc_ticks);

        static const constexpr uint32_t MAX_HEAT_LAMP_TIMER_TICKS = 65535UL;
        inline static const constexpr size_t HEAT_LAMP_TIMER_TICK_FREQ_HZ = 1000000UL;

    private:
        TIM_HandleTypeDef* hdl_heatlamp_timer = nullptr;

        inline static volatile TIM_TypeDef* const HW_TIM = TIM15;

        inline static const constexpr char* TAG = "HeatLampHWTimer";
};