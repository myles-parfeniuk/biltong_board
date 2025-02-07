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

class ZeroCross
{
    public:
        ZeroCross(Device& d, HeatLampHWTimer& hw_timer_heat_lamp);
        bool init();
        int32_t avg_window();
        float hz_calc(int32_t window_avg);

    private:
        static void mv_zx_period_smpl_to_window(ZeroCross* _zx, const uint16_t sample);
        static void window_swp(ZeroCross* _zx);
        static void hz_calc_evt(ZeroCross* _zx);
        static void zero_cross_ISR_cb(void* arg);

        // constants
        inline static const constexpr size_t ZX_PERIOD_SAMPLE_WINDOW_SZ = 16UL;
        inline static const constexpr size_t EXPECTED_MAINS_FREQ_HZ = 60UL;
        inline static const constexpr size_t EXPECTED_ZX_PERIOD = HeatLampHWTimer::HEAT_LAMP_TIMER_TICK_FREQ_HZ / (2UL * EXPECTED_MAINS_FREQ_HZ);

        Device& d;
        HeatLampHWTimer& hw_timer_heat_lamp;
        uint8_t window_pos = 0UL;
        uint16_t window_a[ZX_PERIOD_SAMPLE_WINDOW_SZ] = {0UL};
        uint16_t window_b[ZX_PERIOD_SAMPLE_WINDOW_SZ] = {0UL};
        volatile uint16_t* sample_window = nullptr;
        volatile uint16_t* proc_window = nullptr;

        inline static const constexpr char* TAG = "ZeroCross";
};