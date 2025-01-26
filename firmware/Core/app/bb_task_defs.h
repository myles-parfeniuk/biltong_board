#pragma once

// std library includes
#include <stdint.h>
// third party includes
#include "FreeRTOS.h"
#include "task.h"

const constexpr size_t _1KB = 128UL;

const constexpr size_t BB_IDLE_TSK_SZ = _1KB * 5UL;
const constexpr size_t BB_TEMP_RH_TSK_SZ = _1KB + (_1KB/2UL);
const constexpr size_t BB_SW_SCAN_TSK_SZ = _1KB + (_1KB/2UL);
const constexpr size_t BB_HL_CTRL_TSK_SZ = _1KB + (_1KB/2UL);
const constexpr size_t BB_SERIAL_SVC_TSK_SZ = _1KB * 1UL;