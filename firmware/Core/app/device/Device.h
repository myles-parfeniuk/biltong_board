#pragma once

// std library includes
#include <stdint.h>
// in-house
#include "Device_types.h"

class Device
{
    public:
        switch_t switches;
        sens_t sensors;
        heat_lamps_t heat_lamps;

        Device()
            : switches({SwitchEvent::uninit, SwitchEvent::uninit, SwitchEvent::uninit}) // initial value of uninitialized
            , sensors{temp_sens_t{{TEMPERATURE_DEFAULT_SAMPLE_RATE_MS}, {{-1, -1}}}, rh_sens_t{{HUMIDITY_DEFAULT_SAMPLE_RATE_MS}, {{-1, -1}}}}
            , heat_lamps{false, 0U, 0.0f}

        {
        }
};
