#pragma once

//std library includes
#include <stdint.h>
#include <stdio.h>

//cube mx includes
#include "i2c.h"
#include "usart.h"

//third-party includes
#include "WeSensorsSDK.h"
#include "WSEN_HIDS_2525020210002.h"

class tempHumiditySensor
{
public:
    tempHumiditySensor();

    bool init();
    int32_t getTemp();
    int32_t getHumidity();

private:
    static WE_sensorInterface_t hids;
};