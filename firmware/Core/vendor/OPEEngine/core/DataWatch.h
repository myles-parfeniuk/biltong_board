#pragma once

// OPEEngine
#include "OPEEngine_types.h"
#include "OPEEngine_freeRTOS_hook.h"
#include "CbHelper.h"
#include "ForwardUtil.h"
#include "SubCtrlBlk.h"
#include "Subscriber.h"

namespace opee
{
    template <typename TArg, opee_size_t DWStkSz, opee_size_t CbMaxCnt>
    class DataWatch
    {

        private:
            typedef struct arg2p_wrpr_t
            {
                    TArg arg;
                    SemaphoreHandle_t lock;
            } arg2p_wrpr_t;

            SubCtrlBlk subscribers[CbMaxCnt];
            opee_uint8_t sub_count = 0;
            TArg data;
            opee_size_t dw_stk;
            CbPoolManager<OPEEconfigMAX_DATA_WATCH_CNT>& pool_manager;
            OPEEngineRes_t dw_stk_alloc_res = OPEE_DW_STK_NOT_INIT;
            arg2p_wrpr_t arg2p_buffer[OPEEconfigMAX_SET_REQUEST];
            StaticSemaphore_t arg2p_locks[OPEEconfigMAX_SET_REQUEST];

        public:
            DataWatch(TArg init_data)
                : data(init_data)
                , pool_manager(CbHelper<OPEEconfigMAX_DATA_WATCH_CNT>::get_manager())
            {
                dw_stk_alloc_res = pool_manager.template allocate_dw_stk<DWStkSz, CbMaxCnt>(dw_stk, subscribers);

                for (opee_size_t i = 0UL; i < OPEEconfigMAX_SET_REQUEST; i++)
                {
                    arg2p_buffer[i].lock = xSemaphoreCreateBinaryStatic(&arg2p_locks[i]);

                    if (arg2p_buffer[i].lock != NULL)
                        xSemaphoreGive(arg2p_buffer[i].lock);
                }
            }

            ~DataWatch()
            {
            }

            template <opee_size_t CbWrprMaxSz, typename TLambda>
            OPEEngineRes_t subscribe(TLambda&& lambda, Subscriber<TArg>* sub_interface = nullptr)
            {
                using TCb = ForwardUtil::decay_t<TLambda>; // get the actual type of the lambda by stripping it of references with decay

                // ensure respective DWStk allocated correctly
                if (dw_stk_alloc_res != OPEE_OK)
                    return dw_stk_alloc_res;

                // check if the max amount of subscribers has been reached
                if (sub_count < CbMaxCnt)
                {
                    CbWrprDefined<TArg, TCb> cb_wrpr(ForwardUtil::forward<TLambda>(lambda)); // create a temp wrapper object on stack to store callback

                    OPEEngineRes_t OPEEres = pool_manager.template store_cb<TArg, TCb, CbWrprMaxSz>(subscribers, sub_count, dw_stk, &cb_wrpr);

                    if ((sub_interface != nullptr) && (OPEEres == OPEE_OK))
                    {
                        *sub_interface = Subscriber<TArg>(&subscribers[sub_count - 1]);
                    }

                    return OPEEres;
                }

                return OPEE_MAX_SUB_CNT_EXCEEDED;
            }

            OPEEngineRes_t set(TArg arg, bool execute_callbacks = true, TickType_t timeout_ms = 1U)
            {
                OPEEngineRes_t OPEEres = OPEE_ALL_CB_MUTED;
                opee_size_t arg2p_idx = OPEEconfigMAX_SET_REQUEST;

                if (execute_callbacks && (sub_count > 0))
                {

                    for (opee_size_t i = 0UL; i < OPEEconfigMAX_SET_REQUEST; i++)
                    {
                        if (xSemaphoreTake(arg2p_buffer[i].lock, 0UL) != pdFALSE)
                        {
                            arg2p_idx = i;
                            break;
                        }
                    }

                    if (arg2p_idx >= OPEEconfigMAX_SET_REQUEST)
                        return OPEE_CB_HELPER_BUSY;

                    arg2p_buffer[arg2p_idx].arg = arg;

                    const opee_uintptr_t _arg2p = reinterpret_cast<opee_uintptr_t>(&arg2p_buffer[arg2p_idx].arg);
                    const opee_uintptr_t _data = reinterpret_cast<opee_uintptr_t>(&data);

                    OPEEres = CbHelper<OPEEconfigMAX_DATA_WATCH_CNT>::queue_cbs(subscribers, sub_count, _arg2p, _data, &arg2p_buffer[arg2p_idx].lock);

                    if (OPEEres == OPEE_ALL_CB_MUTED)
                    {
                        xSemaphoreGive(arg2p_buffer[arg2p_idx].lock);
                        data = arg;
                        OPEEres = OPEE_OK;
                    }
                }
                else
                {
                    data = arg;
                }

                return OPEEres;
            }

            TArg get()
            {
                return data;
            }
    };
} // namespace opee
