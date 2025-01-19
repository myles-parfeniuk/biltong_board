#pragma once

//in-house includes
#include "Device.h"

class HeatLampDriver
{
    public:
    HeatLampDriver(Device &d);

    private:
    Device &d;
};