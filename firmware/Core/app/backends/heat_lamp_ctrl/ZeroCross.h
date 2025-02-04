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
        enum class ZxSampleState
        {
            INACTIVE,
            START,
            SAMPLING
        };

        static uint32_t grab_sample_and_reset_zx_timer(ZeroCross* _zx);
        static void mv_hz_smpl_to_window(ZeroCross* _zx, const uint32_t sample);
        static void window_swp(ZeroCross* _zx);
        static void hz_calc_evt(ZeroCross* _zx);
        static void zero_cross_ISR_cb(void* arg);

        // constants
        inline static const constexpr size_t MAINS_HZ_WINDOW_SZ = 16UL;
        inline static const constexpr size_t EXPECTED_MAINS_FREQ_HZ = 60UL;
        inline static const constexpr size_t EXPECTED_ZX_PERIOD = HeatLampHWTimer::HEAT_LAMP_TIMER_TICK_FREQ_HZ / (2UL * EXPECTED_MAINS_FREQ_HZ);

        Device& d;
        HeatLampHWTimer& hw_timer_heat_lamp;
        ZxSampleState state = ZxSampleState::INACTIVE;
        uint8_t window_pos = 0UL;
        uint32_t mains_hz_window_a[MAINS_HZ_WINDOW_SZ] = {0UL};
        uint32_t mains_hz_window_b[MAINS_HZ_WINDOW_SZ] = {0UL};
        volatile uint32_t* sample_window = nullptr;
        volatile uint32_t* proc_window = nullptr;

        inline static const constexpr char* TAG = "ZeroCross";
};