#pragma once
// OPEEngine
#include "SubCtrlBlk.h"

namespace opee
{
    template <typename TArg>
    class Subscriber
    {
        public:
            Subscriber()
                : _ctrl_blk(nullptr)
            {
            }

            Subscriber(SubCtrlBlk* _ctrl_blk)
                : _ctrl_blk(_ctrl_blk)
            {
            }

            void notify(TArg arg2p)
            {
                this->arg2p = arg2p;
                opee_uintptr_t arg2p_addr = reinterpret_cast<opee_uintptr_t>(&this->arg2p);
                _ctrl_blk->_cb_wrpr->invoke(arg2p_addr, 0U);
            }

            bool is_muted()
            {
                return _ctrl_blk->muted;
            }

            void mute()
            {
                _ctrl_blk->muted = true;
            }

            void un_mute()
            {
                _ctrl_blk->muted = false;
            }

            bool validate_checksum()
            {
                return SubCtrlBlk::validate_checksum(*_ctrl_blk);
            }

        private:
            SubCtrlBlk* _ctrl_blk;
            TArg arg2p;
            static const constexpr char* TAG = "Subscriber";
    };
} // namespace opee