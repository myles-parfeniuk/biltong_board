#pragma once
// OPEEngine
#include "OPEEngine_types.h"

namespace opee
{
    // base class for callback wrapper
    class CbWrprGeneric
    {
        public:
            virtual bool invoke(opee_uintptr_t arg2p_addr, opee_uintptr_t data_addr) = 0;
            virtual ~CbWrprGeneric() = default;
    };
} // namespace opee