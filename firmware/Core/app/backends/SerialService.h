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

enum bb_log_lvl_t
{
    BB_LL_INFO,
    BB_LL_SUCCESS,
    BB_LL_WARNING,
    BB_LL_ERROR
};

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
            queue_serial_hdl = xQueueCreateStatic(MAX_PENDING_MSGS, STR_BUFF_SZ * sizeof(uint8_t), queue_serial_buff, &queue_serial);
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
        template <bb_log_lvl_t level>
        static bool LOG_ln(const char* TAG, const char* format, ...)
        {
            BaseType_t pushed_to_queue = pdFALSE;
            size_t length = 0U;
            uint8_t str_to_send[STR_BUFF_SZ] = {};
            va_list args;

            if (hdl_uart == nullptr)
                return false;

            if (!append_color_code<level>(str_to_send, length))
                return false;

            if (!append_tag(str_to_send, length, TAG))
                return false;

            va_start(args, format);
            if (!append_message(str_to_send, length, format, args))
            {
                va_end(args);
                return false;
            }
            va_end(args);

            // reset color
            if (!append_color_code<BB_LL_INFO>(str_to_send, length))
                return false;

            if (!append_end_l(str_to_send, length))
                return false;

            // store length
            str_to_send[MAX_STR_SZ] = static_cast<uint8_t>(length);

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
            uint8_t str_to_send[STR_BUFF_SZ];

            while (1)
            {
                if (xQueueReceive(queue_serial_hdl, str_to_send, portMAX_DELAY) == pdTRUE)
                    HAL_UART_Transmit(hdl_uart, str_to_send, str_to_send[MAX_STR_SZ], TX_TIMEOUT_TICKS);
            }
        }

        static const constexpr size_t MAX_STR_SZ = 100U; ///< max string size in bytes that can be sent over queue
        static const constexpr size_t STR_BUFF_SZ = 101U;
        static const constexpr size_t MAX_PENDING_MSGS = 3U; ///< max strings that can be in queue_serial at any given time

    private:
        static const constexpr char* DEFAULT = "\033[0m"; // white (default)
        static const constexpr char* GREEN = "\033[32m";  // green (success)
        static const constexpr char* RED = "\033[31m";    // red (error)
        static const constexpr char* YELLOW = "\033[33m"; // yellow (warning)

        template <bb_log_lvl_t level>
        static bool append_color_code(uint8_t* str, size_t& length)
        {
            // clang-format off
            const char *escape_code =   (level == BB_LL_INFO) ? DEFAULT :
                                        (level == BB_LL_SUCCESS) ? GREEN :
                                        (level == BB_LL_WARNING) ? YELLOW :
                                        (level == BB_LL_ERROR) ? RED : 
                                        DEFAULT;
            // clang-format on
            const size_t escape_code_length = (level == BB_LL_INFO)      ? strlen(DEFAULT)
                                              : (level == BB_LL_SUCCESS) ? strlen(GREEN)
                                              : (level == BB_LL_WARNING) ? strlen(YELLOW)
                                              : (level == BB_LL_ERROR)   ? strlen(RED)
                                                                         : strlen(DEFAULT);

            const size_t new_length = length + escape_code_length;

            if (new_length < MAX_STR_SZ)
            {
                memcpy(str + length, escape_code, escape_code_length);
                length = new_length;
                return true;
            }

            return false;
        }

        static bool append_tag(uint8_t* str, size_t& length, const char* TAG)
        {
            const size_t TAG_NAME_LENGTH = strlen(TAG);
            const size_t new_length = length + TAG_NAME_LENGTH + 2UL;

            if (new_length < MAX_STR_SZ)
            {
                memcpy(str + length, TAG, TAG_NAME_LENGTH);
                str[length + TAG_NAME_LENGTH] = ':';
                str[length + TAG_NAME_LENGTH + 1] = ' ';
                length = new_length;
                return true;
            }

            return false;
        }

        static bool append_message(uint8_t* str, size_t& length, const char* format, va_list args)
        {
            size_t new_length = 0UL;
            int msg_sz = 0;

            // get message size
            msg_sz = vsnprintf(nullptr, 0, format, args);

            new_length = length + msg_sz;

            if (new_length < MAX_STR_SZ)
            {
                // append msg
                vsnprintf(reinterpret_cast<char*>(str) + length, MAX_STR_SZ - length, format, args);
                length = new_length;
                return true;
            }

            return false;
        }

        static bool append_end_l(uint8_t* str, size_t& length)
        {
            const size_t new_length = length + 2UL;

            if (new_length < MAX_STR_SZ)
            {
                str[length] = '\n';
                str[length + 1] = '\r';
                str[length + 2] = '\0';
                length = new_length;
                return true;
            }

            return false;
        }

        inline static TaskHandle_t task_serial_svc_hdl = NULL; ///< Serial service task handle.
        inline static StaticTask_t task_serial_svc_tcb;
        inline static StackType_t task_serial_svc_stk[BB_SERIAL_SVC_TSK_SZ] = {0UL};
        inline static uint8_t queue_serial_buff[MAX_PENDING_MSGS * (STR_BUFF_SZ * sizeof(uint8_t))];
        inline static StaticQueue_t queue_serial;
        inline static QueueHandle_t queue_serial_hdl = NULL;      ///< Queue for sending data to serial_task()
        inline static UART_HandleTypeDef* hdl_uart = nullptr;     ///< Pointer to stm32CUBE HAL uart handle to use with serial transmissions.
        static const constexpr uint32_t TX_TIMEOUT_TICKS = 100UL; ///< Uart sending timeout in ms (only in ms if HAL tick is set to 1ms)

        static const constexpr char* TAG = "SerialService"; ///< Class tag, used in debub logs.
};