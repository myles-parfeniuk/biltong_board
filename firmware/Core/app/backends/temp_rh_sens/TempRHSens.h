#pragma once

// std lib 
#include <stdint.h>
// cube-mx
#include "i2c.h"
// third-party 
#include "WeSensorsSDK.h"
#include "WSEN_HIDS_2525020210002.h"
// in-house 
#include "SerialService.h"

class TempRHSens
{
    public:
        TempRHSens(I2C_HandleTypeDef* hi2c);

        bool init();
        bool get_temp(int32_t& temperature_raw);
        bool get_humidity(int32_t& humidity_raw);

    private:
        // constants
        inline static const constexpr TickType_t BOOT_DELAY_MS = 50UL / portTICK_PERIOD_MS;

        I2C_HandleTypeDef* hdl_i2c = nullptr;
        WE_sensorInterface_t hids;

        inline static const constexpr char* TAG = "TempRHSens";
};