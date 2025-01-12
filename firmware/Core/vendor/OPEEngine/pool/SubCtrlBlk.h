#pragma once
// OPEEngine
#include "OPEEngine_types.h"
#include "CbPool.h"
#include "CbWrprGeneric.h"

namespace opee
{
    class SubCtrlBlk
    {
        public:
            SubCtrlBlk()
                : cb_pool_addr_ofs(0U)
                , data_sz(0U)
                , checksum(0U)
                , _cb_pool(nullptr)
                , _cb_wrpr(nullptr)
                , muted(true)
            {
            }

            SubCtrlBlk(opee_size_t dw_stk_addr_offs, opee_size_t data_sz, opee_size_t checksum, CbPool<OPEEconfigCB_POOL_SZ>* _cb_pool, CbWrprGeneric* _cb_wrpr)
                : cb_pool_addr_ofs(dw_stk_addr_offs)
                , data_sz(data_sz)
                , checksum(checksum)
                , _cb_pool(_cb_pool)
                , _cb_wrpr(_cb_wrpr)
                , muted(false)
            {
            }

            static opee_uint8_t create_checksum(const opee_size_t cb_pool_addr_ofs, const opee_size_t data_sz, const CbPool<OPEEconfigCB_POOL_SZ>& cb_pool)
            {
                opee_uint8_t checksum = 0U;

                for (opee_ssize_t i = cb_pool_addr_ofs; i < (cb_pool_addr_ofs + data_sz); i++)
                    checksum ^= cb_pool[i];

                return checksum;
            }

            static bool validate_checksum(const SubCtrlBlk& ctrl_blk)
            {
                opee_uint8_t checksum = 0U;

                if (ctrl_blk._cb_pool != nullptr)
                    checksum = create_checksum(ctrl_blk.cb_pool_addr_ofs, ctrl_blk.data_sz, *ctrl_blk._cb_pool);

                return (checksum == ctrl_blk.checksum);
            }

            template <opee_size_t Bytes2Allocate, opee_size_t CbWrprMaxSz>
            static constexpr void check_cb_wrpr_sz()
            {
                static_assert(Bytes2Allocate <= CbWrprMaxSz, "CbWrapper size exceeds CbWrprMaxSz, increase on subscribe call.");
            }

            opee_size_t cb_pool_addr_ofs;           ///< start address of respective callback (as offset from start address of cb_pool)
            opee_size_t data_sz;                    ///< size of stored callback in bytes (excludes checksum byte)
            opee_uint8_t checksum;                  ///< check sum byte
            CbPool<OPEEconfigCB_POOL_SZ>* _cb_pool; ///< CB Pool access for checksum validation
            CbWrprGeneric* _cb_wrpr;                ///< pointer to callback wrapper in cb_pool
            bool muted;                             ///< if true this callback will not be executed even if DataWatch::set is called
    };
} // namespace opee