#pragma once

// std library includes
#include <stdio.h>
#include <stdint.h>
#include <cstring>
#include <cstdarg>
// cube mx inclues
#include "usart.h"
// third party includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

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

            // create serial service queue to hold 5 strings of max size 100 bytes.
            serial_queue = xQueueCreate(QUEUE_SZ, MAX_STR_SZ * sizeof(char));

            // create serial service task
            if (xTaskCreate(task_serial_service, "serial_service", 256, NULL, 1, &hdl_task_serial_service) != pdTRUE)
                return false;

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
            int16_t chars_written = 0U;
            char str_to_send[MAX_STR_SZ] = {};
            va_list args;
            static const uint16_t TAG_NAME_LENGTH = strlen(TAG);
            static const uint16_t TAG_NAME_LENGTH_TOT = strlen(TAG) + 2; // +2 for ": "

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
            chars_written = vsnprintf(str_to_send + TAG_NAME_LENGTH_TOT, MAX_STR_SZ, format, args);

            va_end(args);

            if (chars_written >= (MAX_STR_SZ - TAG_NAME_LENGTH_TOT - 2)) // -2 for "\n\0"
                return false;

            str_to_send[TAG_NAME_LENGTH_TOT + chars_written] = '\n';
            str_to_send[TAG_NAME_LENGTH_TOT + chars_written + 1] = '\0';

            // send to queue and ensure it is not already full
            if (xQueueSend(serial_queue, str_to_send, 0UL) != pdTRUE)
                return false;

            return true;
        }

        static const constexpr uint16_t MAX_STR_SZ = 100U; ///< max string size in bytes that can be sent over queue

    private:
        /**
         * @brief Task responsible for sending messages over serial via UART.
         *
         * @param arg Task input argument, not used, see xTaskCreate.
         *
         * @return void, nothing to return.
         */
        static void task_serial_service(void* arg)
        {
            char str_to_send[MAX_STR_SZ] = {};

            while (1)
            {
                // wait for a string to be received over the queue, max possible timeout
                if (xQueueReceive(serial_queue, str_to_send, portMAX_DELAY) != pdFALSE)
                {
                    HAL_UART_Transmit(hdl_uart, reinterpret_cast<uint8_t*>(str_to_send), strlen(str_to_send), TX_TIMEOUT_MS);
                }
            }
        }

        static inline TaskHandle_t hdl_task_serial_service = NULL; ///< Serial service task handle.
        static inline QueueHandle_t serial_queue = NULL;           ///< Queue to send messages from tasks calling print APIs to task_serial_service()
        static inline UART_HandleTypeDef* hdl_uart = nullptr;      ///< Pointer to stm32CUBE HAL uart handle to use with serial transmissions.

        static const constexpr uint16_t QUEUE_SZ = 5U;         ///< <Max strings that can be queued at once
        static const constexpr uint32_t TX_TIMEOUT_MS = 100UL; ///< Uart sending timeout in ms (only in ms if HAL tick is set to 1ms)
        static const constexpr char* TAG = "SerialService";    ///< Class tag, used in debub logs.
};