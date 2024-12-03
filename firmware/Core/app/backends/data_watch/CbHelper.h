#pragma once
// std library includes
#include <stdio.h>
// third party includes
#include <etl/vector.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
// in house includes
#include "CbArgTyped.h"

/**
 * @class CbHelper
 *
 * @brief Fully static class to manage execution of callbacks registered to DataWatch objects.
 * */
class CbHelper
{
    public:
        inline static bool initialized = false; ///< Whether or not CbHelper has been initialized.

        /**
         * @brief Initializes CbHelper by creating cb queue and launching task_cb().
         *
         * @return True on init success.
         */
        static bool init()
        {
            hdl_queue_cb = xQueueCreate(CB_QUEUE_SZ, sizeof(CbGeneric*));
            // create cb task
            if (xTaskCreate(task_cb, "cb", 256, NULL, 1, &hdl_task_cb) != pdTRUE)
                return false;

            initialized = true;
            return true;
        }

        /**
         * @brief Queues callbacks to be passed to task_cb() and executed.
         *
         * @tparam TData Type of data this callback is passed.
         * @tparam MAX_SUBS Max subscribes for the DataWatch object calling this function.
         *
         * @param cb_list Reference to callback list contained within calling DataWatch object.
         *
         * @return True if all callbacks were successfully queued.
         */
        template <typename TData, size_t MAX_SUBS>
        static bool queue_cbs(etl::vector<CbArgTyped<TData>, MAX_SUBS>& cb_list)
        {
            for (const auto& cb : cb_list)
            {
                CbGeneric* cb2invoke = &cb;

                if (xQueueSend(hdl_queue_cb, &cb2invoke, 0UL) != pdTRUE)
                    return false;
            }

            return true;
        }

    private:
        inline static TaskHandle_t hdl_task_cb = NULL;   ///< Callback task handle.
        inline static QueueHandle_t hdl_queue_cb = NULL; ///< Callback queue handle.

        /**
         * @brief Task responsible for executing callbacks subscribed to any DataWatch objects.
         *
         * @param arg Task input argument, not used, see xTaskCreate.
         *
         * @return void, nothing to return.
         */
        static void task_cb(void* arg)
        {
            CbGeneric* cb2invoke;

            while (1)
            {
                // receive callback through queue as generic and execute
                if (xQueueReceive(hdl_queue_cb, &cb2invoke, portMAX_DELAY) != pdFALSE)
                    cb2invoke->invoke();
            }
        }

        static const constexpr uint16_t CB_QUEUE_SZ = 10; ///< Max amount of callbacks that can be queued/ pending to be executed at once.
        static const constexpr char* TAG = "CbHelper";    ///< Class tag, used in debug logs.
};