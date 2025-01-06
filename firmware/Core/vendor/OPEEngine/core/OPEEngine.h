#pragma once

#include "DataWatch.h"
#include "Subscriber.h"
#include "CbHelper.h"
#include "OPEEngine_types.h"

namespace opee
{
    static void OPEEngine_init()
    {
        CbPoolManager<OPEEconfigMAX_DATA_WATCH_CNT>& pool_manager = CbHelper<OPEEconfigMAX_DATA_WATCH_CNT>::get_manager();
        pool_manager.reset();
        CbHelper<OPEEconfigMAX_DATA_WATCH_CNT>::init();
    }

    constexpr const char* OPEEngineRes_to_str(OPEEngineRes_t res)
    {
        switch (res)
        {
            case OPEE_OK:
                return "OPEE_OK";
            case OPEE_CB_POOL_FULL:
                return "OPEE_CB_POOL_FULL";
            case OPEE_DW_STK_NOT_INIT:
                return "OPEE_DW_STK_NOT_INIT";
            case OPEE_MAX_DWSTK_CNT_EXCEEDED:
                return "OPEE_MAX_DWSTK_CNT_EXCEEDED";
            case OPEE_DWSTK_FULL:
                return "OPEE_DWSTK_FULL";
            case OPEE_INVALID_DWSTK_IDX:
                return "OPEE_INVALID_DWSTK_IDX";
            case OPEE_CB_POOL_RGN_NOT_EMPTY:
                return "OPEE_CB_POOL_RGN_NOT_EMPTY";
            case OPEE_CB_WRPR_CREATION_FAILED:
                return "OPEE_CB_WRPR_CREATION_FAILED";
            case OPEE_MAX_SUB_CNT_EXCEEDED:
                return "OPEE_MAX_SUB_CNT_EXCEEDED";
            case OPEE_CB_QUEUE_FULL:
                return "OPEE_MAX_SUB_CNT_EXCEEDED";
            case OPEE_ALL_CB_MUTED:
                return "OPEE_MAX_SUB_CNT_EXCEEDED";
            case OPEE_CB_HELPER_BUSY:
                return "OPEE_MAX_SUB_CNT_EXCEEDED";
            default:
                return "UNKNOWN_ERROR";
        }
    }
} // namespace opee
