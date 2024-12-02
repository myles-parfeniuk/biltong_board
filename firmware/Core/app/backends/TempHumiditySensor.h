#pragma once

// std library includes
#include <stdint.h>
#include <stdio.h>
// cube mx includes
#include "i2c.h"
// third-party includes
#include "WeSensorsSDK.h"
#include "WSEN_HIDS_2525020210002.h"
// in-house includes
#include "SerialService.h"

class TempHumiditySensor
{
    public:
        TempHumiditySensor(I2C_HandleTypeDef* hi2c);

        bool init();
        int32_t get_temp();
        int32_t get_humidity();

    private:
        I2C_HandleTypeDef* hdl_i2c;
        WE_sensorInterface_t hids;
        static const constexpr char* TAG = "TempHumiditySensor";
};