#pragma once

// cube-mx
#include "tim.h"
// in-house
#include "Device.h"
#include "ISRCbDispatch.h"
#include "SerialService.h"

class ZeroCross
{
    public:
        ZeroCross(Device& d, TIM_HandleTypeDef* hdl_zx_timer, EventGroupHandle_t& evt_grp_lamp_ctrl_hdl);
        bool init();
        int32_t avg_window(); 
        float hz_calc(int32_t window_avg);

    private:
        enum class ZxSampleState
        {
            INACTIVE,
            START,
            DEBOUNCE,
            DONE_SAMPLE
        };

        static const constexpr size_t MAINS_HZ_WINDOW_SZ = 8UL;
        static const constexpr size_t ZX_TIMER_TICK_FREQ_HZ = 1000000UL;
        static const constexpr size_t EXPECTED_MAINS_FREQ_HZ = 120UL;
        static const constexpr size_t EXPECTED_ZX_PERIOD = ZX_TIMER_TICK_FREQ_HZ / EXPECTED_MAINS_FREQ_HZ;
        static const constexpr size_t DEBOUNCE_THRESHOLD = EXPECTED_ZX_PERIOD / 10UL;

        static uint32_t grab_sample_and_reset_zx_timer(ZeroCross* _zx);
        static void mv_hz_smpl_to_window(ZeroCross* _zx, const uint32_t sample);
        static void window_swp(ZeroCross* _zx);
        static void hz_calc_evt(ZeroCross* _zx);
        static void zero_cross_ISR_cb(void* arg);

        Device& d;
        TIM_HandleTypeDef* hdl_zx_timer = nullptr;
        EventGroupHandle_t& evt_grp_lamp_ctrl_hdl;
        ZxSampleState state = ZxSampleState::INACTIVE;
        uint8_t window_pos = 0UL;
        uint32_t mains_hz_window_a[MAINS_HZ_WINDOW_SZ] = {0UL};
        uint32_t mains_hz_window_b[MAINS_HZ_WINDOW_SZ] = {0UL};
        volatile uint32_t* sample_window = nullptr;
        volatile uint32_t* proc_window = nullptr;

        static const constexpr EventBits_t EVT_GRP_LAMP_CTRL_HZ_CALC = (1UL << 0UL);
        static const constexpr EventBits_t EVT_GRP_LAMP_CTRL_ALL = EVT_GRP_LAMP_CTRL_HZ_CALC;

        static const constexpr char* TAG = "ZeroCross";
};