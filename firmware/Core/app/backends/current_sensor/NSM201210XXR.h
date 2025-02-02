#pragma once

// cube-mx
#include "adc.h"
//third-party
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
// in-house
#include "bb_pin_defs.h"
#include "bb_task_defs.h"
class NSM201210XXR
{
    public:
        NSM201210XXR(ADC_HandleTypeDef* hdl_isense_adc);
        bool init();

    private:
        inline static StaticTimer_t timer_isense_sample;
        inline static TimerHandle_t timer_isense_sample_hdl = NULL;
        ADC_HandleTypeDef* hdl_isense_adc;
};