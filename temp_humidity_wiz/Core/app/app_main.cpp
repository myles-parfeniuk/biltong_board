#include "app_main.h"
#include "temp_humidity_sensor/tempHumiditySensor.h"
#include <cstring>

static void debugPrintln(char _out[])
{
	HAL_UART_Transmit(&huart3, (uint8_t *)_out, strlen(_out), 10);
	char* newline = "\r\n";
	HAL_UART_Transmit(&huart3, (uint8_t *)newline, strlen(newline), 10);
}


extern "C" int app_main()
{
    tempHumiditySensor test;
    const char* str = "testing\n\r";
    debugPrintln("");
    debugPrintln("");
    debugPrintln("");
    debugPrintln("");
    debugPrintln("");
    test.init();
    while(1)
    {
        HAL_Delay(500);
        HAL_UART_Transmit(&huart3, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
        test.getHumidity();
        test.getTemp();
    }


    return 0; 
}