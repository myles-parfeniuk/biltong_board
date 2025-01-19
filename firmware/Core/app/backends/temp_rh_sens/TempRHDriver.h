#pragma once

// std library includes
#include <stdint.h>
// cube mx includes
#include "i2c.h"
// in-house includes
#include "Device.h"
#include "TempRHSens.h"
#include "SerialService.h"

class TempRHDriver
{
    public:
        TempRHDriver(Device& d, I2C_HandleTypeDef* hdl_i2c_th_A, I2C_HandleTypeDef* hdl_i2c_th_B);
        bool init();

    private:
        static void task_temp_rh_trampoline(void* arg);
        void task_temp_rh();
        static void timer_temp_cb(TimerHandle_t timer_hdl);
        static void timer_rh_cb(TimerHandle_t timer_hdl);
        
        Device& d;
        I2C_HandleTypeDef* hdl_i2c_th_A;
        I2C_HandleTypeDef* hdl_i2c_th_B;
        TempRHSens th_A;
        TempRHSens th_B;
        TickType_t temp_sample_rate_ms;
        TickType_t humidity_sample_rate_ms;
        TaskHandle_t task_temp_rh_hdl = NULL;
        StaticTimer_t timer_temp_buff;
        StaticTimer_t timer_rh_buff; 
        TimerHandle_t timer_temp_hdl = NULL;
        TimerHandle_t timer_rh_hdl = NULL;

        inline static EventGroupHandle_t evt_grp_temp_rh_hdl = NULL;
        inline static StaticEventGroup_t evt_grp_temp_rh_buff;

        static const constexpr EventBits_t EVT_GRP_TEMP_RH_SAMPLE_TEMP = (1UL << 0UL);
        static const constexpr EventBits_t EVT_GRP_TEMP_RH_SAMPLE_RH = (1UL << 1UL);
        static const constexpr EventBits_t EVT_GRP_TEMP_RH_ALL = (EVT_GRP_TEMP_RH_SAMPLE_TEMP | EVT_GRP_TEMP_RH_SAMPLE_RH);
        static const constexpr char* TAG = "TempRHDriver";
};