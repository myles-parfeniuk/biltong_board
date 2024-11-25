#pragma once

#include "../third_party/WSEN_HIDS_2525020210002.h"

class tempHumiditySensor
{
public:
    tempHumiditySensor();

    bool init();
    uint32_t getTemp();
    uint32_t getHumidity_Temp();

private:
    static WE_sensorInterface_t hids;
};