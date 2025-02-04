#include "SerialService.h"

bool SerialService::init(UART_HandleTypeDef* huart)
{
    // assign uart handle
    hdl_uart = huart;

    // create queue
    queue_serial_hdl = xQueueCreateStatic(MAX_PENDING_MSGS, STR_BUFF_SZ * sizeof(uint8_t), queue_serial_buff, &queue_serial);
    if (queue_serial_hdl == NULL)
        return false;

    // launch serial service task
    task_serial_svc_hdl = xTaskCreateStatic(serial_task, "bbSerialSvcTsk", BB_SERIAL_SVC_TSK_SZ, NULL, 5, task_serial_svc_stk, &task_serial_svc_tcb);

    return true;
}

bool SerialService::LOG_ln(const bb_log_lvl_t log_lvl, const char* TAG, const char* format, ...)
{
    BaseType_t pushed_to_queue = pdFALSE;
    size_t length = 0U;
    uint8_t str_to_send[STR_BUFF_SZ] = {};
    va_list args;

    if (hdl_uart == nullptr)
        return false;

    if (!append_color_code(log_lvl, str_to_send, length))
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
    if (!append_color_code(BB_LL_INFO, str_to_send, length))
        return false;

    if (!append_end_l(str_to_send, length))
        return false;

    // store length
    str_to_send[LENGTH_IDX] = static_cast<uint8_t>(length);

    // send message to serial_task via queue_serial
    xQueueSend(queue_serial_hdl, str_to_send, 10UL);
    return (pushed_to_queue == pdTRUE);
}

bool SerialService::putchar(uint8_t char2put)
{
    return transmit(&char2put, 1);
}

bool SerialService::append_color_code(const bb_log_lvl_t log_lvl, uint8_t* str, size_t& length)
{
    // clang-format off
            const char *escape_code =   (log_lvl == BB_LL_INFO) ? DEFAULT :
                                        (log_lvl == BB_LL_SUCCESS) ? GREEN :
                                        (log_lvl == BB_LL_WARNING) ? YELLOW :
                                        (log_lvl == BB_LL_ERROR) ? RED : 
                                        (log_lvl == BB_LL_SPECIAL) ? BLUE : 
                                        DEFAULT;
    // clang-format on

    const size_t escape_code_length = strlen(escape_code);
    const size_t new_length = length + escape_code_length;

    if (new_length < MAX_STR_SZ)
    {
        memcpy(str + length, escape_code, escape_code_length);
        length = new_length;
        return true;
    }

    return false;
}

bool SerialService::append_tag(uint8_t* str, size_t& length, const char* TAG)
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

bool SerialService::append_message(uint8_t* str, size_t& length, const char* format, va_list args)
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

bool SerialService::append_end_l(uint8_t* str, size_t& length)
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

bool SerialService::wait_tx_done()
{
    if (xTaskNotifyWait(0UL, 0UL, NULL, TX_TIMEOUT_MS) != pdTRUE)
        return true;
    else
        return false;
}

void SerialService::assert_tx_done()
{
    BaseType_t higher_priority_task_awoken = pdFALSE;
    xTaskNotifyFromISR(SerialService::task_serial_svc_hdl, 0UL, eNoAction, &higher_priority_task_awoken);
    portYIELD_FROM_ISR(higher_priority_task_awoken);
}

bool SerialService::transmit(const uint8_t* str_to_send, const size_t length)
{
    HAL_StatusTypeDef op_success = HAL_ERROR;

    op_success = HAL_UART_Transmit_IT(hdl_uart, str_to_send, length);

    if (op_success != HAL_OK)
        return false;

    return wait_tx_done();
}

void SerialService::serial_task(void* arg)
{
    uint8_t str_to_send[STR_BUFF_SZ];

    while (1)
    {
        if (xQueueReceive(queue_serial_hdl, str_to_send, portMAX_DELAY) == pdTRUE)
        {
            transmit(str_to_send, str_to_send[LENGTH_IDX]);
        }
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)
{
    SerialService::assert_tx_done();
}
