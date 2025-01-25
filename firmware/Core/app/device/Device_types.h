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
        opee::DataWatch<SwitchEvent, 80, 5> up;
        opee::DataWatch<SwitchEvent, 80, 5> down;
        opee::DataWatch<SwitchEvent, 80, 5> enter;
} switch_t;

typedef struct temp_sens_t
{
        opee::DataWatch<uint32_t, 32, 2> sample_rate;
        opee::DataWatch<temp_data_t, 64, 4> celsius;
} temp_sens_t;

typedef struct rh_sens_t
{
        opee::DataWatch<uint32_t, 32, 2> sample_rate;
        opee::DataWatch<rh_data_t, 64, 4> relative;

} rh_sens_t;

typedef struct sens_t
{
        temp_sens_t temperature;
        rh_sens_t humidity;
} sens_t;

typedef struct heat_lamps_t
{
        opee::DataWatch<bool, 32, 2> relay_closed;
        opee::DataWatch<uint8_t, 32, 2> intensity;
        opee::DataWatch<float, 32, 2> mains_hz;
} heat_lamps_t;