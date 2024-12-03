#pragma once

// std library includes
#include <stdio.h>
// third party includes
#include <etl/vector.h>
#include "FreeRTOS.h"
// in house includes
#include "CbHelper.h"

/**
 * @class DataWatch
 *
 * @brief Class to manage and encapsulate a piece of data (ie temperature reading, current reading, button event, etc...).
 *
 * @tparam TData The type of the data to be managed (ie int, float, etc.).
 * @tparam MAX_SUBS The maximum number of subscribers (callbacks) that can be registered with this object.
 *
 * This class allows for callbacks to be registered with the subscribe() method and executed with the set() method.
 * The callbacks will be executed independently in a task managed by this library.
 * */
template <typename TData, size_t MAX_SUBS>
class DataWatch
{
    public:
        /**
         * @brief Constructs a DataWatch object.
         *
         * @param init_data Initial value of the contained data.
         *
         * @return void, nothing to return
         */
        DataWatch(TData init_data)
            : data(init_data)
            , arg2p(init_data)
        {
        }

        /**
         * @brief Sets contained data and executes any subscribed callbacks.
         *
         * @param new_data The new data value to be set.
         *
         * @return True if all callbacks were successfully queued. 
         */
        bool set(TData new_data)
        {
            // set arg to pass with new value such that callbacks pass it correctly
            arg2p = new_data;
            if (cb_list.size() > 0)
            {
                // if cb helper isn't initialized, nothing will execute anyway
                if (!CbHelper::initialized)
                    return false;

                CbHelper::queue_cbs<TData, MAX_SUBS>(cb_list); // queue the callbacks, they'll handle setting data = new_data themselves
            }
            else
            {
                data = new_data; // no callbacks registered, we have to do this ourselves
            }

            return true; 
        }

        /**
         * @brief Sets contained data and executes any subscribed callbacks.
         *
         * @return The current data.
         */
        TData get()
        {
            return data;
        }

        /**
         * @brief Registers a callback to be executed when contained data is set().
         *
         * @param cb The callback function to subscribe.
         *
         * @return True on subscription success.
         */
        bool subscribe(std::function<void(TData)> cb)
        {
            // if cb helper isn't initialized, nothing will execute anyway
            if (!CbHelper::initialized)
                return false;

            // only register a function if MAX_SUBS has not been reached
            if (cb_list.size() < MAX_SUBS)
            {
                // new element to be added, this is no-longer the final element in vector
                if (cb_list.size() > 0)
                    cb_list.back().last_in_chain = false;

                cb_list.push_back(CbArgTyped<TData>(cb, &arg2p, &data)); // Create a cb wrapper and push it into the vector
                cb_list.back().last_in_chain = true;                     // set new final element in vector

                return true;
            }

            return false;
        }

    private:
        TData data;                                       ///< Current data value.
        TData arg2p;                                      ///< Value to be passed to callbacks when they are invoked (newest data).
        etl::vector<CbArgTyped<TData>, MAX_SUBS> cb_list; ///< List of subscribed callbacks.

        static const constexpr char* TAG = "DataWatch"; ///< Class tag, used in debug logs.
};
