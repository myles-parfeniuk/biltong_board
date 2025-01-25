#pragma once

// std library
#include <stdio.h>
#include <stdint.h>
#include <cstring>
#include <cstdarg>
// cube mx
#include "usart.h"
// third party
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
// in-house
#include "bb_task_defs.h"

/**
 * @class SerialService
 *
 * @brief Fully static class with task-safe APIs for printing debug messages over serial.
 * */
class SerialService
{
    public:
        /**
         * @brief Initializes serial service for printing debug message.
         *
         * Creates queue and launches task to handle uart transmissions.
         *
         * @param huart Pointer to stm32CUBE HAL uart handle to use with transmissions.
         *
         * @return True on init success.
         */
        static bool init(UART_HandleTypeDef* huart)
        {
            // assign uart handle
            hdl_uart = huart;

            // create queue
            queue_serial_hdl = xQueueCreateStatic(MAX_PENDING_MSGS, MAX_STR_SZ * sizeof(uint8_t), queue_serial_buff, &queue_serial);
            if (queue_serial_hdl == NULL)
                return false;

            // launch serial service task
            task_serial_svc_hdl =
                    xTaskCreateStatic(serial_task, "bbSerialSvcTsk", BB_SERIAL_SVC_TSK_SZ, NULL, 5, task_serial_svc_stk, &task_serial_svc_tcb);

            return true;
        }

        /**
         * @brief Prints a message over serial with appended tag and new line.
         *
         * @param TAG Tag used to differentiate which module this function was called from.
         * @param format String with or without variable arguments, ie same as printf formatting.
         *
         * @warning Total string length + tag must be less than SerialService::MAX_STR_SZ.
         *
         * @return True on success to pass string SerialService queue, false on failure (queue is full).
         */
        static bool print_log_ln(const char* TAG, const char* format, ...)
        {
            BaseType_t pushed_to_queue = pdFALSE;
            int16_t chars_written = 0U;
            uint8_t str_to_send[MAX_STR_SZ] = {};
            va_list args;
            const uint16_t TAG_NAME_LENGTH = strlen(TAG);
            const uint16_t TAG_NAME_LENGTH_TOT = TAG_NAME_LENGTH + 2;

            if (hdl_uart == nullptr)
                return false;

            if (TAG_NAME_LENGTH_TOT + 1 >= MAX_STR_SZ)
                return false;

            memcpy(str_to_send, TAG, TAG_NAME_LENGTH);
            str_to_send[TAG_NAME_LENGTH] = ':';
            str_to_send[TAG_NAME_LENGTH + 1] = ' ';

            // set args to point to variable arguments passed through format
            va_start(args, format);

            // capture the arguments into a buffer
            chars_written = vsnprintf(reinterpret_cast<char*>(str_to_send) + TAG_NAME_LENGTH_TOT, MAX_STR_SZ, format, args);

            va_end(args);

            if (chars_written >= (MAX_STR_SZ - TAG_NAME_LENGTH_TOT - 3)) // -3 for "\n\r\0"
                return false;

            str_to_send[TAG_NAME_LENGTH_TOT + chars_written] = '\n';
            str_to_send[TAG_NAME_LENGTH_TOT + chars_written + 1] = '\r';
            str_to_send[TAG_NAME_LENGTH_TOT + chars_written + 2] = '\0';

            // send message to serial_task via queue_serial
            xQueueSend(queue_serial_hdl, str_to_send, 0UL);
            return (pushed_to_queue == pdTRUE);
        }

        static bool putchar(uint8_t char2put)
        {
            HAL_StatusTypeDef tx_res = HAL_ERROR;
            tx_res = HAL_UART_Transmit(hdl_uart, &char2put, 1, TX_TIMEOUT_TICKS);

            return (tx_res == HAL_OK);
        }

        static void serial_task(void* arg)
        {
            uint8_t str_to_send[MAX_STR_SZ];

            while (1)
            {
                if (xQueueReceive(queue_serial_hdl, str_to_send, portMAX_DELAY) == pdTRUE)
                    HAL_UART_Transmit(hdl_uart, str_to_send, strlen(reinterpret_cast<char*>(str_to_send)), TX_TIMEOUT_TICKS);
            }
        }

        static const constexpr uint16_t MAX_STR_SZ = 100U;     ///< max string size in bytes that can be sent over queue
        static const constexpr uint16_t MAX_PENDING_MSGS = 3U; ///< max strings that can be in queue_serial at any given time

    private:
        inline static TaskHandle_t task_serial_svc_hdl = NULL; ///< Serial service task handle.
        inline static StaticTask_t task_serial_svc_tcb;
        inline static StackType_t task_serial_svc_stk[BB_SERIAL_SVC_TSK_SZ] = {0UL};
        inline static uint8_t queue_serial_buff[MAX_PENDING_MSGS * (MAX_STR_SZ * sizeof(uint8_t))];
        inline static StaticQueue_t queue_serial;
        inline static QueueHandle_t queue_serial_hdl = NULL;      ///< Queue for sending data to serial_task()
        inline static UART_HandleTypeDef* hdl_uart = nullptr;     ///< Pointer to stm32CUBE HAL uart handle to use with serial transmissions.
        static const constexpr uint32_t TX_TIMEOUT_TICKS = 100UL; ///< Uart sending timeout in ms (only in ms if HAL tick is set to 1ms)
        static const constexpr char* TAG = "SerialService";       ///< Class tag, used in debub logs.
};