#pragma once

// std library
#include <stdint.h>
// cube-mx
#include "i2c.h"
// third-party
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
// in-house
#include "bb_pin_defs.h"
#include "Device.h"
#include "TempRHSens.h"
#include "SerialService.h"
#include "BBEventHandler.h"

class TempRHDriver
{
    public:
        TempRHDriver(Device& d, I2C_HandleTypeDef* hdl_i2c_th_A, I2C_HandleTypeDef* hdl_i2c_th_B);
        bool init();

    private:
        static void timer_temp_cb(TimerHandle_t timer_hdl);
        static void timer_rh_cb(TimerHandle_t timer_hdl);
        static void sample_temp_evt_cb(void *arg);
        static void sample_rh_evt_cb(void *arg);
  
        Device& d;
        I2C_HandleTypeDef* hdl_i2c_th_A;
        I2C_HandleTypeDef* hdl_i2c_th_B;
        TempRHSens th_A;
        TempRHSens th_B;

        inline static StaticTimer_t timer_temp_buff;
        inline static StaticTimer_t timer_rh_buff;
        inline static TimerHandle_t timer_temp_hdl = NULL;
        inline static TimerHandle_t timer_rh_hdl = NULL;

        inline static const constexpr char* TAG = "TempRHDriver";
};