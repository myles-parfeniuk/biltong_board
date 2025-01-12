#include "UnityInterface.h"

/* This is the C version of Test_putchar */
void Test_putchar(int c)
{
    uint8_t char2tx = (uint8_t)(c);

    if(char2tx == '\n')
    {
        char2tx = '\r';
        HAL_UART_Transmit(&huart3, &char2tx, 1, 100UL);
        char2tx = '\n';
    }

    HAL_UART_Transmit(&huart3, &char2tx, 1, 100UL);
}
