#pragma once

#include "CbGeneric.h"

/**
 * @class CbArgTyped
 *
 * @brief Class to wrap an std::function with an argument to pass it.
 *
 * @tparam TData The type of data to be passed to the callback. The callbacks take the form of std::function<void(TData)>.
 * */
template <typename TData>
class CbArgTyped : public CbGeneric
{
    public:
        /**
         * @brief Constructs a CbArgTyped object.
         *
         * @param cb The callback to wrap.
         * @param arg2p Pointer to argument to pass to callback and invocation (lives inside DataWatch object), argument is passed as value.
         * @param data Pointer to data element to set as arg2p if this callback is the last one to be executed, for a salvo initiated by a specific
         *             DataWatch object.
         *
         * @return void, nothing to return
         */
        CbArgTyped(std::function<void(TData)> cb, TData* arg2p, TData* data)
            : cb(cb)
            , arg2p(arg2p)
            , last_in_chain(false)
        {
        }

        /**
         * @brief Invokes contained callback, passing it the current value of arg2p.
         *
         * This will also update the data contained in the DataWatch object if it is the last
         * callback to be executed for a salvo of callbacks initiated by said DataWatch object (last_in_chain).
         *
         * @return void, nothing to return
         */
        void invoke() override
        {
            cb(*arg2p);

            // overwrite current data with new data if this is the last callback to be executed
            if (last_in_chain)
                *data = *arg2p;
        }

        bool last_in_chain = false; ///< True if this callback is the last to be executed in a salvo initiated by a DataWatch object.

    private:
        std::function<void(TData)> cb; ///< The wrapped callback function.
        TData* arg2p;                  ///< Pointer to argument to pass to wrapped callback, lives within a respective DataWatch object.
        TData* data; ///< Pointer to DataWatch data to overwrite with arg2p if this callback is the last in a salvo of callbacks initiated by said DataWatch object.
};