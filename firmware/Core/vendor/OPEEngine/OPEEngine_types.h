#pragma once
#include <cstddef>
#include <stdint.h>

using opee_uint8_t = uint8_t;
using opee_uint_t = uint16_t;
using opee_int_t = int16_t;
using opee_uintl_t = uint32_t;
using opee_intl_t = int32_t;
using opee_size_t = size_t;
using opee_ssize_t = int32_t;
using opee_uintptr_t = uintptr_t;

enum OPEEngineRes_t
{
    OPEE_OK = 0,                 ///< Successful operation.
    OPEE_CB_POOL_FULL,           ///< CB pool is full, failure occurs if allocating a DWStk will result in CB Pool overflow, see OPEEconfigCB_POOL_SZ.
    OPEE_DW_STK_NOT_INIT,        ///< The DWStk for this DataWatch object was never correctly initialized.
    OPEE_MAX_DWSTK_CNT_EXCEEDED, ///< Max DataWatch objects have been exceeded, see OPEEconfigMAX_DATA_WATCH_CNT.
    OPEE_DWSTK_FULL,        ///< DWStk for this DataWatch object is full, failure occurs if allocating a callback will result in DWStk overflow, see DWStkSz on DataWatch object.
    OPEE_INVALID_DWSTK_IDX, ///< Invalid DWStk idx, there is no corresponding index for DWStk in CbPoolManager.
    OPEE_CB_POOL_RGN_NOT_EMPTY,   ///< Region in CB Pool where DWStk allocation was attempted is not empty.
    OPEE_CB_WRPR_CREATION_FAILED, ///< Null pointer was returned from CbWrpr constructor, callback storage aborted.
    OPEE_MAX_SUB_CNT_EXCEEDED,    ///< Max sub count for a given DataWatch object has been exceeded, see CbMaxCnt on DataWatch object.
    OPEE_CB_QUEUE_FULL,           ///< CB Queue is full, callbacks could not be posted to cb_task() for execution.
    OPEE_ALL_CB_MUTED,            ///< All CBs on subscriber list are muted.
    OPEE_CB_HELPER_BUSY           ///< CbHelper is already busy executing callbacks for this object, see OPEEconfigMAX_SET_REQUEST.
};
