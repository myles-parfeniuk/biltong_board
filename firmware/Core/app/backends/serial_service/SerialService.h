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

#define BB_LOGI(TAG, format, ...) SerialService::LOG_ln(BB_LL_INFO, TAG, format, ##__VA_ARGS__)
#define BB_LOGSC(TAG, format, ...) SerialService::LOG_ln(BB_LL_SUCCESS, TAG, format, ##__VA_ARGS__)
#define BB_LOGE(TAG, format, ...) SerialService::LOG_ln(BB_LL_ERROR, TAG, format, ##__VA_ARGS__)
#define BB_LOGW(TAG, format, ...) SerialService::LOG_ln(BB_LL_WARNING, TAG, format, ##__VA_ARGS__)
#define BB_LOGSP(TAG, format, ...) SerialService::LOG_ln(BB_LL_SPECIAL, TAG, format, ##__VA_ARGS__)

enum bb_log_lvl_t
{
    BB_LL_INFO,
    BB_LL_SUCCESS,
    BB_LL_WARNING,
    BB_LL_ERROR,
    BB_LL_SPECIAL
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
        static bool init(UART_HandleTypeDef* huart);

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
        static bool LOG_ln(const bb_log_lvl_t log_lvl, const char* TAG, const char* format, ...);

        static bool putchar(uint8_t char2put);

        static void assert_tx_done();

        inline static const constexpr size_t MAX_STR_SZ = 100U; ///< max string size in bytes that can be sent over queue
        inline static const constexpr size_t MAX_PENDING_MSGS =
                3U; ///< max strings that can be in queue_serial at any given time (amount of string buffers)

    private:
        inline static const constexpr size_t LENGTH_IDX = MAX_STR_SZ + 1;  ///< index of string buffer of which length of string is stored
        inline static const constexpr size_t STR_BUFF_SZ = MAX_STR_SZ + 2; ///< size of string buffers
        inline static const constexpr char* DEFAULT = "\033[0m";           // white (default) BB_LOGI
        inline static const constexpr char* GREEN = "\033[32m";            // green (success) BB_LOGSC
        inline static const constexpr char* RED = "\033[31m";              // red (error) BB_LOGE
        inline static const constexpr char* YELLOW = "\033[33m";           // yellow (warning) BB_LOGW
        inline static const constexpr char* BLUE = "\033[1;34m";           // blue (special) BB_LOGSP

        static bool append_color_code(const bb_log_lvl_t log_lvl, uint8_t* str, size_t& length);
        static bool append_tag(uint8_t* str, size_t& length, const char* TAG);
        static bool append_message(uint8_t* str, size_t& length, const char* format, va_list args);
        static bool append_end_l(uint8_t* str, size_t& length);

        static bool wait_tx_done();
        static bool transmit(const uint8_t* str_to_send, const size_t length);

        static void serial_task(void* arg);

        inline static TaskHandle_t task_serial_svc_hdl = NULL; ///< Serial service task handle.
        inline static StaticTask_t task_serial_svc_tcb;
        inline static StackType_t task_serial_svc_stk[BB_SERIAL_SVC_TSK_SZ] = {0UL};
        inline static uint8_t queue_serial_buff[MAX_PENDING_MSGS * (STR_BUFF_SZ * sizeof(uint8_t))];
        inline static StaticQueue_t queue_serial;
        inline static QueueHandle_t queue_serial_hdl = NULL;  ///< Queue for sending data to serial_task()
        inline static UART_HandleTypeDef* hdl_uart = nullptr; ///< Pointer to stm32CUBE HAL uart handle to use with serial transmissions.
        inline static const constexpr TickType_t TX_TIMEOUT_MS = 100UL / portTICK_PERIOD_MS; ///< Uart sending timeout in ms

        inline static const constexpr char* TAG = "SerialService"; ///< Class tag, used in debub logs.
};