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
            SAMPLING
        };

        static const constexpr size_t MAINS_HZ_WINDOW_SZ = 8UL;
        static const constexpr size_t ZX_TIMER_TICK_FREQ_HZ = 1000000UL;
        static const constexpr size_t EXPECTED_MAINS_FREQ_HZ = 60UL;
        static const constexpr size_t EXPECTED_ZX_PERIOD = ZX_TIMER_TICK_FREQ_HZ / (2UL * EXPECTED_MAINS_FREQ_HZ); 
        static const constexpr size_t DEBOUNCE_THRESHOLD = EXPECTED_ZX_PERIOD / 10UL;

        uint32_t intensity2ticks(uint8_t intensity);
        bool set_triac_trig_ticks(uint32_t new_trig_ticks);
        void set_zx_timer_triac_trig_ch(uint32_t oc_ticks);
        static uint32_t grab_sample_and_reset_zx_timer(ZeroCross* _zx);
        static void mv_hz_smpl_to_window(ZeroCross* _zx, const uint32_t sample);
        static void window_swp(ZeroCross* _zx);
        static void hz_calc_evt(ZeroCross* _zx);
        static void triac_trig_ISR_cb(void* arg);
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
        volatile bool triac_triggering = false; 

        inline static const constexpr uint32_t MAX_ZX_TIMER_TICKS = 65535UL;
        inline static const constexpr uint32_t TRIAC_TRIGGING_TIME_US = 800UL; 
        inline static const constexpr EventBits_t EVT_GRP_LAMP_CTRL_HZ_CALC = (1UL << 0UL);
        inline static const constexpr EventBits_t EVT_GRP_LAMP_CTRL_ALL = EVT_GRP_LAMP_CTRL_HZ_CALC;

        inline static const constexpr char* TAG = "ZeroCross";
};