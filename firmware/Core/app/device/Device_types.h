#pragma once

// std library includes
#include <stdint.h>
// third-pary
#include "OPEEngine.h"

// constants
static const constexpr TickType_t TEMPERATURE_DEFAULT_SAMPLE_RATE_MS = 2000UL;
static const constexpr TickType_t HUMIDITY_DEFAULT_SAMPLE_RATE_MS = 2000UL;

// data types
typedef struct temp_data_t
{
        int32_t A;
        int32_t B;
} temp_data_t;

typedef struct rh_data_t
{
        int32_t A;
        int32_t B;
} rh_data_t;

enum class SwitchEvent
{
    quick_press,
    long_press,
    held,
    release,
    uninit
};

// device model
typedef struct switch_t
{
        opee::DataWatch<SwitchEvent, 64, 8> up;
        opee::DataWatch<SwitchEvent, 64, 8> down;
        opee::DataWatch<SwitchEvent, 64, 8> enter;
} switch_t;

typedef struct
{
        struct
        {
                opee::DataWatch<uint32_t, 32, 4> sample_rate;
                opee::DataWatch<temp_data_t, 32, 4> celsius;
        } temperature;

        struct
        {
                opee::DataWatch<uint32_t, 32, 4> sample_rate;
                opee::DataWatch<rh_data_t, 32, 4> relative;
        } humidity;

} sens_t;

typedef struct heat_lamps_t
{
        opee::DataWatch<bool, 32, 4> relay_closed;
        opee::DataWatch<uint8_t, 32, 4> intensity;
        opee::DataWatch<float, 32, 4> mains_hz;
        opee::DataWatch<int32_t, 16, 2> zx_period_us;
} heat_lamps_t;