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
#include "HeatLampHWTimer.h"
#include "Triac.h"
#include "ZeroCross.h"

class HeatLampDriver
{
    public:
        HeatLampDriver(Device& d, TIM_HandleTypeDef* hdl_heat_lamp_timer);
        bool init();

    private:
        uint32_t intensity_2_triac_trig_ticks(uint8_t new_intensity);
        static void mains_hz_event_cb(void* arg);

        Device& d;
        HeatLampHWTimer hw_timer_heat_lamp;
        Triac triac;
        ZeroCross zero_cross;

        inline static const constexpr char* TAG = "HeatLampDriver";
};