#pragma once

//in-house includes
#include "ButtonDriver_types.h"
#include "Device.h"

class ButtonDriver
{
    public:
        ButtonDriver(Device &d);
        bool init(); 
    private:
        Device &d;
};


