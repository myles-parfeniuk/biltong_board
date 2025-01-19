#pragma once
// OPEEngine
#include "CbWrprGeneric.h"
#include "PlacementUtil.h"

namespace opee
{

    // template derived class that wraps a callback
    template <typename TArg, typename TCb>
    class CbWrprDefined : public CbWrprGeneric
    {
        private:
            TCb cb;

        public:
            // constructor to set the callback
            CbWrprDefined(TCb cb)
                : cb(cb)
            {
            }

            bool invoke(opee_uintptr_t arg2p_addr, opee_uintptr_t data_addr) override
            {
                volatile TArg* _arg2p = reinterpret_cast<volatile TArg*>(arg2p_addr);
                volatile TArg* _data;
                TArg temp_arg2p;
                opee::_memcpy(const_cast<TArg*>(&temp_arg2p), const_cast<TArg*>(_arg2p), sizeof(TArg));

                cb(temp_arg2p);

                // last callback in salvo callbacks requested by data watch object if data_addr != 0
                if (data_addr != 0)
                {
                    // overwrite the data with the new data after all callbacks are finished executing
                    _data = reinterpret_cast<volatile TArg*>(data_addr);
                    opee::_memcpy(const_cast<TArg*>(_data), const_cast<TArg*>(_arg2p), sizeof(TArg));

                    return true;
                }

                return false;
            }
    };
} // namespace opee
