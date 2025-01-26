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
            , sensors({TEMPERATURE_DEFAULT_SAMPLE_RATE_MS, temp_data_t{-1, -1}}, {HUMIDITY_DEFAULT_SAMPLE_RATE_MS, rh_data_t{-1, -1}})
            , heat_lamps{false, 0U, -1.0f, -1L}

        {
        }
};
