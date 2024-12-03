#pragma once

#include <functional>

/**
 * @class CbGeneric
 *
 * @brief A generic wrapper class to abstract away template details related to std::function.
 *
 * This class serves as a parent class for `CbArgTyped`. It provides a mechanism to treat the
 * callback as a generic pointer, enabling the storage and handling of different callback types in a uniform way.
 * This primarily exists such that callbacks declared with any data type can be passed through a single queue.
 */
class CbGeneric
{
    public:
        /**
         * @brief Pure virtual function to invoke callback wrapped by child template class.
         *
         * @return void, nothing to return
         */
        virtual void invoke() = 0;

        /**
         * @brief Destructor for cleanup of derived objects.
         * @return void, nothing to return
         */
        ~CbGeneric() = default;

    protected:
        /**
         * @brief Protected constructor to prevent instantiation of the base class.
         *
         * This class is meant to be inherited and never instantiated directly, it is only used to cast to type, ie:
         * CbArgTyped<uint16_t> cb_typed;
         * CbGeneric *cb_generic = &cb_typed;
         */
        CbGeneric() = default;

        static const constexpr char* TAG = "CbGeneric"; ///< Class tag, used in debug logs.
};