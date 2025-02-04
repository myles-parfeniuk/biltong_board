#pragma once

// std-lib
#include <stdint.h>
// third-party
#include "FreeRTOS.h"
#include "task.h"
// in-house
#include "Device.h"
#include "ISRCbDispatch.h"
#include "BBEventHandler.h"
#include "SerialService.h"
#include "HeatLampHWTimer.h"

class Triac
{
    public:
        Triac(Device& d, HeatLampHWTimer& hw_timer_heat_lamp);
        bool init();
        bool set_trig_ticks(uint32_t new_trig_ticks);

    private:
        static void triac_trig_ISR_cb(void* arg);

        Device& d;
        HeatLampHWTimer& hw_timer_heat_lamp;
        volatile bool triggering = false;
        volatile uint32_t trig_ticks = HeatLampHWTimer::MAX_HEAT_LAMP_TIMER_TICKS;

        inline static const constexpr uint32_t TRIAC_TRIGGING_TIME_US = 25UL;
        inline static const constexpr char* TAG = "Triac";
};