#include "app_main.h"
#include "../Inc/tempHumiditySensor.h"
#include "usart.h"
#include <cstring>

extern "C" int app_main()
{
    tempHumiditySensor test;
    const char* str = "testing\n\r";
    test.init();
    while(1)
    {
        HAL_UART_Transmit(&huart3, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
        test.getHumidity_Temp();
    }


    return 0; 
}