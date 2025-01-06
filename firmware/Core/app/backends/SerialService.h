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
#include "semphr.h"

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
    static bool init(UART_HandleTypeDef *huart)
    {
        // assign uart handle
        hdl_uart = huart;

        // create lock
        lock_uart = xSemaphoreCreateMutex();

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
    static bool print_log_ln(const char *TAG, const char *format, ...)
    {
        HAL_StatusTypeDef tx_res = HAL_ERROR;
        int16_t chars_written = 0U;
        char str_to_send[MAX_STR_SZ] = {};
        va_list args;
        static const uint16_t TAG_NAME_LENGTH = strlen(TAG);
        static const uint16_t TAG_NAME_LENGTH_TOT = TAG_NAME_LENGTH + 2;

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

        if (chars_written >= (MAX_STR_SZ - TAG_NAME_LENGTH_TOT - 3)) // -3 for "\n\r\0"
            return false;

        str_to_send[TAG_NAME_LENGTH_TOT + chars_written] = '\n';
        str_to_send[TAG_NAME_LENGTH_TOT + chars_written + 1] = '\r';
        str_to_send[TAG_NAME_LENGTH_TOT + chars_written + 2] = '\0';

        if (xSemaphoreTake(lock_uart, LOCK_TIMEOUT_MS / portMAX_DELAY) == pdTRUE)
        {
            tx_res = HAL_UART_Transmit(hdl_uart, reinterpret_cast<uint8_t *>(str_to_send), strlen(str_to_send), TX_TIMEOUT_TICKS);
            xSemaphoreGive(lock_uart);
        }

        return (tx_res == HAL_OK);
    }

       static bool putchar(uint8_t char2put)
       {
            HAL_StatusTypeDef tx_res = HAL_ERROR;
            tx_res = HAL_UART_Transmit(hdl_uart, &char2put, 1, TX_TIMEOUT_TICKS);

            return (tx_res == HAL_OK);
       }

    static const constexpr uint16_t MAX_STR_SZ = 100U; ///< max string size in bytes that can be sent over queue

private:
    static inline TaskHandle_t hdl_task_serial_service = NULL; ///< Serial service task handle.
    static inline SemaphoreHandle_t lock_uart = NULL;          ///< Mutex for ensuring UART peripheral is written to by two tasks at once.
    static inline UART_HandleTypeDef *hdl_uart = nullptr;      ///< Pointer to stm32CUBE HAL uart handle to use with serial transmissions.

    static const constexpr BaseType_t LOCK_TIMEOUT_MS = 100UL / portTICK_PERIOD_MS; ///<
    static const constexpr uint32_t TX_TIMEOUT_TICKS = 100UL;                       ///< Uart sending timeout in ms (only in ms if HAL tick is set to 1ms)
    static const constexpr char *TAG = "SerialService";                             ///< Class tag, used in debub logs.
};