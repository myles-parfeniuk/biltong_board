#pragma once

#include <stdint.h>
#include <stdio.h>
#include "i2c.h"
#include "usart.h"
#include "third_party/WSEN_HIDS/WeSensorsSDK.h"
#include "third_party/WSEN_HIDS/WSEN_HIDS_2525020210002.h"

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