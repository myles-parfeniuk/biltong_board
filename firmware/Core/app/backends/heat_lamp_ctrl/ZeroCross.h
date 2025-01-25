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
        uint16_t hz_calc();

    private:
        enum class ZxSampleState
        {
            INACTIVE,
            START,
            DEBOUNCE,
            SAMPLING
        };

        static const constexpr size_t MAINS_HZ_WINDOW_SZ = 8UL;
        static const constexpr size_t ZX_TIMER_TICK_FREQ_HZ = 1000000UL;
        static const constexpr size_t EXPECTED_MAINS_FREQ_HZ = 120UL;
        static const constexpr size_t EXPECTED_ZX_PERIOD = ZX_TIMER_TICK_FREQ_HZ / EXPECTED_MAINS_FREQ_HZ;

        void window_swp();
        void mv_hz_smpl_to_window();
        void hz_calc_evt();
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