#pragma once
// third-pary
#include "OPEEngine.h"
// in-house
#include "ButtonDriver_types.h"

// clang-format off
class Device
{
    public:
        typedef struct button_t
        {
            opee::DataWatch<ButtonEvent, 80, 5> up;
            opee::DataWatch<ButtonEvent, 80, 5> down;
            opee::DataWatch<ButtonEvent, 80, 5> enter;
        }button_t;

        /*USER CODE: instantiate data containers here:*/

        button_t buttons; 

        /*USER CODE: end*/

        Device(): 

            /*USER CODE: instantiate data objects with initial values:*/

            buttons({ButtonEvent::uninit, ButtonEvent::uninit, ButtonEvent::uninit}) // initial value of uninitialized
            
            /*USER CODE: end*/
        {
        }
};

// clang-format on