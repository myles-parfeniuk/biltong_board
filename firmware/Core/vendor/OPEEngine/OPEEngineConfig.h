#pragma once

// OPEEngine
#include "OPEEngine_types.h"

const constexpr opee_size_t OPEEconfigCB_POOL_SZ = 512UL; ///< Size of combined callback storage pool in bytes.

const constexpr opee_size_t OPEEconfigCB_TASK_STK_SZ = 256UL; ///< Stack size for callback execution task in words (according to freeRTOS port, on a 32-bit system (256words)(4bytes) == 1kB)

const constexpr UBaseType_t OPEEconfigCB_TASK_PRIO = 7UL; ///< Priority of callback task, modify as needed, preferably kept as high priority as possible.

const constexpr opee_size_t OPEEconfigCB_QUEUE_SZ = 15UL; ///< Size of callback queue, maximum amount of callbacks that can be waiting for execution at any given time.

const constexpr opee_size_t OPEEconfigMAX_SET_REQUEST = 3UL; ///< Max amount of times set() can be called on a DW object when callbacks for respective object are already executing.

const constexpr opee_size_t OPEEconfigMAX_DATA_WATCH_CNT = 10UL; ///< Max amount of DataWatch objects that can be created.

const constexpr opee_size_t OPEEconfigMAX_DATA_WATCH_STK_SZ = 256UL; ///< Max size of a singular DataWatch stack.
