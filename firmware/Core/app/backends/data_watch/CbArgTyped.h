#pragma once

#include "CbGeneric.h"
#include "FreeRTOS.h"
#include "semphr.h"

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
        CbArgTyped(std::function<void(TData)> cb, TData* arg2p, TData* data, SemaphoreHandle_t* mutex_cb_execution)
            : cb(cb)
            , arg2p(arg2p)
            , data(data)
            , last_in_chain(false)
            , mutex_cb_execution(mutex_cb_execution)
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

            // this callback is the last in a salvo of callbacks to be executed, initiated by a DataWatch object
            if (last_in_chain)
            {
                // overwrite DataWatch data with new data (arg2p)
                *data = *arg2p;
                // release the cb execution mutex such that callbacks are free to execute for next call of DataWatch::set()
                xSemaphoreGive(*mutex_cb_execution);
            }
        }

        bool last_in_chain = false; ///< True if this callback is the last to be executed in a salvo initiated by a DataWatch object.

    private:
        std::function<void(TData)> cb; ///< The wrapped callback function.
        TData* arg2p;                  ///< Pointer to argument to pass to wrapped callback, lives within a respective DataWatch object.
        TData* data; ///< Pointer to DataWatch data to overwrite with arg2p if this callback is the last in a salvo of callbacks initiated by said DataWatch object.
        SemaphoreHandle_t* mutex_cb_execution =
                nullptr; ///< Mutex to prevent callback execution from being requested before previous salvo of callback executions has completed.
};