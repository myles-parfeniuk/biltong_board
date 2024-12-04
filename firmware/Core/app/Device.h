#pragma once

#include "DataWatch.h"
#include "ButtonDriver_types.h"

class Device
{
    public:
        typedef struct biltong_buttons_t
        {
                DataWatch<ButtonEvent, 4> up;
                DataWatch<ButtonEvent, 4> enter;
                DataWatch<ButtonEvent, 4> down;
        } biltong_buttons_t;

        Device();

        biltong_buttons_t buttons; 
};
