#pragma once
// OPEEngine
#include "OPEEngineConfig.h"
#include "OPEEngine_types.h"
#include "CbPool.h"
#include "DWStkCtrlBlk.h"
#include "SubCtrlBlk.h"
#include "CbWrprDefined.h"

namespace opee
{
    template <opee_size_t DWMaxCnt>
    class CbPoolManager
    {
        private:
            static constexpr const char* TAG = "CbPoolManager"; ///< Class tag for debug logs.
            static constexpr const opee_uint8_t DW_STK_GUARD_BYTE = 0xFFU;

            inline static CbPool<OPEEconfigCB_POOL_SZ> cb_pool;    ///< Callback memory pool, contains all data watch stacks, which contain individual callbacks
            inline static DWStkCtrlBlk dw_stk_ctrl_blks[DWMaxCnt]; ///< DataWatch Stack control blocks (contains context for each DataWatch stack)
            inline static opee_size_t dw_count = 0U;               ///< Total count of allocated DataWatch stacks
            inline static opee_size_t allocator_ofs = 0U;          ///< DataWatch stack allocator offset (points to next free element in cb_poo to allocate DataWatch stack)

            template <opee_size_t DWStkSz>
            static constexpr bool check_cb_pool_overflow(const opee_size_t next_allocator_ofs)
            {
                return (next_allocator_ofs < OPEEconfigCB_POOL_SZ);
            }

            template <opee_size_t CbWrprMaxSz>
            static constexpr bool check_dw_stk_overflow(const DWStkCtrlBlk dw_stk_ctrl_blk)
            {
                return ((dw_stk_ctrl_blk.cb_pool_addr_ofs + dw_stk_ctrl_blk.stk_ptr_ofs + CbWrprMaxSz) < (dw_stk_ctrl_blk.cb_pool_addr_ofs + dw_stk_ctrl_blk.stk_sz));
            }

        public:
            CbPoolManager()
            {
            }

            void reset()
            {
                dw_count = 0U;
                allocator_ofs = 0U;

                for (opee_size_t i = 0; i < DWMaxCnt; i++)
                    dw_stk_ctrl_blks[i] = DWStkCtrlBlk();

                cb_pool.reset();
            }

            template <opee_size_t DWStkSz, opee_size_t CbMaxCnt>
            OPEEngineRes_t allocate_dw_stk(opee_size_t& dw_stk, SubCtrlBlk* subscribers)
            {

                // compilation check: does does DWStkSz exceed exceed max OPEEconfigMAX_DATA_WATCH_STK_SZ?
                DWStkCtrlBlk::check_dw_stk_sz<DWStkSz>();

                // runtime check: will allocating this DWStk exceed OPEEconfigCB_POOL_SZ?
                if (!check_cb_pool_overflow<DWStkSz>(DWStkSz + allocator_ofs))
                    return OPEE_CB_POOL_FULL;

                // ensure max amount of DW objects have not been exceeded
                if (dw_count >= DWMaxCnt)
                    return OPEE_MAX_DWSTK_CNT_EXCEEDED;

                // verify the entirety of desired space is free (== 0)
                for (opee_size_t i = allocator_ofs; i < DWStkSz; i++)
                    if (cb_pool[i] != 0U)
                        return OPEE_CB_POOL_RGN_NOT_EMPTY;

                // store context for respective dw stack
                dw_stk_ctrl_blks[dw_count] = DWStkCtrlBlk(allocator_ofs, 0, DWStkSz, CbMaxCnt, subscribers);

                // increment the datawatch count and allocator offset for next DW registration
                allocator_ofs += DWStkSz;
                // save dw_stk # to reference for cb allocation
                dw_stk = dw_count;

                // insert DwStk guard bytes
                cb_pool[dw_stk_ctrl_blks[dw_count].cb_pool_addr_ofs + DWStkSz - 1] = DW_STK_GUARD_BYTE;
                cb_pool[dw_stk_ctrl_blks[dw_count].cb_pool_addr_ofs + DWStkSz - 2] = DW_STK_GUARD_BYTE;

                dw_count++;

                return OPEE_OK;
            }

            template <typename TArg, typename TCb, opee_size_t CbWrprMaxSz>
            OPEEngineRes_t store_cb(SubCtrlBlk* subscribers, opee_uint8_t& sub_count, const opee_size_t dw_stk, CbWrprDefined<TArg, TCb>* cb_wrpr)
            {
                const constexpr opee_size_t data_sz = sizeof(CbWrprDefined<TArg, TCb>);

                // compilation check: does cb size in memory exceed CbWrprMaxSz?
                SubCtrlBlk::check_cb_wrpr_sz<data_sz, CbWrprMaxSz>();

                // runtime check: will allocating this CbWrpr exceed respective DWStkSz?
                if (!check_dw_stk_overflow<CbWrprMaxSz>(dw_stk_ctrl_blks[dw_stk]))
                    return OPEE_DWSTK_FULL;

                if (dw_stk >= dw_count)
                    return OPEE_INVALID_DWSTK_IDX;

                // todo define optional debug statements
                // ESP_LOGI(TAG, "CbWrapper Sz: %dbytes", data_sz);

                const opee_size_t cb_pool_addr_ofs = dw_stk_ctrl_blks[dw_stk].cb_pool_addr_ofs + dw_stk_ctrl_blks[dw_stk].stk_ptr_ofs;

                // verify the entirety of desired space is free (== 0)
                for (opee_size_t i = cb_pool_addr_ofs; i < (cb_pool_addr_ofs + CbWrprMaxSz); i++)
                    if (cb_pool[i] != 0)
                        return OPEE_CB_POOL_RGN_NOT_EMPTY;

                CbWrprDefined<TArg, TCb>* _wrpr = new (cb_pool.begin() + cb_pool_addr_ofs) CbWrprDefined<TArg, TCb>(*cb_wrpr);

                if (_wrpr == nullptr)
                    return OPEE_CB_WRPR_CREATION_FAILED;

                // checksum is all elements of serialized calback data XOR'd together
                const opee_uint8_t checksum = SubCtrlBlk::create_checksum(cb_pool_addr_ofs, data_sz, cb_pool);

                // save callback context
                subscribers[sub_count++] = SubCtrlBlk(cb_pool_addr_ofs, data_sz, checksum, &cb_pool, reinterpret_cast<CbWrprGeneric*>(_wrpr));
                // store checksum as guard byte at cb wrapper boundry
                cb_pool[cb_pool_addr_ofs + CbWrprMaxSz - 1] = checksum;

                // increment respective stack pointer to next free chunk
                dw_stk_ctrl_blks[dw_stk].stk_ptr_ofs += CbWrprMaxSz;

                return OPEE_OK;
            }

            friend class PoolManagerTests;
    };
} // namespace opee