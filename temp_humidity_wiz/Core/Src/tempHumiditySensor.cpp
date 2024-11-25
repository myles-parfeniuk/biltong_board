#include "../Inc/tempHumiditySensor.h"
#include "../third_party/WSEN_HIDS_2525020210002.h"
#include <stdio.h>
#include "i2c.h"
#include "usart.h"
#include <cstring>
/* Sensor interface configuration */
WE_sensorInterface_t tempHumiditySensor::hids;

tempHumiditySensor::tempHumiditySensor()
{
}

/* Debug output functions */
static void debugPrint(char _out[])
{
	HAL_UART_Transmit(&huart3, (uint8_t *)_out, strlen(_out), 10);
}

static void debugPrintln(char _out[])
{
	HAL_UART_Transmit(&huart3, (uint8_t *)_out, strlen(_out), 10);
	char newline[3] = "\r\n";
	HAL_UART_Transmit(&huart3, (uint8_t *)newline, 2, 10);
}

bool tempHumiditySensor::init()
{
	HIDS_Get_Default_Interface(&hids);
	hids.interfaceType = WE_i2c_fifo;
	hids.handle = &hi2c1;
	/* Wait for boot */
	HAL_Delay(50);
	if (WE_SUCCESS != HIDS_Sensor_Init(&hids))
	{
		debugPrintln("**** HIDS_MULTIPLEXER_Init error. STOP ****");
		HAL_Delay(5);
		while (1)
			;
	}

	debugPrintln("**** WE_isSensorInterfaceReady(): OK ****");
	return true;
}

uint32_t tempHumiditySensor::getHumidity_Temp()
{
	int32_t temperatureRaw = 0;
	int32_t humidityRaw = 0;
	hids_measureCmd_t measureCmd = HIDS_MEASURE_HPM;
	if (WE_SUCCESS == HIDS_Sensor_Measure_Raw(&hids, measureCmd, &temperatureRaw, &humidityRaw))
	{
		char bufferHumidity[11];
		sprintf(bufferHumidity, "%li", humidityRaw);
		debugPrint(bufferHumidity);
		debugPrint(",");
		char bufferTemperature[11];
		sprintf(bufferTemperature, "%li", temperatureRaw);
		debugPrint(bufferTemperature);
		debugPrintln("");
	}
}

uint32_t tempHumiditySensor::getTemp()
{
}
