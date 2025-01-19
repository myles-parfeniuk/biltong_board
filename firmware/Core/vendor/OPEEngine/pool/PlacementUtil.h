#pragma once
#include "OPEEngine_types.h"

// overloaded new operator for placement new
inline void* operator new(opee_size_t size, opee_uint8_t* ptr)
{
    return static_cast<void*>(ptr);
}

namespace opee
{
    static void* _memcpy(void* dest, const void* src, opee_size_t n)
    {
        opee_uint8_t* d = reinterpret_cast<opee_uint8_t*>(dest);
        const opee_uint8_t* s = reinterpret_cast<const opee_uint8_t*>(src);

        for (opee_size_t i = 0; i < n; ++i)
        {
            d[i] = s[i];
        }

        return dest; // Return the destination pointer, like standard memcpy
    }
} // namespace opee