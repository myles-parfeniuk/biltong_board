#pragma once
// OPEEngine
#include "OPEEngine_types.h"
#include "SubCtrlBlk.h"

namespace opee
{
    class DWStkCtrlBlk
    {
        public:
            DWStkCtrlBlk()
                : cb_pool_addr_ofs(0U)
                , stk_ptr_ofs(0U)
                , stk_sz(0U)
                , max_sub_cnt(0U)
                , _subscribers(nullptr)
            {
            }

            DWStkCtrlBlk(opee_size_t cb_pool_addr_ofs, opee_size_t stk_ptr_ofs, opee_size_t stk_sz, opee_uint8_t max_sub_cnt, SubCtrlBlk* _subscribers)
                : cb_pool_addr_ofs(cb_pool_addr_ofs)
                , stk_ptr_ofs(stk_ptr_ofs)
                , stk_sz(stk_sz)
                , max_sub_cnt(max_sub_cnt)
                , _subscribers(_subscribers)
            {
            }

            template <opee_size_t DWStkSz>
            static constexpr void check_dw_stk_sz()
            {
                static_assert(DWStkSz <= OPEEconfigMAX_DATA_WATCH_STK_SZ, "Max DataWatch stack size exceeded. Increase OPEEconfigMAX_DW_STK_SZ or decrease DataWatch stack size.");
            }

            opee_size_t cb_pool_addr_ofs; ///< start address of respective DataWatch stack (as offset from start address of cb_pool)
            opee_size_t stk_ptr_ofs;      ///< stack pointer offset (points to next free element in respective DataWatch stack)
            opee_size_t stk_sz;           ///< total allocated stack size for respective DataWatch stack
            opee_uint8_t max_sub_cnt;     ///< max callbacks allowed to be registered to this DataWatch stack
            SubCtrlBlk* _subscribers;     ///< pointer to subscriber list that lives in respective DataWatch object
    };
} // namespace opee